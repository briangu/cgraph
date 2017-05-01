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
  entry->soEntries = malloc(sizeof(EntityPair) * PREDICATE_ENTRY_INITIAL_ALLOCATION_LENGTH);
  entry->osEntries = malloc(sizeof(EntityPair) * PREDICATE_ENTRY_INITIAL_ALLOCATION_LENGTH);
  return entry;
}

void freePredicateEntry(PredicateEntry *entry) {
  free(entry->soEntries);
  free(entry->osEntries);
  free(entry);
}

void growPredicateEntry(PredicateEntry *entry) {
  entry->currentEntriesLength *= 2;
  entry->soEntries = realloc(entry->soEntries, entry->currentEntriesLength);
  entry->osEntries = realloc(entry->osEntries, entry->currentEntriesLength);
}

void addToPredicateEntry(PredicateEntry *entry, SubjectId subject, ObjectId object) {
  if ((entry->entryCount + 1) >= entry->currentEntriesLength) {
    growPredicateEntry(entry);
  }
  entry->soEntries[entry->entryCount] = toSOEntry(subject, object);
  entry->osEntries[entry->entryCount] = toOSEntry(object, subject);
  entry->entryCount++;
}

PredicateEntryIterator *createPredicateEntryIterator() {
  PredicateEntryIterator *iterator = malloc(sizeof(PredicateEntryIterator));
  iterator->position = 0;
  iterator->done = 0;
  return iterator;
}

void freePredicateEntryIterator(PredicateEntryIterator *iterator) {
  free(iterator);
}

Triple iteratePredicateEntry(PredicateEntry *entry, PredicateEntryIterator *iterator) {
  if (iterator->position >= entry->entryCount) {
    iterator->done = 1;
    return 0;
  }
  printf("position: %ld\n", iterator->position);
  return toTripleFromSOEntry(entry->soEntries[iterator->position++], entry->predicate);
}

void testTriple() {
  Triple triple = toTriple(1,2,3);

  assert((triple & SUBJECT_MASK) == toTriple(1,0,0));
  assert((triple & PREDICATE_MASK) == toTriple(0,2,0));
  assert((triple & OBJECT_MASK) == toTriple(0,0,3));

  assert(subjectIdFromTriple(triple) == 1);
  assert(predicateIdFromTriple(triple) == 2);
  assert(objectIdFromTriple(triple) == 3);

  EntityPair soPair = tripleToSOEntry(triple);
  assert(soPair == (((EntityPair)1 << 32) | (EntityPair)3));
  assert((soPair >> 32) == 1);
  assert((soPair & 0xFFFFFFFF) == 3);
  assert(soPair == ((EntityPair)1 << 32 | 3));

  EntityPair osPair = tripleToOSEntry(triple);
  assert(osPair == ((EntityPair)3 << 32 | 1));

  assert(toTripleFromSOEntry(soPair, 2) == triple);
  assert(toTripleFromOSEntry(osPair, 2) == triple);
}

void testPredicateEntry() {
  PredicateEntry *entry = createPredicateEntry(2);

  for (int i = 1; i < (PREDICATE_ENTRY_INITIAL_ALLOCATION_LENGTH * 3); i++) {
    addToPredicateEntry(entry, i, 3);
  }

  assert(entry->entryCount == ((PREDICATE_ENTRY_INITIAL_ALLOCATION_LENGTH * 3) - 1));

  Triple triple = toTripleFromSOEntry(entry->soEntries[0], entry->predicate);
  assert(subjectIdFromTriple(triple) == 1);
  assert(predicateIdFromTriple(triple) == 2);
  assert(objectIdFromTriple(triple) == 3);

  PredicateEntryIterator *iterator = createPredicateEntryIterator();

  SubjectId i = 1;
  while (!iterator->done) {
    Triple triple = iteratePredicateEntry(entry, iterator);
    assert(subjectIdFromTriple(triple) == i++);
    assert(predicateIdFromTriple(triple) == 2);
    assert(objectIdFromTriple(triple) == 3);
  }

  assert(i == ((PREDICATE_ENTRY_INITIAL_ALLOCATION_LENGTH * 3) - 1));

  free(iterator);
  freePredicateEntry(entry);
}

void initialize() {
  testTriple();
  testPredicateEntry();
}
