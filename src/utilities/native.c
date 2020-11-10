#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "utilities/native.h"
#include "helpers/error.h"
#include "types/object.h"

static Value error(Handler* handler, const char* message, ...) {
  va_list list;

  va_start(list, message);

  handler->error = false;

  vfprintf(stdout, message, list);

  va_end(list);

  return UNDEFINED;
}

void set_handler(Handler* handler, VM* vm) {
  handler->vm = vm;

  handler->error = false;

  strncpy(handler->message, run_time[UNDEFINED_ERROR], LINE_LENGTH_MAX);
}

Value stopwatch_native(int count, Value* arguments, Handler* handler) {
  if (count == 0) {
    double seconds = (double)clock() / CLOCKS_PER_SEC;

    return NUMBER_FROM_DOUBLE(handler->vm, seconds);
  } 

  return error(handler, run_time[EXPECT_ARGUMENTS_NUMBER], 0, count);
}

Value number_native(int count, Value* arguments, Handler* handler) {
  if (count == 0) 
    return NUMBER_FROM_DOUBLE(handler->vm, 0.0);

  if (count == 1) {
    Value argument = arguments[0];

    if (IS_NUMBER(argument))
      return argument;

    if (IS_STRING(argument))
      return NUMBER_FROM_STRING(handler->vm, AS_STRING(argument)->content);

    return error(handler, run_time[MUST_BE_NUMBER_OR_STRING]);
  }

  return error(handler, run_time[EXPECT_ARGUMENTS_NUMBER], 1, count);
}

Value print_native(int count, Value* arguments, Handler* handler) {
  for (int i = 0; i < count; i++)
    print_value(arguments[i]);

  printf("\n");
  
  return UNDEFINED;
}