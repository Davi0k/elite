#include <stdio.h>

#include "utils/memory.h"
#include "utils/value.h"

void initialize_constants(Constants* constants) {
  constants->values = NULL;
  constants->capacity = 0;
  constants->count = 0;
}

void free_constants(Constants* constants) {
  FREE_ARRAY(Value, constants->values, constants->capacity);
  initialize_constants(constants);
}

void write_constants(Constants* constants, Value value) {
  if (constants->capacity < constants->count + 1) {
    int capacity = constants->capacity;

    constants->capacity = GROW_CAPACITY(capacity);

    constants->values = GROW_ARRAY(Value, constants->values, capacity, constants->capacity);
  }

  constants->values[constants->count] = value;

  constants->count++;
}

void print_value(Value value) {
  printf("%g", value);
}