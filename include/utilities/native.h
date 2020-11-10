#ifndef NATIVE_H
#define NATIVE_H

#include "common.h"
#include "utilities/table.h"
#include "types/value.h"

typedef struct {
  bool error;
  char message[LINE_LENGTH_MAX];
} Handler;

typedef Value (*Internal)(Value* arguments, int count, Handler* handler);

void set_handler(Handler* handler);

Value stopwatch(Value* arguments, int count, Handler* handler);

Value print(Value* arguments, int count, Handler* handler);

#endif