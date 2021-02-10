#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "utilities/native.h"
#include "types/object.h"

static Value error(Handler* handler, const char* message, int count, ...) {
  va_list list;

  va_start(list, count);

  handler->error = true;

  vsprintf(handler->message, message, list);

  va_end(list);

  return UNDEFINED;
}

void set_handler(Handler* handler, VM* vm) {
  handler->vm = vm;

  handler->error = false;

  strncpy(handler->message, run_time_errors[UNDEFINED_ERROR], LINE_LENGTH_MAX);
}

Value stopwatch_native(int count, Value* arguments, Handler* handler) {
  if (count == 0) {
    double seconds = (double)clock() / CLOCKS_PER_SEC;

    return OBJECT(allocate_number_from_double(handler->vm, seconds));
  } 

  return error(handler, run_time_errors[EXPECT_ARGUMENTS_NUMBER], 2, 0, count);
}

Value number_native(int count, Value* arguments, Handler* handler) {
  if (count == 0) 
    return OBJECT(allocate_number_from_double(handler->vm, 0.0));

  if (count == 1) {
    Value argument = arguments[0];

    if (IS_NUMBER(argument))
      return argument;

    if (IS_STRING(argument))
      return OBJECT(allocate_number_from_string(handler->vm, AS_STRING(argument)->content));

    return error(handler, run_time_errors[MUST_BE_NUMBER_OR_STRING], 0);
  }

  return error(handler, run_time_errors[EXPECT_ARGUMENTS_NUMBER], 2, 1, count);
}

Value print_native(int count, Value* arguments, Handler* handler) {
  for (int i = 0; i < count; i++)
    print_value(arguments[i]);
  
  printf("\n");

  return UNDEFINED;
}

Value input_native(int count, Value* arguments, Handler* handler) {
  if (count == 0 || count == 1) {
    if (count == 1) {
      if (IS_STRING(arguments[0]) == true) 
        print_value(arguments[0]);
      else return error(handler, run_time_errors[MUST_BE_STRING], 0);
    }

    char* content = NULL;

    size_t size = 0;

    ssize_t length = getline(&content, &size, stdin);

    return OBJECT(copy_string(handler->vm, content, length - 1));
  } 
  
  return error(handler, run_time_errors[EXPECT_ARGUMENTS_NUMBER], 2, 1, count);
}

Value length_native(int count, Value* arguments, Handler* handler) {
  if (count == 1) {
    Value value = arguments[0];

    if (IS_STRING(value) == true) 
      return OBJECT(allocate_number_from_double(handler->vm, AS_STRING(value)->length));
    else return error(handler, run_time_errors[MUST_BE_STRING], 0);
  } 
  
  return error(handler, run_time_errors[EXPECT_ARGUMENTS_NUMBER], 2, 1, count);
}