#include <stdio.h>
#include <string.h>

#include "vm.h"
#include "utilities/memory.h"
#include "utilities/table.h"
#include "types/object.h"
#include "types/value.h"

#define ALLOCATE_OBJECT(vm, object, type) \
  (object*)allocate_object(vm, sizeof(object), type)

static Object* allocate_object(VM* vm, size_t size, Objects type) {
  Object* object = (Object*)reallocate(NULL, 0, size);
  object->type = type;

  object->next = vm->objects;
  vm->objects = object;

  return object;
}

static uint32_t hash_string(const char* key, int length) {
  uint32_t hash = 2166136261u;

  for (int i = 0; i < length; i++) {
    hash ^= key[i];
    hash *= 16777619;
  }

  return hash;
}

String* allocate_string(VM* vm, const char* content, int length, uint32_t hash) {
  String* string = ALLOCATE_OBJECT(vm, String, OBJECT_STRING);
  string->content = (char*)content;
  string->length = length;

  string->hash = hash;

  table_set(&vm->table, string, UNDEFINED);

  return string;
}

String* copy_string(VM* vm, const char* content, int length) {
  uint32_t hash = hash_string(content, length);

  String* intern = table_find_string(&vm->table, content, length, hash);

  if (intern != NULL) return intern;

  char* heap = ALLOCATE(char, length + 1);
  memcpy(heap, content, length);
  heap[length] = '\0';

  return allocate_string(vm, heap, length, hash);
}

String* take_string(VM* vm, const char* content, int length) {
  uint32_t hash = hash_string(content, length);

  String* intern = table_find_string(&vm->table, content, length, hash);

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