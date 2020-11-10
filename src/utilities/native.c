#include <string.h>
#include <stdio.h>
#include <time.h>

#include "utilities/native.h"
#include "helpers/error.h"

void set_handler(Handler* handler) {
  handler->error = false;

  strncpy(handler->message, run_time[UNDEFINED_ERROR], LINE_LENGTH_MAX);
}

Value stopwatch(Value* arguments, int count, Handler* handler) {
  if (count == 0) {
    double seconds = (double)clock() / CLOCKS_PER_SEC;

    Value number = NUMBER_FROM_VALUE(seconds);

    return number;
  } 

  handler->error = true;

  sprintf(handler->message, run_time[EXPECT_ARGUMENTS_NUMBER], 0, count);
  
  return UNDEFINED;
}

Value print(Value* arguments, int count, Handler* handler) {
  for (int i = 0; i < count; i++) {
    Value value = arguments[i];

    print_value(value);

    printf("\n");
  }
  
  return UNDEFINED;
}