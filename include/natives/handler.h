#ifndef HANDLER_H
#define HANDLER_H

#include "common.h"

#include "types/value.h"

#define LINE_LENGTH_MAX 1024

typedef struct {
  VM* vm;

  bool error;

  char message[LINE_LENGTH_MAX];
} Handler;

void load_default_native_functions(VM* vm);

void set_handler(Handler* handler, VM* vm);

Value throw(Handler* handler, const char* message, int count, ...);

typedef Value (*CFunction)(int count, Value* arguments, Handler* handler);

typedef Value (*CMethod)(Value receiver, int count, Value* arguments, Handler* handler);

#endif