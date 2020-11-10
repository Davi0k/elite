#include <stdio.h>
#include <string.h>

#include "vm.h"
#include "utilities/memory.h"
#include "utilities/table.h"
#include "types/object.h"
#include "types/value.h"
#include "helpers/hash.h"

#define ALLOCATE_OBJECT(vm, object, type) \
  (object*)allocate_object(vm, sizeof(object), type)

static Object* allocate_object(VM* vm, size_t size, Objects type) {
  Object* object = (Object*)reallocate(NULL, 0, size);
  object->type = type;

  object->next = vm->objects;
  vm->objects = object;

  return object;
}

Number* new_number(VM* vm, mpf_t value) {
  Number* number = ALLOCATE_OBJECT(vm, Number, OBJECT_NUMBER);
  mpf_init_set(number->content, value);
  return number;
}

Number* number_from_double(VM* vm, double value) {
  Number* number = ALLOCATE_OBJECT(vm, Number, OBJECT_NUMBER);
  mpf_init_set_d(number->content, value);
  return number;
}

Number* number_from_string(VM* vm, const char* value) {
  Number* number = ALLOCATE_OBJECT(vm, Number, OBJECT_NUMBER);
  mpf_init_set_str(number->content, value, 10);
  return number;
}

String* allocate_string(VM* vm, const char* content, int length, uint32_t hash) {
  String* string = ALLOCATE_OBJECT(vm, String, OBJECT_STRING);
  string->content = (char*)content;
  string->length = length;

  string->hash = hash;

  table_set(&vm->strings, string, UNDEFINED);

  return string;
}

String* copy_string(VM* vm, const char* content, int length) {
  uint32_t hash = hashing(content, length);

  String* intern = table_find_string(&vm->strings, content, length, hash);

  if (intern != NULL) return intern;

  char* heap = ALLOCATE(char, length + 1);
  memcpy(heap, content, length);
  heap[length] = '\0';

  return allocate_string(vm, heap, length, hash);
}

String* take_string(VM* vm, const char* content, int length) {
  uint32_t hash = hashing(content, length);

  String* intern = table_find_string(&vm->strings, content, length, hash);

  if (intern != NULL) {
    FREE_ARRAY(char, (char*)content, length + 1);
    return intern;
  }

  return allocate_string(vm, content, length, hash);
}

Function* new_function(VM* vm) {
  Function* function = ALLOCATE_OBJECT(vm, Function, OBJECT_FUNCTION);

  function->arity = 0;
  function->identifier = NULL;

  initialize_chunk(&function->chunk);

  return function;
}

Native* new_native(VM* vm, Internal internal) {
  Native* native = ALLOCATE_OBJECT(vm, Native, OBJECT_NATIVE);
  native->internal = internal;
  return native;
}

void print_object(Value value) {
  switch (OBJECT_TYPE(value)) {
    case OBJECT_NUMBER: gmp_printf("%.Ff", AS_NUMBER(value)); break;

    case OBJECT_STRING: printf("%s", AS_STRING(value)->content); break;

    case OBJECT_FUNCTION: {
      Function* function = AS_FUNCTION(value);

      if (function->identifier == NULL)
        printf("<Script>");
      else printf("<Function %s>", function->identifier->content);

      break;
    }

    case OBJECT_NATIVE: printf("<Native Function>"); break;
  }
}