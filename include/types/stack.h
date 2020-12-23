#ifndef STACK_H
#define STACK_H

#include "common.h"

#include "types/value.h"

#define STACK_DEFAULT_SIZE 64 * 1024

typedef struct Stack {
  Value content[STACK_DEFAULT_SIZE]; 

  Value* top;
} Stack;


void push(Stack* stack, Value value);

Value pop(Stack* stack, int count);

Value peek(Stack* stack, int distance);

#endif