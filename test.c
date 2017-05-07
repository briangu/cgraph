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
  printf("testPredicateEntry\n");

  PredicateEntry *entry = createPredicateEntry(2);

  SubjectId i = 1;

  for (; i <= (PREDICATE_ENTRY_INITIAL_ALLOCATION_LENGTH * 3); i++) {
    addToPredicateEntry(entry, i, 3);
  }

  assert(i == ((PREDICATE_ENTRY_INITIAL_ALLOCATION_LENGTH * 3) + 1));
  assert(entry->entryCount == ((PREDICATE_ENTRY_INITIAL_ALLOCATION_LENGTH * 3)));

  Iterator *iterator = createPredicateEntryIterator(entry, filter_PASSTHRU, NULL);
  iterator->init(iterator);

  i = 1;

  Triple triple;
  while (iterate(iterator, &triple)) {
    assert(subjectIdFromTriple(triple) == i++);
    assert(predicateIdFromTriple(triple) == 2);
    assert(objectIdFromTriple(triple) == 3);
  }

  assert(i == (PREDICATE_ENTRY_INITIAL_ALLOCATION_LENGTH * 3) + 1);

  iterator->free(iterator);
}

void testPredicateEntryRangeQuery() {
  printf("testPredicateEntryRangeQuery\n");

  FilterStateRangeQuery rqstate;
  rqstate.TYPE = FILTER_TYPE_RANGE_QUERY;
  rqstate.begin = 3;
  rqstate.end = 10;

  assert(!filter_S((FilterState *)&rqstate, toTriple(1,2,3)));
  assert(filter_S((FilterState *)&rqstate, toTriple(3,2,3)));
  assert(filter_S((FilterState *)&rqstate, toTriple(5,2,3)));
  assert(filter_S((FilterState *)&rqstate, toTriple(10,2,3)));
  assert(!filter_S((FilterState *)&rqstate, toTriple(11,2,3)));

  PredicateEntry *entry = createPredicateEntry(2);

  SubjectId i = 1;

  for (; i <= 20; i++) {
    addToPredicateEntry(entry, i, 3);
  }

  assert(i == 20 + 1);
  assert(entry->entryCount == 20);

  Iterator *iterator = createPredicateEntryIterator(entry, filter_S, (FilterState *)&rqstate);
  iterator->init(iterator);

  i = 3;

  Triple triple;
  while (iterate(iterator, &triple)) {
    assert(subjectIdFromTriple(triple) == i++);
    assert(predicateIdFromTriple(triple) == 2);
    assert(objectIdFromTriple(triple) == 3);
  }

  assert(i == 10 + 1);

  iterator->free(iterator);
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

  Iterator *aIterator = createPredicateEntryIterator(aEntry, filter_PASSTHRU, NULL);
  Iterator *bIterator = createPredicateEntryIterator(bEntry, filter_PASSTHRU, NULL);
  Iterator *iterator = createPredicateEntryORIterator(aIterator, bIterator);
  iterator->init(iterator);

  i = 1;

  Triple triple;
  while (iterate(iterator, &triple)) {
    assert(subjectIdFromTriple(triple) == i++);
    if (subjectIdFromTriple(triple) < 5) {
      assert(predicateIdFromTriple(triple) == 2);
      assert(objectIdFromTriple(triple) == 10);
    } else {
      assert(predicateIdFromTriple(triple) == 3);
      assert(objectIdFromTriple(triple) == 20);
    }
  }

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

  Iterator *aIterator = createPredicateEntryIterator(aEntry, filter_PASSTHRU, NULL);
  Iterator *bIterator = createPredicateEntryIterator(bEntry, filter_PASSTHRU, NULL);
  Iterator *iterator1 = createPredicateEntryORIterator(aIterator, bIterator);

  Iterator *cIterator = createPredicateEntryIterator(cEntry, filter_PASSTHRU, NULL);
  Iterator *dIterator = createPredicateEntryIterator(dEntry, filter_PASSTHRU, NULL);
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

// void testPredicateEntryANDIterator1() {
//   printf("testPredicateEntryANDIterator1\n");

//   PredicateEntry *aEntry = createPredicateEntry(2);
//   PredicateEntry *bEntry = createPredicateEntry(2);

//   addToPredicateEntry(aEntry, 1, 10);
//   addToPredicateEntry(bEntry, 1, 10);

//   Iterator *aIterator = createPredicateEntryIterator(aEntry);
//   Iterator *bIterator = createPredicateEntryIterator(bEntry);
//   Iterator *iterator = createPredicateEntryANDIterator(aIterator, bIterator);
//   iterator->init(iterator);

//   SubjectId i = 2;

//   Triple triple;
//   while (iterate(iterator, &triple)) {
//     // printf("triple: %ld,%ld,%ld\n", subjectIdFromTriple(triple), predicateIdFromTriple(triple), objectIdFromTriple(triple));
//     assert(subjectIdFromTriple(triple) == 1);
//     assert(predicateIdFromTriple(triple) == 2);
//     assert(objectIdFromTriple(triple) == 10);
//   }

//   // printf("i = %ld\n", i);
//   assert(i == 2);

//   iterator->free(iterator);
// }

// void testPredicateEntryANDIterator2() {
//   printf("testPredicateEntryANDIterator2\n");

//   PredicateEntry *aEntry = createPredicateEntry(2);
//   PredicateEntry *bEntry = createPredicateEntry(3);

//   for (SubjectId i = 2; i < 4; i++) {
//     addToPredicateEntry(aEntry, i, 10);
//   }
//   for (SubjectId i = 1; i < 5; i++) {
//     addToPredicateEntry(bEntry, i, 10);
//   }

//   Iterator *aIterator = createPredicateEntryIterator(aEntry);
//   Iterator *bIterator = createPredicateEntryIterator(bEntry);
//   Iterator *iterator = createPredicateEntryANDIterator(aIterator, bIterator);
//   iterator->init(iterator);

//   Triple triple;
//   while (iterate(iterator, &triple)) {
//     assert(0);
//   }

//   iterator->free(iterator);
// }

// TODO:
// add scan operand
// rename nextOperand to nextIterator
// rewrite tests to use scan operand on single entry
// add new tests for multiple entries and do a join across multiple entries

// void testPredicateEntryANDIterator3() {
//   printf("testPredicateEntryANDIterator3\n");

//   PredicateEntry *aEntry = createPredicateEntry(2);
//   PredicateEntry *bEntry = createPredicateEntry(2);

//   for (SubjectId i = 2; i < 4; i++) {
//     addToPredicateEntry(aEntry, i, 10);
//   }
//   for (SubjectId i = 1; i < 5; i++) {
//     addToPredicateEntry(bEntry, i, 10);
//   }

//   Iterator *aIterator = createPredicateEntryIterator(aEntry);
//   Iterator *bIterator = createPredicateEntryIterator(bEntry);
//   Iterator *iterator = createPredicateEntryANDIterator(aIterator, bIterator);
//   iterator->init(iterator);

//   SubjectId i = 2;

//   Triple triple;
//   while (iterate(iterator, &triple)) {
//     // printf("triple: %ld,%ld,%ld\n", subjectIdFromTriple(triple), predicateIdFromTriple(triple), objectIdFromTriple(triple));
//     assert(subjectIdFromTriple(triple) == i++);
//     assert(predicateIdFromTriple(triple) == 2);
//     assert(objectIdFromTriple(triple) == 10);
//   }

//   // printf("i = %ld\n", i);
//   assert(i == 4);

//   iterator->free(iterator);
// }

int main(void) {
  testTriple();
  testPredicateEntry();
  testPredicateEntryRangeQuery();
  testPredicateEntryORIterator();
  testPredicateEntryORIteratorNested();
  // testPredicateEntryANDIterator1();
  //  // testPredicateEntryANDIterator2();
  // testPredicateEntryANDIterator3();
}
