#ifndef TRIPLE_H_INCLUDED
#define TRIPLE_H_INCLUDED

typedef unsigned char BOOL;
#define TRUE ((BOOL)1);
#define FALSE ((BOOL)0);

typedef unsigned int EntityId;
typedef EntityId SubjectId;
typedef unsigned int PredicateId;
typedef EntityId ObjectId;

// NOTE: must be big enough to hold 2 * sizeof(EntityId) + sizeof(PredicateId)
typedef unsigned long long Triple;

// NOTE: these widths must fit into EntityId
#define SUBJECT_BIT_WIDTH 22
#define OBJECT_BIT_WIDTH 22

// NOTE: these widths must fit into PredicateId
#define PREDICATE_BIT_WIDTH 20

#define TRIPLE_MASK ((Triple)~((Triple)0))
#define TRIPLE_BIT_COUNT (SUBJECT_BIT_WIDTH + PREDICATE_BIT_WIDTH + OBJECT_BIT_WIDTH)

#if (TRIPLE_BIT_COUNT && (TRIPLE_BIT_COUNT & (TRIPLE_BIT_COUNT - 1)))
// #pragma message ( "correcting for extra bits" )
#define EXTRA_MSB_MASK ((Triple)~(TRIPLE_MASK << (SUBJECT_BIT_WIDTH + PREDICATE_BIT_WIDTH + OBJECT_BIT_WIDTH)))
#define SUBJECT_MASK ((EXTRA_MSB_MASK & TRIPLE_MASK) << (PREDICATE_BIT_WIDTH + OBJECT_BIT_WIDTH))
#define OBJECT_MASK ((EXTRA_MSB_MASK & TRIPLE_MASK) >> (SUBJECT_BIT_WIDTH + PREDICATE_BIT_WIDTH))
#define PREDICATE_MASK (EXTRA_MSB_MASK & ((Triple)~((Triple)SUBJECT_MASK) & (Triple)~((Triple)OBJECT_MASK)))
#else
// #pragma message ( "bits are multiple of 2" )
#define SUBJECT_MASK (TRIPLE_MASK << (PREDICATE_BIT_WIDTH + OBJECT_BIT_WIDTH))
#define OBJECT_MASK (TRIPLE_MASK >> (SUBJECT_BIT_WIDTH + PREDICATE_BIT_WIDTH))
#define PREDICATE_MASK ((Triple)~((Triple)SUBJECT_MASK) & (Triple)~((Triple)OBJECT_MASK))
#endif

SubjectId subjectIdFromTriple(Triple triple);
PredicateId predicateIdFromTriple(Triple triple);
ObjectId objectIdFromTriple(Triple triple);
Triple toTriple(SubjectId subject, PredicateId predicate, ObjectId object);

#endif
