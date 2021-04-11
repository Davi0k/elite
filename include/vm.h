#ifndef VM_H
#define VM_H

#include <stdlib.h>

#include "common.h"

#include "utilities/chunk.h"
#include "utilities/table.h"
#include "types/stack.h"
#include "types/value.h"
#include "natives/methods.h"

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

  Prototypes prototypes;

  Upvalue* upvalues;

  Object* objects;
} VM;

void initialize_VM(VM* vm);
void free_VM(VM* vm);
void reset_VM(VM* vm);

Results interpret(VM* vm, const char* source);

#endif