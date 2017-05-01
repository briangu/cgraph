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
  printf("growPredicateEntry\n");
  entry->currentEntriesLength *= 2;
  entry->soEntries = realloc(entry->soEntries, sizeof(EntityPair) * entry->currentEntriesLength);
  entry->osEntries = realloc(entry->osEntries, sizeof(EntityPair) * entry->currentEntriesLength);
}

void addToPredicateEntry(PredicateEntry *entry, SubjectId subject, ObjectId object) {
  if ((entry->entryCount + 1) >= entry->currentEntriesLength) {
    growPredicateEntry(entry);
  }
  entry->soEntries[entry->entryCount] = toSOEntry(subject, object);
  // printf("soEntry: %0.16llX\n", entry->soEntries[entry->entryCount]);
  entry->osEntries[entry->entryCount] = toOSEntry(object, subject);
  // printf("osEntry: %0.16llX\n", entry->osEntries[entry->entryCount]);
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
  assert((SUBJECT_BIT_WIDTH + PREDICATE_BIT_WIDTH + OBJECT_BIT_WIDTH) == 64);

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

  for (SubjectId i = 0; i < (2 ^ SUBJECT_BIT_WIDTH); i++) {
    Triple triple = toTriple(i,2,3);
    assert(subjectIdFromTriple(triple) == i);
    assert(predicateIdFromTriple(triple) == 2);
    assert(objectIdFromTriple(triple) == 3);
  }

  for (PredicateId i = 0; i < (2 ^ PREDICATE_BIT_WIDTH); i++) {
    Triple triple = toTriple(1,i,3);
    assert(subjectIdFromTriple(triple) == 1);
    assert(predicateIdFromTriple(triple) == i);
    assert(objectIdFromTriple(triple) == 3);
  }

  for (ObjectId i = 0; i < (2 ^ OBJECT_BIT_WIDTH); i++) {
    Triple triple = toTriple(1,2,i);
    assert(subjectIdFromTriple(triple) == 1);
    assert(predicateIdFromTriple(triple) == 2);
    assert(objectIdFromTriple(triple) == i);
  }
}

void testPredicateEntry() {
  PredicateEntry *entry = createPredicateEntry(2);

  for (SubjectId i = 1; i < (PREDICATE_ENTRY_INITIAL_ALLOCATION_LENGTH * 3); i++) {
    // printf("adding i = %ld\n", i);
    addToPredicateEntry(entry, i, 3);
  }

  assert(entry->entryCount == ((PREDICATE_ENTRY_INITIAL_ALLOCATION_LENGTH * 3) - 1));

  Triple triple = toTripleFromSOEntry(entry->soEntries[0], entry->predicate);
  printf("triple: %0.16llX\n", triple);
  assert(subjectIdFromTriple(triple) == 1);
  assert(predicateIdFromTriple(triple) == 2);
  assert(objectIdFromTriple(triple) == 3);

  triple = toTripleFromSOEntry(entry->soEntries[256], entry->predicate);
  printf("triple: %0.16llX\n", triple);
  assert(subjectIdFromTriple(triple) == 257);
  assert(predicateIdFromTriple(triple) == 2);
  assert(objectIdFromTriple(triple) == 3);

  PredicateEntryIterator *iterator = createPredicateEntryIterator();

  SubjectId i = 1;
  triple = iteratePredicateEntry(entry, iterator);
  while (!iterator->done) {
    printf("triple: %0.16llX\n", triple);
    assert(subjectIdFromTriple(triple) == i++);
    assert(predicateIdFromTriple(triple) == 2);
    assert(objectIdFromTriple(triple) == 3);
    triple = iteratePredicateEntry(entry, iterator);
  }

  assert(i == (PREDICATE_ENTRY_INITIAL_ALLOCATION_LENGTH * 3));

  free(iterator);
  freePredicateEntry(entry);
}

void initialize() {
  testTriple();
  testPredicateEntry();
}
