/*

  Graph structure:

  64 bits per triple:

  subject:    32 bit
  predicate:  32 bit
  object:     32 bit

  alternatively, we can pack everything into a 64-bit int if the number of objects aren't that great.

*/

typedef unsigned char BOOL;
#define TRUE 1;
#define FALSE 0;

typedef unsigned long EntityId;
typedef EntityId SubjectId;
typedef unsigned long PredicateId;
typedef EntityId ObjectId;
typedef unsigned long long EntityPair;
typedef unsigned long long Triple;

#define SUBJECT_BIT_WIDTH 22
#define PREDICATE_BIT_WIDTH 20
#define OBJECT_BIT_WIDTH 22

#define SUBJECT_MASK (~((SubjectId)0) << (PREDICATE_BIT_WIDTH + OBJECT_BIT_WIDTH))
#define PREDICATE_MASK (~SUBJECT_MASK & ((~((PredicateId)0) >> OBJECT_BIT_WIDTH) << OBJECT_BIT_WIDTH))
#define OBJECT_MASK (~SUBJECT_MASK & ~PREDICATE_MASK)

#define PREDICATE_ENTRY_INITIAL_ALLOCATION_LENGTH 1024

typedef struct {
  PredicateId predicate;

  unsigned long entryCount;
  unsigned long currentEntriesLength;

  EntityPair *soEntries;
  EntityPair *osEntries;

} PredicateEntry;

SubjectId subjectIdFromTriple(Triple triple);
PredicateId predicateIdFromTriple(Triple triple);
ObjectId objectIdFromTriple(Triple triple);
EntityPair toSOEntry(SubjectId subject, ObjectId object);
EntityPair toOSEntry(ObjectId object, SubjectId subject);
EntityPair tripleToSOEntry(Triple triple);
EntityPair tripleToOSEntry(Triple triple);
Triple toTriple(SubjectId subject, PredicateId predicate, ObjectId object);
Triple toTripleFromSOEntry(EntityPair soPair, PredicateId predicate);
Triple toTripleFromOSEntry(EntityPair osPair, PredicateId predicate);

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

typedef struct {
  unsigned int TYPE;
} FilterState;

typedef struct {
  unsigned int TYPE;
  EntityId value;
} FilterStateEqual;

typedef struct {
  unsigned int TYPE;
  EntityId begin;
  EntityId end;
} FilterStateRangeQuery;

#define FILTER_TYPE_EQUAL ((unsigned int)1)
#define FILTER_TYPE_RANGE_QUERY ((unsigned int)(FILTER_TYPE_EQUAL + 1))

typedef BOOL (*filterFn)(FilterState *state, Triple triple);

BOOL filter_PASSTHRU(FilterState *state, Triple triple);
BOOL filter_S(FilterState *state, Triple triple);
BOOL filter_O(FilterState *state, Triple triple);

typedef int (*comparatorFn)(Iterator *aIterator, Iterator *bIterator);

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
  comparatorFn compare;
};

typedef struct {
  Iterator fn;
  PredicateEntry *entry;
  unsigned long position;
  BOOL done;
  filterFn filter;
  FilterState *filterState;
} PredicateEntryIterator;

Iterator* createPredicateEntryIterator(PredicateEntry *entry, filterFn filter, FilterState *filterState);

typedef struct {
  Iterator fn;
  Iterator *aIterator;
  Iterator *bIterator;
  Iterator *currentIterator;
  comparatorFn comparator;
} PredicateEntryJoinIterator;

Iterator* createPredicateEntryORIterator(Iterator *aIterator, Iterator *bIterator);
Iterator* createPredicateEntryANDIterator(Iterator *aIterator, Iterator *bIterator, comparatorFn comparator);

void initialize();
