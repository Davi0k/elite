#include "vm.h"
#include "types/stack.h"

void push(Stack* stack, Value value) {
  *stack->top = value;

  stack->top++;
}

Value pop(Stack* stack, int count) {
  stack->top = stack->top - count;

  return *stack->top;
}

Value peek(Stack* stack, int distance) {
  return stack->top[- 1 - distance];
}