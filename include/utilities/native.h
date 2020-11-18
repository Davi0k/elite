#ifndef NATIVE_H
#define NATIVE_H

#include "common.h"
#include "utilities/table.h"
#include "types/value.h"

typedef struct VM VM;

typedef struct {
  VM* vm;

  bool error;

  char message[LINE_LENGTH_MAX];
} Handler;

void set_handler(Handler* handler, VM* vm);

Value stopwatch_native(int count, Value* arguments, Handler* handler);
Value number_native(int count, Value* arguments, Handler* handler);
Value print_native(int count, Value* arguments, Handler* handler);
Value input_native(int count, Value* arguments, Handler* handler);

typedef Value (*Internal)(int count, Value* arguments, Handler* handler);

#endif