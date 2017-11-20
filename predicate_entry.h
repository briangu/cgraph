#include "triple.h"

typedef unsigned char BOOL;
#define TRUE 1;
#define FALSE 0;

typedef unsigned long long EntityPair;

EntityPair toSOEntry(SubjectId subject, ObjectId object);
EntityPair toOSEntry(ObjectId object, SubjectId subject);
EntityPair tripleToSOEntry(Triple triple);
EntityPair tripleToOSEntry(Triple triple);
Triple toTripleFromSOEntry(EntityPair soPair, PredicateId predicate);
Triple toTripleFromOSEntry(EntityPair osPair, PredicateId predicate);

#define PREDICATE_ENTRY_INITIAL_ALLOCATION_LENGTH 1024

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

struct Iterator_t;
typedef struct Iterator_t Iterator;

typedef void (*advanceFn)(Iterator *iterator);
typedef void (*nextOperandFn)(Iterator *iterator);
typedef Triple (*peekFn)(Iterator *iterator);
typedef BOOL (*doneFn)(Iterator *iterator);
typedef void (*initFn)(Iterator *iterator);
typedef void (*freeFn)(Iterator *iterator);

#define ENTRY_ITERATOR  ((unsigned char)1)
#define JOIN_ITERATOR   ((unsigned char)2)

BOOL iterate(Iterator *iterator, Triple *triple);

struct Iterator_t {
  unsigned char TYPE;
  advanceFn advance;
  nextOperandFn nextOperand;
  peekFn peek;
  doneFn done;
  initFn init;
  freeFn free;
};

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
