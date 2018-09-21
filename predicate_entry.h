#ifndef PREDICATE_ENTRY_H_INCLUDED
#define PREDICATE_ENTRY_H_INCLUDED

#include "triple.h"
#include "iterator.h"

// EntityPair must be wide enough to hold sizeof(EntityId) * 2
typedef unsigned long long EntityPair;

#define ENTITY_PAIR_BIT_COUNT (sizeof(EntityPair) * 8)
#define ENTITY_PAIR_HALF_BIT_COUNT (ENTITY_PAIR_BIT_COUNT >> 1)
#define ENTITY_PAIR_HALF_MASK ((EntityPair)~((EntityPair)0) >> ENTITY_PAIR_HALF_BIT_COUNT)

EntityPair toSOEntry(SubjectId subject, ObjectId object);
EntityPair toOSEntry(ObjectId object, SubjectId subject);
SubjectId subjectIdFromSOEntry(EntityPair pair);
ObjectId objectIdFromSOEntry(EntityPair pair);
SubjectId subjectIdFromOSEntry(EntityPair pair);
ObjectId objectIdFromOSEntry(EntityPair pair);
EntityPair tripleToSOEntry(Triple triple);
EntityPair tripleToOSEntry(Triple triple);
Triple toTripleFromSOEntry(EntityPair soPair, PredicateId predicate);
Triple toTripleFromOSEntry(EntityPair osPair, PredicateId predicate);

#define PREDICATE_ENTRY_INITIAL_ALLOCATION_LENGTH 16

typedef struct {
  PredicateId predicate;

  unsigned long entryCount;
  unsigned long currentEntriesLength;

  EntityPair *soEntries;
  EntityPair *osEntries;

} PredicateEntry;

PredicateEntry *createPredicateEntry(PredicateId predicate);
void freePredicateEntry(PredicateEntry *entry);

void growPredicateEntry(PredicateEntry *entry);
void addToPredicateEntry(PredicateEntry *entry, SubjectId subject, ObjectId object);
void optimizePredicateEntry(PredicateEntry *entry);

typedef struct {
  Iterator fn;
  PredicateEntry *entry;
  unsigned long position;
  BOOL done;
} PredicateEntryIterator;

Iterator* createPredicateEntryIterator(PredicateEntry *entry);
void freePredicateEntryIterator(PredicateEntryIterator *iterator);

typedef struct {
  Iterator fn;
  Iterator *aIterator;
  Iterator *bIterator;
  Iterator *currentIterator;
} PredicateEntryJoinIterator;

Iterator* createPredicateEntryORIterator(Iterator *aIterator, Iterator *bIterator);
Iterator* createPredicateEntryANDIterator(Iterator *aIterator, Iterator *bIterator);
void freePredicateEntryJoinIterator(PredicateEntryJoinIterator *iterator);

#endif
