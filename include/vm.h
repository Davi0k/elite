#ifndef VM_H
#define VM_H

#include "common.h"
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

  Stack stack;

  Upvalue* upvalues;

  Object* objects;

  Table strings, globals;
} VM;

void native(VM* vm, const char* identifier, Internal internal);

void reset(VM* vm);

void initialize_VM(VM* vm);
void free_VM(VM* vm);

Results interpret(VM* vm, const char* source);

extern Function* compile(VM* vm, const char* source);

#endif