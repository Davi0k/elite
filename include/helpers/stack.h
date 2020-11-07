#ifndef STACK_H
#define STACK_H

#include "common.h"
#include "types/value.h"

typedef struct {
  Value content[STACK_MAX];
  Value* top;
} Stack;

void push(Stack* stack, Value value);

Value pop(Stack* stack, int distance);

Value peek(Stack* stack, int distance);

#endif