#ifndef TRIPLE_H_INCLUDED
#define TRIPLE_H_INCLUDED

typedef unsigned char BOOL;
#define TRUE 1;
#define FALSE 0;

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

#define TOTAL_BIT_COUNT (SUBJECT_BIT_WIDTH + PREDICATE_BIT_WIDTH + OBJECT_BIT_WIDTH)

#if (TOTAL_BIT_COUNT && (TOTAL_BIT_COUNT & (TOTAL_BIT_COUNT - 1)))
// #pragma message ( "correcting for extra bits" )
#define EXTRA_MSB_MASK (~(~((Triple)0) << (SUBJECT_BIT_WIDTH + PREDICATE_BIT_WIDTH + OBJECT_BIT_WIDTH)))
#define SUBJECT_MASK ((EXTRA_MSB_MASK & ~((Triple)0)) << (PREDICATE_BIT_WIDTH + OBJECT_BIT_WIDTH))
#define OBJECT_MASK ((EXTRA_MSB_MASK & ~((Triple)0)) >> (SUBJECT_BIT_WIDTH + PREDICATE_BIT_WIDTH))
#define PREDICATE_MASK (EXTRA_MSB_MASK & (~SUBJECT_MASK & ~OBJECT_MASK))
#else
// #pragma message ( "bits are multiple of 2" )
#define SUBJECT_MASK (~((Triple)0) << (PREDICATE_BIT_WIDTH + OBJECT_BIT_WIDTH))
#define OBJECT_MASK (~((Triple)0) >> (SUBJECT_BIT_WIDTH + PREDICATE_BIT_WIDTH))
#define PREDICATE_MASK (~SUBJECT_MASK & ~OBJECT_MASK)
#endif

SubjectId subjectIdFromTriple(Triple triple);
PredicateId predicateIdFromTriple(Triple triple);
ObjectId objectIdFromTriple(Triple triple);
Triple toTriple(SubjectId subject, PredicateId predicate, ObjectId object);

#endif
