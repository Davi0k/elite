#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "vm.h"

#include "types/object.h"

#include "natives/handler.h"
#include "natives/functions.h"

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