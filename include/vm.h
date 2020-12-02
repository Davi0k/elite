#ifndef VM_H
#define VM_H

#include <stdlib.h>

#include "common.h"

#include "compiler.h"
#include "utilities/chunk.h"
#include "utilities/table.h"
#include "utilities/native.h"
#include "helpers/stack.h"
#include "types/value.h"

#define FRAME_DEFAULT_SIZE 16

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} Results;

typedef struct {
  Closure* closure;
  uint8_t* ip;
  Value* slots;
} Frame;

typedef struct VM {
  Frame* frames;
  int capacity;
  int count;

  size_t allocate, threshold;

  Stack stack;

  Table strings, globals;

  Upvalue* upvalues;

  Object* objects;

  Parser* parser;
} VM;

void reset(VM* vm);

void initialize_VM(VM* vm);
void free_VM(VM* vm);

void native(VM* vm, const char* identifier, Internal internal);

Results interpret(VM* vm, const char* source);


#endif