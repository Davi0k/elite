#ifndef STACK_H
#define STACK_H

#include "common.h"

#include "types/value.h"

#define STACK_DEFAULT_SIZE ( 1024 * 64 )

typedef struct {
  Value content[STACK_DEFAULT_SIZE];
  Value* top;
} Stack;

void push(Stack* stack, Value value);

Value pop(Stack* stack, int distance);

Value peek(Stack* stack, int distance);

#endif