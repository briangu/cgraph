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

typedef unsigned long SubjectId;
typedef unsigned long PredicateId;
typedef unsigned long ObjectId;
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

typedef Triple (*iterateFn)(Iterator *iterator);
typedef Triple (*peekFn)(Iterator *iterator);
typedef BOOL (*doneFn)(Iterator *iterator);

struct Iterator_t {
  iterateFn iterate;
  peekFn peek;
  doneFn done;
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
} PredicateEntryJoinIterator;

Iterator* createPredicateEntryORIterator(Iterator *aIterator, Iterator *bIterator);
Iterator* createPredicateEntryANDIterator(Iterator *aIterator, Iterator *bIterator);
void freePredicateEntryJoinIterator(PredicateEntryJoinIterator *iterator);

void initialize();
