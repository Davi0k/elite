#ifndef VM_H
#define VM_H

#include "utilities/chunk.h"
#include "types/value.h"
#include "utilities/table.h"

#define STACK_MAX 256

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} Results;

typedef struct {
  Value content[STACK_MAX];
  Value* top;
} Stack;

void reset_stack(Stack* stack);

void push(Stack* stack, Value value);
Value pop(Stack* stack, int distance);

typedef struct {
  Chunk* chunk;
  Stack stack;

  Object* objects;

  Table strings, globals;
} VM;

void initialize_VM(VM* vm);
void free_VM(VM* vm);

Results interpret(VM* vm, const char* source);

#endif