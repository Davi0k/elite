#ifndef VALUE_H
#define VALUE_H

#include "common.h"

typedef double Value;

typedef struct {
  int capacity;
  int count;
  Value* values;
} Constants;

void initialize_constants(Constants* constants);
void free_constants(Constants* constants);
void write_constants(Constants* constants, Value value);

#endif