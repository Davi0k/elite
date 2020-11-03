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

void print_object(Value value) {
  switch (OBJECT_TYPE(value)) {
    case OBJECT_STRING:
      printf("%s", AS_STRING(value)->content);
      break;
  }
}