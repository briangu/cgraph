#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "graph.h"

SubjectId subjectIdFromTriple(Triple triple) {
  return (SubjectId)((triple & SUBJECT_MASK) >> (PREDICATE_BIT_WIDTH + OBJECT_BIT_WIDTH));
}

PredicateId predicateIdFromTriple(Triple triple) {
  return (PredicateId)((triple & PREDICATE_MASK) >> OBJECT_BIT_WIDTH);
}

ObjectId objectIdFromTriple(Triple triple) {
  return (ObjectId)(triple & OBJECT_MASK);
}

EntityPair toSOEntry(SubjectId subject, ObjectId object) {
  return ((EntityPair)subject << 32) | (EntityPair)object;
}

EntityPair toOSEntry(ObjectId object, SubjectId subject) {
  return ((EntityPair)object << 32) | (EntityPair)subject;
}

EntityPair tripleToSOEntry(Triple triple) {
  return toSOEntry(subjectIdFromTriple(triple), objectIdFromTriple(triple));
}

EntityPair tripleToOSEntry(Triple triple) {
  return toOSEntry(objectIdFromTriple(triple), subjectIdFromTriple(triple));
}

Triple toTriple(SubjectId subject, PredicateId predicate, ObjectId object) {
  return (((Triple)subject) << (PREDICATE_BIT_WIDTH + OBJECT_BIT_WIDTH))
        | (((Triple)predicate) << OBJECT_BIT_WIDTH)
        | (Triple)object;
}

Triple toTripleFromSOEntry(EntityPair soPair, PredicateId predicate) {
  return toTriple((SubjectId)(soPair >> 32), predicate, (ObjectId)(soPair & 0xFFFFFFFF));
}

Triple toTripleFromOSEntry(EntityPair osPair, PredicateId predicate) {
  return toTriple((SubjectId)(osPair & 0xFFFFFFFF), predicate, (ObjectId)(osPair >> 32));
}

PredicateEntry* createPredicateEntry(PredicateId predicate) {
  PredicateEntry *entry = malloc(sizeof(PredicateEntry));
  entry->predicate = predicate;
  entry->entryCount = 0;
  entry->currentEntriesLength = PREDICATE_ENTRY_INITIAL_ALLOCATION_LENGTH;
  entry->soEntries = malloc(sizeof(EntityPair) * entry->currentEntriesLength);
  entry->osEntries = malloc(sizeof(EntityPair) * entry->currentEntriesLength);
  return entry;
}

void freePredicateEntry(PredicateEntry *entry) {
  free(entry->soEntries);
  free(entry->osEntries);
  free(entry);
}

void growPredicateEntry(PredicateEntry *entry) {
  entry->currentEntriesLength *= 2;
  entry->soEntries = realloc(entry->soEntries, sizeof(EntityPair) * entry->currentEntriesLength);
  entry->osEntries = realloc(entry->osEntries, sizeof(EntityPair) * entry->currentEntriesLength);
}

void addToPredicateEntry(PredicateEntry *entry, SubjectId subject, ObjectId object) {
  if ((entry->entryCount + 1) >= entry->currentEntriesLength) {
    growPredicateEntry(entry);
  }
  entry->soEntries[entry->entryCount] = toSOEntry(subject, object);
  entry->osEntries[entry->entryCount] = toOSEntry(object, subject);
  entry->entryCount++;
}

/*
  Iterator
*/
Triple iterate(Iterator *iterator) {
  PredicateEntryIterator *p = (PredicateEntryIterator *)iterator;
  if (p->position >= p->entry->entryCount) {
    p->done = TRUE;
    return 0;
  }
  return toTripleFromSOEntry(p->entry->soEntries[p->position++], p->entry->predicate);
}

Triple peek(Iterator *iterator) {
  PredicateEntryIterator *p = (PredicateEntryIterator *)iterator;
  if (p->position >= p->entry->entryCount) {
    p->done = TRUE;
    return 0;
  }
  return toTripleFromSOEntry(p->entry->soEntries[p->position], p->entry->predicate);
}

BOOL done(Iterator *iterator) {
  PredicateEntryIterator *p = (PredicateEntryIterator *)iterator;
  return (p->position >= p->entry->entryCount);
}

Iterator* createPredicateEntryIterator(PredicateEntry *entry) {
  PredicateEntryIterator *iterator = malloc(sizeof(PredicateEntryIterator));
  iterator->fn.iterate = &iterate;
  iterator->fn.peek = &peek;
  iterator->fn.done = &done;
  iterator->entry = entry;
  iterator->position = 0;
  iterator->done = FALSE;
  return (Iterator*)iterator;
}

void freePredicateEntryIterator(PredicateEntryIterator *iterator) {
  free(iterator);
}

Triple iterateOR(Iterator *iterator) {
  PredicateEntryJoinIterator *p = (PredicateEntryJoinIterator *)iterator;
  Iterator *aIterator = p->aIterator;
  Iterator *bIterator = p->bIterator;

  if (aIterator->done(aIterator)) {
    if (bIterator->done(bIterator)) {
      return 0;
    } else {
      return bIterator->iterate(bIterator);
    }
  } else {
    if (bIterator->done(bIterator)) {
      return aIterator->iterate(aIterator);
    } else {
      Triple a = aIterator->peek(aIterator);
      Triple b = bIterator->peek(bIterator);
      return a < b ? aIterator->iterate(aIterator) : bIterator->iterate(bIterator);
    }
  }
}

BOOL doneJoinOR(Iterator *iterator) {
  PredicateEntryJoinIterator *p = (PredicateEntryJoinIterator *)iterator;
  Iterator *aIterator = p->aIterator;
  Iterator *bIterator = p->bIterator;
  return (aIterator->done(aIterator) && bIterator->done(bIterator));
}

Triple iterateAND(Iterator *iterator) {
  PredicateEntryJoinIterator *p = (PredicateEntryJoinIterator *)iterator;
  Iterator *aIterator = p->aIterator;
  Iterator *bIterator = p->bIterator;

  Iterator *curIter = NULL;
  Triple nextTriple = 0;

  while (!aIterator->done(aIterator) && !bIterator->done(bIterator)) {
    Triple a = aIterator->peek(aIterator);
    Triple b = bIterator->peek(bIterator);

    if (a == b) {
      if (curIter) {
        nextTriple = curIter->iterate(curIter);
        curIter = curIter == aIterator ? bIterator : aIterator;
      } else {
        curIter = aIterator;
        nextTriple = curIter->iterate(curIter);
      }
      break;
    } else {
      bIterator->iterate(bIterator);
    }
  }

  return nextTriple;
}

// TODO: this is sub-optimal. we should do what we did in chriple
//      nextOperand, hasValue, getValue
BOOL doneJoinAND(Iterator *iterator) {
  PredicateEntryJoinIterator *p = (PredicateEntryJoinIterator *)iterator;
  Iterator *aIterator = p->aIterator;
  Iterator *bIterator = p->bIterator;

  // Iterator *curIter = NULL;
  BOOL done = TRUE;

  while (!aIterator->done(aIterator) && !bIterator->done(bIterator)) {
    Triple a = aIterator->peek(aIterator);
    Triple b = bIterator->peek(bIterator);

    if (a < b) {
      aIterator->iterate(aIterator);
    } else if (a == b) {
      done = FALSE;
      break;
    } else {
      bIterator->iterate(bIterator);
    }
  }

  return done;
}

Triple peekJoin(Iterator *iterator) {
  PredicateEntryJoinIterator *p = (PredicateEntryJoinIterator *)iterator;
  Iterator *aIterator = p->aIterator;
  Iterator *bIterator = p->bIterator;

  if (aIterator->done(aIterator)) {
    if (bIterator->done(bIterator)) {
      return 0;
    } else {
      return bIterator->peek(bIterator);
    }
  } else {
    if (bIterator->done(bIterator)) {
      return aIterator->peek(aIterator);
    } else {
      Triple a = aIterator->peek(aIterator);
      Triple b = bIterator->peek(bIterator);
      return a < b ? a : b;
    }
  }
}

Iterator* createPredicateEntryORIterator(Iterator *aIterator, Iterator *bIterator) {
  PredicateEntryJoinIterator *iterator = malloc(sizeof(PredicateEntryJoinIterator));
  iterator->fn.iterate = &iterateOR;
  iterator->fn.peek = &peekJoin;
  iterator->fn.done = &doneJoinOR;
  iterator->aIterator = aIterator;
  iterator->bIterator = bIterator;
  return (Iterator*)iterator;
}

Iterator* createPredicateEntryANDIterator(Iterator *aIterator, Iterator *bIterator) {
  PredicateEntryJoinIterator *iterator = malloc(sizeof(PredicateEntryJoinIterator));
  iterator->fn.iterate = &iterateAND;
  iterator->fn.peek = &peekJoin;
  iterator->fn.done = &doneJoinAND;
  iterator->aIterator = aIterator;
  iterator->bIterator = bIterator;
  return (Iterator*)iterator;
}

void freePredicateEntryORIterator(PredicateEntryJoinIterator *iterator) {
}

void initialize() {
}
