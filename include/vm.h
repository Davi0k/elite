#ifndef VM_H
#define VM_H

#include <stdlib.h>

#include "common.h"

#include "utilities/chunk.h"
#include "utilities/table.h"
#include "utilities/native.h"
#include "types/stack.h"
#include "types/value.h"

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

typedef struct {
  Frame* frames;
  int capacity;
  int count;
} Call;

typedef struct VM {
  size_t allocate, threshold;

  Call call;

  Stack stack;

  Table strings, globals;

  Upvalue* upvalues;

  Object* objects;
} VM;

void initialize_VM(VM* vm);
void free_VM(VM* vm);

void native(VM* vm, const char* identifier, Internal internal);

Results interpret(VM* vm, const char* source);

#endif