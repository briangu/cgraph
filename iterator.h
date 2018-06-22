#ifndef ITERATOR_H_INCLUDED
#define ITERATOR_H_INCLUDED

#include "triple.h"

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

#endif
