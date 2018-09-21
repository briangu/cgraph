#ifndef TRIPLE_H_INCLUDED
#define TRIPLE_H_INCLUDED

typedef unsigned char BOOL;
#define TRUE 1;
#define FALSE 0;

typedef unsigned int EntityId;
typedef EntityId SubjectId;
typedef unsigned int PredicateId;
typedef EntityId ObjectId;

typedef unsigned long long Triple;

#define SUBJECT_BIT_WIDTH 22
#define PREDICATE_BIT_WIDTH 20
#define OBJECT_BIT_WIDTH 22

#define SUBJECT_MASK (~((Triple)0) << (PREDICATE_BIT_WIDTH + OBJECT_BIT_WIDTH))
#define PREDICATE_MASK (~SUBJECT_MASK & ((~((Triple)0) >> OBJECT_BIT_WIDTH) << OBJECT_BIT_WIDTH))
#define OBJECT_MASK (~SUBJECT_MASK & ~PREDICATE_MASK)

SubjectId subjectIdFromTriple(Triple triple);
PredicateId predicateIdFromTriple(Triple triple);
ObjectId objectIdFromTriple(Triple triple);
Triple toTriple(SubjectId subject, PredicateId predicate, ObjectId object);

#endif
