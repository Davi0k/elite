#include <stdio.h>

#include "utils/memory.h"
#include "utils/value.h"

Value NUMBER(mpf_t number) {
  Value value;

  value.type = VALUE_NUMBER;

  mpf_init_set(value.content.number, number);

  return value;
}

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

bool equal(Value left, Value right) {
  if (left.type != right.type) return false;

  switch (left.type) {
    case VALUE_NUMBER: return mpf_cmp(AS_NUMBER(left), AS_NUMBER(right)) == 0; 
    case VALUE_BOOLEAN: return AS_BOOLEAN(left) == AS_BOOLEAN(right);
    case VALUE_VOID: return true;

    default: return false;
  }
}

void print_value(Value value) {
  switch (value.type) {
    case VALUE_NUMBER: gmp_printf("%.Ff", AS_NUMBER(value)); break;

    case VALUE_BOOLEAN: printf(AS_BOOLEAN(value) ? "true" : "false"); break;

    case VALUE_VOID: printf("void"); break;
  }
}