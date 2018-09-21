#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "graph.h"
// #include "quicksort.h"

void testTriple() {
  printf("testTriple\n");

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

int cmpfunc (const void * a, const void * b) {
   return ( subjectIdFromSOEntry(*(EntityPair *)a) - subjectIdFromSOEntry(*(EntityPair *)b) );
}

void testQuickSort() {
  printf("testQuickSort\n");

  int length = PREDICATE_ENTRY_INITIAL_ALLOCATION_LENGTH * 3;
  EntityPair arr[length];

  for (int i = 0; i < length; i++) {
    arr[i] = toSOEntry(length - i, 1);
  }

  // printf("before\n");
  // for (int i = 0; i < length; i++) {
  //   printf("%016llx\n", arr[i]);
  // }

  qsort(arr, length, sizeof(EntityPair), cmpfunc);

  // printf("after\n");
  // for (int i = 0; i < length; i++) {
  //   printf("%016llx\n", arr[i]);
  // }

  for (int i = 0; i < length; i++) {
    // printf("%d %llx %llx %lx\n", i, arr[i], toSOEntry(i + 1, 1), subjectIdFromSOEntry(arr[i]));
    assert(arr[i] == toSOEntry(i + 1, 1));
  }
}

void testPredicateEntry() {
  printf("testPredicateEntry\n");

  PredicateEntry *entry = createPredicateEntry(2);

  int length = PREDICATE_ENTRY_INITIAL_ALLOCATION_LENGTH * 3 + 1;

  SubjectId i = 1;

  while(i <= length) {
    addToPredicateEntry(entry, i++, 3);
  }

  assert(i == (length + 1));
  assert(entry->entryCount == length);

  optimizePredicateEntry(entry);

  assert(entry->entryCount == length);

  Iterator *iterator = createPredicateEntryIterator(entry);
  iterator->init(iterator);

  i = 1;

  Triple triple;
  while (iterate(iterator, &triple)) {
    // printf("i = %lx triple = %llx subjectId=%lx\n", i, triple, subjectIdFromTriple(triple));
    assert(subjectIdFromTriple(triple) == i++);
    assert(predicateIdFromTriple(triple) == 2);
    assert(objectIdFromTriple(triple) == 3);
  }

  // printf("i = %ld\n", i);
  assert(i == length + 1);

  iterator->free(iterator);
  // free(iterator);
  // freePredicateEntry(entry);
}

void testPredicateEntryORIterator() {
  printf("testPredicateEntryORIterator\n");

  PredicateEntry *aEntry = createPredicateEntry(2);
  PredicateEntry *bEntry = createPredicateEntry(3);

  SubjectId i = 1;

  for (; i < 5; i++) {
    addToPredicateEntry(aEntry, i, 10);
  }
  for (; i < 7; i++) {
    addToPredicateEntry(bEntry, i, 20);
  }

  optimizePredicateEntry(aEntry);
  optimizePredicateEntry(bEntry);

  Iterator *aIterator = createPredicateEntryIterator(aEntry);
  Iterator *bIterator = createPredicateEntryIterator(bEntry);
  Iterator *iterator = createPredicateEntryORIterator(aIterator, bIterator);
  iterator->init(iterator);

  i = 1;

  Triple triple;
  while (iterate(iterator, &triple)) {
    // printf("triple: %ld,%ld,%ld\n", subjectIdFromTriple(triple), predicateIdFromTriple(triple), objectIdFromTriple(triple));
    assert(subjectIdFromTriple(triple) == i++);
    if (subjectIdFromTriple(triple) < 5) {
      assert(predicateIdFromTriple(triple) == 2);
      assert(objectIdFromTriple(triple) == 10);
    } else {
      assert(predicateIdFromTriple(triple) == 3);
      assert(objectIdFromTriple(triple) == 20);
    }
  }

  // printf("i = %ld\n", i);
  assert(i == 7);

  iterator->free(iterator);
}

void testPredicateEntryORIteratorNested() {
  printf("testPredicateEntryORIteratorNested\n");

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

  optimizePredicateEntry(aEntry);
  optimizePredicateEntry(bEntry);
  optimizePredicateEntry(cEntry);
  optimizePredicateEntry(dEntry);

  Iterator *aIterator = createPredicateEntryIterator(aEntry);
  Iterator *bIterator = createPredicateEntryIterator(bEntry);
  Iterator *iterator1 = createPredicateEntryORIterator(aIterator, bIterator);

  Iterator *cIterator = createPredicateEntryIterator(cEntry);
  Iterator *dIterator = createPredicateEntryIterator(dEntry);
  Iterator *iterator2 = createPredicateEntryORIterator(cIterator, dIterator);

  Iterator *iterator = createPredicateEntryORIterator(iterator1, iterator2);
  iterator->init(iterator);

  i = 1;

  Triple triple;
  while (iterate(iterator, &triple)) {
    // printf("triple: %ld,%ld,%ld\n", subjectIdFromTriple(triple), predicateIdFromTriple(triple), objectIdFromTriple(triple));
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
  }

  // printf("i = %ld\n", i);
  assert(i == 9);

  iterator->free(iterator);
}

void testPredicateEntryANDIterator() {
  printf("testPredicateEntryANDIterator\n");

  PredicateEntry *aEntry = createPredicateEntry(2);
  PredicateEntry *bEntry = createPredicateEntry(2);

  for (SubjectId i = 2; i < 4; i++) {
    addToPredicateEntry(aEntry, i, 10);
  }
  for (SubjectId i = 1; i < 5; i++) {
    addToPredicateEntry(bEntry, i, 10);
  }

  optimizePredicateEntry(aEntry);
  optimizePredicateEntry(bEntry);

  Iterator *aIterator = createPredicateEntryIterator(aEntry);
  Iterator *bIterator = createPredicateEntryIterator(bEntry);
  Iterator *iterator = createPredicateEntryANDIterator(aIterator, bIterator);
  iterator->init(iterator);

  SubjectId i = 2;

  Triple triple;
  while (iterate(iterator, &triple)) {
    // printf("triple: %ld,%ld,%ld\n", subjectIdFromTriple(triple), predicateIdFromTriple(triple), objectIdFromTriple(triple));
    assert(subjectIdFromTriple(triple) == i++);
    assert(predicateIdFromTriple(triple) == 2);
    assert(objectIdFromTriple(triple) == 10);
  }

  // printf("i = %ld\n", i);
  assert(i == 4);

  iterator->free(iterator);
}

void testGlobalAssertions() {
  printf("testGlobalAssertions\n");

  printf("sizeof(short): %d\n", (int) sizeof(short));
  printf("sizeof(int): %d\n", (int) sizeof(int));
  printf("sizeof(unsigned int): %d\n", (int) sizeof(unsigned int));
  printf("sizeof(long): %d\n", (int) sizeof(long));
  printf("sizeof(unsigned long): %d\n", (int) sizeof(unsigned long));
  printf("sizeof(long long): %d\n", (int) sizeof(long long));
  printf("sizeof(unsigned long long): %d\n", (int) sizeof(unsigned long long));
  printf("sizeof(size_t): %d\n", (int) sizeof(size_t));
  printf("sizeof(void *): %d\n", (int) sizeof(void *));
  printf("\n");
  printf("sizeof(EntityId): %ld\n", sizeof(EntityId));
  printf("sizeof(PredicateId): %ld\n", sizeof(PredicateId));
  printf("sizeof(EntityPair): %ld\n", sizeof(EntityPair));
  printf("sizeof(Triple): %ld\n", sizeof(Triple));
  printf("\n");
  assert(sizeof(EntityId) * 2 <= sizeof(EntityPair));
}

int main(void) {
  testGlobalAssertions();
  testTriple();
  testQuickSort();
  testPredicateEntry();
  testPredicateEntryORIterator();
  testPredicateEntryORIteratorNested();
  testPredicateEntryANDIterator();
}
