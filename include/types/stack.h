#ifndef STACK_H
#define STACK_H

#include "common.h"

#include "types/value.h"

typedef struct Stack {
  Value content[1024 * 64];
  
  Value* top;
} Stack;

void push(Stack* stack, Value value);

Value pop(Stack* stack, int count);

Value peek(Stack* stack, int distance);

#endif