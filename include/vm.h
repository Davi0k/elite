#ifndef VM_H
#define VM_H

#include "common.h"
#include "utilities/chunk.h"
#include "utilities/table.h"
#include "utilities/native.h"
#include "helpers/stack.h"
#include "types/value.h"

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} Results;

typedef struct {
  Function* function;
  uint8_t* ip;
  Value* slots;
} Frame;

typedef struct {
  Frame frames[FRAMES_MAX];

  int count;

  Stack stack;

  Object* objects;

  Table strings, globals;
} VM;

void reset_stack(VM* vm);

void native(VM* vm, const char* identifier, Internal internal);

void initialize_VM(VM* vm);
void free_VM(VM* vm);

Results interpret(VM* vm, const char* source);

extern Function* compile(VM* vm, const char* source);

#endif