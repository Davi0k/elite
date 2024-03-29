#include <stdio.h>
#include <string.h>

#include "types/value.h"
#include "types/object.h"
#include "utilities/memory.h"

void initialize_constants(Constants* constants, VM* vm) {
  constants->values = NULL;
  constants->capacity = 0;
  constants->count = 0;

  constants->vm = vm;
}

void free_constants(Constants* constants) {
  FREE_ARRAY(constants->vm, Value, constants->values, constants->capacity);
}

void write_constants(Constants* constants, Value value) {
  if (constants->capacity < constants->count + 1) {
    int capacity = constants->capacity;

    constants->capacity = GROW_CAPACITY(capacity);

    constants->values = ALLOCATE_ARRAY(constants->vm, Value, constants->values, capacity, constants->capacity);
  }

  constants->values[constants->count] = value;

  constants->count++;
}

bool equal(Value left, Value right) {
  if (left.type != right.type) return false;

  switch (left.type) {  
    case VALUE_BOOLEAN: return AS_BOOLEAN(left) == AS_BOOLEAN(right);
    
    case VALUE_OBJECT: {
      if (OBJECT_TYPE(left) == OBJECT_NUMBER && OBJECT_TYPE(right) == OBJECT_NUMBER)
        return mpf_cmp(AS_NUMBER(left)->content, AS_NUMBER(right)->content) == 0; 
      else return AS_OBJECT(left) == AS_OBJECT(right);
    }

    case VALUE_VOID: return true;

    case VALUE_UNDEFINED: return true;

    default: return false;
  }
}

void print_value(Value value) {
  switch (value.type) {
    case VALUE_BOOLEAN: printf(AS_BOOLEAN(value) ? "true" : "false"); break;

    case VALUE_OBJECT: print_object(value); break;

    case VALUE_VOID: printf("void"); break;

    case VALUE_UNDEFINED: printf("undefined"); break;
  }
}