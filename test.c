#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "graph.h"

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

  SubjectId i = 1;

  for (; i <= (PREDICATE_ENTRY_INITIAL_ALLOCATION_LENGTH * 3); i++) {
    addToPredicateEntry(entry, i, 3);
  }

  assert(i == ((PREDICATE_ENTRY_INITIAL_ALLOCATION_LENGTH * 3) + 1));
  assert(entry->entryCount == ((PREDICATE_ENTRY_INITIAL_ALLOCATION_LENGTH * 3)));

  Iterator *iterator = createPredicateEntryIterator(entry);

  i = 1;

  Triple triple = iterator->iterate(iterator);
  while (!iterator->done(iterator)) {
    assert(subjectIdFromTriple(triple) == i++);
    assert(predicateIdFromTriple(triple) == 2);
    assert(objectIdFromTriple(triple) == 3);
    triple = iterator->iterate(iterator);
  }

  assert(i == (PREDICATE_ENTRY_INITIAL_ALLOCATION_LENGTH * 3));

  free(iterator);
  freePredicateEntry(entry);
}

void testPredicateEntryORIterator() {
  PredicateEntry *aEntry = createPredicateEntry(2);
  PredicateEntry *bEntry = createPredicateEntry(3);

  SubjectId i = 1;

  for (; i < 3; i++) {
    addToPredicateEntry(aEntry, i, 10);
  }
  for (; i < 5; i++) {
    addToPredicateEntry(bEntry, i, 20);
  }

  Iterator *aIterator = createPredicateEntryIterator(aEntry);
  Iterator *bIterator = createPredicateEntryIterator(bEntry);
  Iterator *iterator = createPredicateEntryORIterator(aIterator, bIterator);

  i = 1;

  Triple triple = iterator->iterate(iterator);
  // printf("triple: %ld,%ld,%ld\n", subjectIdFromTriple(triple), predicateIdFromTriple(triple), objectIdFromTriple(triple));
  while (!iterator->done(iterator)) {
    assert(subjectIdFromTriple(triple) == i++);
    if (subjectIdFromTriple(triple) < 3) {
      assert(predicateIdFromTriple(triple) == 2);
      assert(objectIdFromTriple(triple) == 10);
    } else {
      assert(predicateIdFromTriple(triple) == 3);
      assert(objectIdFromTriple(triple) == 20);
    }
    triple = iterator->iterate(iterator);
    // printf("triple: %ld,%ld,%ld\n", subjectIdFromTriple(triple), predicateIdFromTriple(triple), objectIdFromTriple(triple));
  }

  // printf("i = %ld\n", i);
  assert(i == 2*2);

  free(aIterator);
  free(bIterator);
  free(iterator);
  freePredicateEntry(aEntry);
  freePredicateEntry(bEntry);
}

void testPredicateEntryORIteratorNested() {
  PredicateEntry *aEntry = createPredicateEntry(2);
  PredicateEntry *bEntry = createPredicateEntry(3);
  PredicateEntry *cEntry = createPredicateEntry(4);
  PredicateEntry *dEntry = createPredicateEntry(5);

  SubjectId i = 1;

  for (; i < 3; i++) {
    addToPredicateEntry(aEntry, i, 10);
  }
  for (; i < 5; i++) {
    addToPredicateEntry(bEntry, i, 20);
  }
  for (; i < 7; i++) {
    addToPredicateEntry(cEntry, i, 30);
  }
  for (; i < 9; i++) {
    addToPredicateEntry(dEntry, i, 40);
  }

  Iterator *aIterator = createPredicateEntryIterator(aEntry);
  Iterator *bIterator = createPredicateEntryIterator(bEntry);
  Iterator *iterator1 = createPredicateEntryORIterator(aIterator, bIterator);

  Iterator *cIterator = createPredicateEntryIterator(cEntry);
  Iterator *dIterator = createPredicateEntryIterator(dEntry);
  Iterator *iterator2 = createPredicateEntryORIterator(cIterator, dIterator);

  Iterator *iterator = createPredicateEntryORIterator(iterator1, iterator2);

  i = 1;

  Triple triple = iterator->iterate(iterator);
  // printf("triple: %ld,%ld,%ld\n", subjectIdFromTriple(triple), predicateIdFromTriple(triple), objectIdFromTriple(triple));
  while (!iterator->done(iterator)) {
    assert(subjectIdFromTriple(triple) == i++);
    if (subjectIdFromTriple(triple) < 3) {
      assert(predicateIdFromTriple(triple) == 2);
      assert(objectIdFromTriple(triple) == 10);
    } else if (subjectIdFromTriple(triple) < 5) {
      assert(predicateIdFromTriple(triple) == 3);
      assert(objectIdFromTriple(triple) == 20);
    } else if (subjectIdFromTriple(triple) < 7) {
      assert(predicateIdFromTriple(triple) == 4);
      assert(objectIdFromTriple(triple) == 30);
    } else {
      assert(predicateIdFromTriple(triple) == 5);
      assert(objectIdFromTriple(triple) == 40);
    }
    triple = iterator->iterate(iterator);
    // printf("triple: %ld,%ld,%ld\n", subjectIdFromTriple(triple), predicateIdFromTriple(triple), objectIdFromTriple(triple));
  }

  // printf("i = %ld\n", i);
  assert(i == 2*4);

  free(aIterator);
  free(bIterator);
  free(iterator);
  freePredicateEntry(aEntry);
  freePredicateEntry(bEntry);
}

int main(void) {
  testTriple();
  testPredicateEntry();
  testPredicateEntryORIterator();
  testPredicateEntryORIteratorNested();
}
