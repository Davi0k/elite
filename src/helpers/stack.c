#include "helpers/stack.h"

void push(Stack* stack, Value value) {
  *stack->top = value;

  stack->top++;
}

Value pop(Stack* stack, int distance) {
  stack->top = stack->top - distance;

  return *stack->top;
}

Value peek(Stack* stack, int distance) {
  return stack->top[- 1 - distance];
}