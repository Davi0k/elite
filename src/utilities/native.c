#include <string.h>
#include <stdio.h>
#include <time.h>

#include "utilities/native.h"

static Value time_native(int count, Value* arguments) {
  double seconds = (double)clock() / CLOCKS_PER_SEC;

  Value number = NUMBER_FROM_VALUE(seconds);

  return number;
}

static void define(VM* vm, const char* identifier, Internal internal) {
  push(&vm->stack, OBJECT(copy_string(vm, identifier, (int)strlen(identifier))));
  push(&vm->stack, OBJECT(new_native(vm, internal)));

  table_set(&vm->globals, AS_STRING(vm->stack.content[0]), vm->stack.content[1]);

  pop(&vm->stack, 2);
}

void natives(VM* vm) {
  define(vm, "time", time_native);
}