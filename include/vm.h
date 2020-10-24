#ifndef VM_H
#define VM_H

#include "utils/chunk.h"
#include "utils/value.h"

#define STACK_MAX 256

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} Results;

typedef struct {
  Chunk* chunk;
  
  Value stack[STACK_MAX];
  Value* top;
} VM;

void initialize_VM(VM* vm);
void free_VM(VM* vm);

void push(VM* vm, Value value);
Value pop(VM* vm);

Results interpret(VM* vm, Chunk* chunk);

#endif