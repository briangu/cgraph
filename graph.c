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
  Iterator filters
*/
BOOL filter_PASSTHRU(FilterState *state, Triple triple) {
  return TRUE;
}

BOOL filter_S(FilterState *state, Triple triple) {
  EntityId a = subjectIdFromTriple(triple);

  if (state->TYPE == FILTER_TYPE_EQUAL) {
    FilterStateEqual *estate = (FilterStateEqual *)state;
    return a == estate->value;
  } else if (state->TYPE == FILTER_TYPE_RANGE_QUERY) {
    FilterStateRangeQuery *rqstate = (FilterStateRangeQuery *)state;
    return a >= rqstate->begin && a <= rqstate->end;
  }

  return FALSE;
}

BOOL filter_O(FilterState *state, Triple triple) {
  // EntityId a = objectIdFromTriple(triple);

  return TRUE;
}

/*
  Iterator
*/
BOOL iterate(Iterator *iterator, Triple *triple) {
  // printf("iterate %p\n", iterator);
  BOOL isDone = iterator->done(iterator);
  // printf("iterate isDone=%d\n", isDone);
  if (!isDone) {
    *triple = iterator->peek(iterator);
    iterator->advance(iterator);
  }
  return !isDone;
}

void advanceEntryIterator(Iterator *iterator) {
  assert(iterator->TYPE == ENTRY_ITERATOR);
  assert(!iterator->done(iterator));
  PredicateEntryIterator *p = (PredicateEntryIterator *)iterator;

  do {
    // printf("advanceEntryIterator %ld %lX\n", p->position, subjectIdFromTriple(iterator->peek(iterator)));
    p->position++;
  } while (!iterator->done(iterator) && !p->filter(p->filterState, iterator->peek(iterator)));
  // printf("advanceEntryIterator e->%ld\n", p->position);
}

void nextOperandEntryIterator(Iterator *iterator) {
  // printf("nextOperandEntryIterator %p\n", iterator);
  assert(iterator->TYPE == ENTRY_ITERATOR);
}

Triple peekEntryIterator(Iterator *iterator) {
  assert(iterator->TYPE == ENTRY_ITERATOR);
  assert(!iterator->done(iterator));
  PredicateEntryIterator *p = (PredicateEntryIterator *)iterator;
  return toTripleFromSOEntry(p->entry->soEntries[p->position], p->entry->predicate);
}

BOOL doneEntryIterator(Iterator *iterator) {
  // printf("doneEntryIterator %p\n", iterator);
  assert(iterator->TYPE == ENTRY_ITERATOR);
  PredicateEntryIterator *p = (PredicateEntryIterator *)iterator;
  // printf("doneEntryIterator %ld %ld\n", p->position, p->entry->entryCount);
  return (p->position >= p->entry->entryCount);
}

void initEntryIterator(Iterator *iterator) {
  assert(iterator->TYPE == ENTRY_ITERATOR);
}

void freeEntryIterator(Iterator *iterator) {
  assert(iterator->TYPE == ENTRY_ITERATOR);
  free(iterator);
}

Iterator* createPredicateEntryIterator(PredicateEntry *entry, filterFn filter, FilterState *filterState) {
  PredicateEntryIterator *iterator = malloc(sizeof(PredicateEntryIterator));
  iterator->fn.TYPE = ENTRY_ITERATOR;
  iterator->fn.advance = &advanceEntryIterator;
  iterator->fn.nextOperand = &nextOperandEntryIterator;
  iterator->fn.peek = &peekEntryIterator;
  iterator->fn.done = &doneEntryIterator;
  iterator->fn.init = &initEntryIterator;
  iterator->fn.free = &freeEntryIterator;
  iterator->filter = filter;
  iterator->filterState = filterState;
  iterator->entry = entry;
  iterator->position = 0;

  // advance to the first allowable filter position
  while (!iterator->fn.done((Iterator *)iterator) && !iterator->filter(iterator->filterState, iterator->fn.peek((Iterator *)iterator))) {
    iterator->position++;
  }

  return (Iterator *)iterator;
}

/*
Join
*/

void advanceJoin(Iterator *iterator) {
  // printf("advanceJoin %p\n", iterator);
  assert(iterator->TYPE == JOIN_ITERATOR);
  assert(!iterator->done(iterator));
  PredicateEntryJoinIterator *p = (PredicateEntryJoinIterator *)iterator;
  p->currentIterator->advance(p->currentIterator);
  iterator->nextOperand(iterator);
}

BOOL doneJoin(Iterator *iterator) {
  // printf("doneJoin %p\n", iterator);
  assert(iterator->TYPE == JOIN_ITERATOR);
  PredicateEntryJoinIterator *p = (PredicateEntryJoinIterator *)iterator;
  return p->currentIterator == NULL;
}

Triple peekJoin(Iterator *iterator) {
  // printf("peekJoin %p\n", iterator);
  assert(iterator->TYPE == JOIN_ITERATOR);
  assert(!iterator->done(iterator));
  PredicateEntryJoinIterator *p = (PredicateEntryJoinIterator *)iterator;
  return p->currentIterator->peek(p->currentIterator);
}

void initJoin(Iterator *iterator) {
  assert(iterator->TYPE == JOIN_ITERATOR);
  // printf("initJoin %p\n", iterator);
  PredicateEntryJoinIterator *p = (PredicateEntryJoinIterator *)iterator;
  assert(p->currentIterator == NULL);
  p->aIterator->init(p->aIterator);
  p->bIterator->init(p->bIterator);
  iterator->nextOperand(iterator);
}

void freeJoin(Iterator *iterator) {
  assert(iterator->TYPE == JOIN_ITERATOR);
  // printf("freeJoin %p\n", iterator);
  PredicateEntryJoinIterator *p = (PredicateEntryJoinIterator *)iterator;
  p->aIterator->free(p->aIterator);
  p->bIterator->free(p->bIterator);
  free(iterator);
}

// EntityId tripleComponentFromOperand(OperandSPOMode: mode, Iterator *iterator) {
//   if mode == OperandSPOModeSubject then return op.getValue().subject;
//   if mode == OperandSPOModeObject then return op.getValue().object;
//   halt("unsupported mode ", mode);
// }

/*
  Comparators
*/

int comparator_S(Iterator *aIterator, Iterator *bIterator) {
  EntityId a = subjectIdFromTriple(aIterator->peek(aIterator));
  EntityId b = subjectIdFromTriple(bIterator->peek(bIterator));

  return a < b ? -1 : (a == b) ? 0 : 1;
}

int comparator_SO(Iterator *aIterator, Iterator *bIterator) {
  Triple a = aIterator->peek(aIterator) & ~PREDICATE_MASK;
  Triple b = bIterator->peek(bIterator) & ~PREDICATE_MASK;

  return a < b ? -1 : (a == b) ? 0 : 1;
}

int comparator_O(Iterator *aIterator, Iterator *bIterator) {
  EntityId a = objectIdFromTriple(aIterator->peek(aIterator));
  EntityId b = objectIdFromTriple(bIterator->peek(bIterator));

  return a < b ? -1 : (a == b) ? 0 : 1;
}

int comparator_OS(Iterator *aIterator, Iterator *bIterator) {
  Triple a = aIterator->peek(aIterator) & ~PREDICATE_MASK;
  Triple b = bIterator->peek(bIterator) & ~PREDICATE_MASK;

  return a < b ? -1 : (a == b) ? 0 : 1;
}

/*
OR
*/

void nextOperandOR(Iterator *iterator) {
  assert(iterator->TYPE == JOIN_ITERATOR);
  // printf("nextOperandOR:S %p\n", iterator);
  PredicateEntryJoinIterator *p = (PredicateEntryJoinIterator *)iterator;
  Iterator *aIterator = p->aIterator;
  Iterator *bIterator = p->bIterator;
  // printf("nextOperandOR:1 %p %p\n", aIterator, bIterator);

  BOOL aDone = aIterator->done(aIterator);
  BOOL bDone = bIterator->done(bIterator);

  if (aDone) {
    // printf("nextOperandOR:2 %p\n", iterator);
    if (bDone) {
      // printf("a done b done\n");
      p->currentIterator = NULL;
    } else {
      // printf("a done b !done\n");
      p->currentIterator = p->bIterator;
    }
  } else {
    // printf("nextOperandOR:3 %p\n", bIterator);
    if (bDone) {
      // printf("a !done b done\n");
      p->currentIterator = p->aIterator;
    } else {
      // printf("a !done b !done\n");
      EntityId a = subjectIdFromTriple(aIterator->peek(aIterator));
      EntityId b = subjectIdFromTriple(bIterator->peek(bIterator));
      p->currentIterator = (a <= b) ? p->aIterator : p->bIterator;
    }
  }
  // printf("nextOperandOR:E\n");
}

Iterator* createPredicateEntryORIterator(Iterator *aIterator, Iterator *bIterator) {
  PredicateEntryJoinIterator *iterator = malloc(sizeof(PredicateEntryJoinIterator));
  iterator->fn.TYPE = JOIN_ITERATOR;
  iterator->fn.advance = &advanceJoin;
  iterator->fn.nextOperand = &nextOperandOR;
  iterator->fn.peek = &peekJoin;
  iterator->fn.done = &doneJoin;
  iterator->fn.init = &initJoin;
  iterator->fn.free = &freeJoin;
  iterator->aIterator = aIterator;
  iterator->bIterator = bIterator;
  iterator->currentIterator = NULL;
  return (Iterator *)iterator;
}

/*
AND
*/

void nextOperandAND(Iterator *iterator) {
  assert(iterator->TYPE == JOIN_ITERATOR);
  PredicateEntryJoinIterator *p = (PredicateEntryJoinIterator *)iterator;
  Iterator *aIterator = p->aIterator;
  Iterator *bIterator = p->bIterator;
  Iterator *nextIterator = NULL;
  BOOL firstAdvance = TRUE;

  while (!aIterator->done(aIterator) && !bIterator->done(bIterator)) {
    int compare = iterator->compare(aIterator, bIterator);
    if (compare < 0) {
      bIterator->advance(bIterator);
      firstAdvance = FALSE;
    } else if (compare == 0) {
      if (p->currentIterator != NULL) {
        if (firstAdvance) {
          p->currentIterator->advance(p->currentIterator);
        }
      }
      nextIterator = (p->currentIterator == aIterator) ? bIterator : aIterator;
      break;
    } else {
      aIterator->advance(aIterator);
      firstAdvance = FALSE;
    }
  }

  p->currentIterator = nextIterator;
}

Iterator* createPredicateEntryANDIterator(
  Iterator *aIterator,
  Iterator *bIterator,
  comparatorFn comparator
) {
  PredicateEntryJoinIterator *iterator = malloc(sizeof(PredicateEntryJoinIterator));
  iterator->fn.TYPE = JOIN_ITERATOR;
  iterator->fn.advance = &advanceJoin;
  iterator->fn.nextOperand = &nextOperandAND;
  iterator->fn.peek = &peekJoin;
  iterator->fn.done = &doneJoin;
  iterator->fn.init = &initJoin;
  iterator->fn.free = &freeJoin;
  iterator->aIterator = aIterator;
  iterator->bIterator = bIterator;
  iterator->currentIterator = NULL;
  iterator->comparator = comparator;
  return (Iterator*)iterator;
}

void initialize() {
}
