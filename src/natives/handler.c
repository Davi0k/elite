#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "vm.h"

#include "types/object.h"

#include "natives/handler.h"
#include "natives/functions.h"

static void load_native_function(VM* vm, const char* identifier, CFunction c_function) {
  push(&vm->stack, OBJECT(copy_string(vm, identifier, (int)strlen(identifier))));
  push(&vm->stack, OBJECT(new_native_function(vm, c_function)));

  table_set(&vm->globals, AS_STRING(vm->stack.content[0]), vm->stack.content[1]);

  pop(&vm->stack, 2);
}

void load_default_native_functions(VM* vm) {
  load_native_function(vm, "stopwatch", stopwatch_native);
  load_native_function(vm, "number", number_native);
  load_native_function(vm, "print", print_native);
  load_native_function(vm, "input", input_native);
  load_native_function(vm, "length", length_native);
  load_native_function(vm, "type", type_native);
}

void set_handler(Handler* handler, VM* vm) {
  handler->vm = vm;

  handler->error = false;

  strncpy(handler->message, run_time_errors[UNDEFINED_ERROR], LINE_LENGTH_MAX);
}

Value throw(Handler* handler, const char* message, int count, ...) {
  va_list list;

  va_start(list, count);

  handler->error = true;

  vsprintf(handler->message, message, list);

  va_end(list);

  return UNDEFINED;
}