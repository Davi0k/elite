#include <stdio.h>
#include <string.h>

#include "vm.h"
#include "utils/memory.h"
#include "utils/object.h"
#include "utils/value.h"

#define ALLOCATE_OBJECT(object, type) \
  (object*)allocate_object(sizeof(object), type)

static Object* allocate_object(size_t size, Objects type) {
  Object* object = (Object*)reallocate(NULL, 0, size);
  object->type = type;
  return object;
}

String* allocate_string(char* content, int length) {
  String* string = ALLOCATE_OBJECT(String, OBJECT_STRING);
  string->content = content;
  string->length = length;
}

String* copy_string(const char* content, int length) {
  char* heap = ALLOCATE(char, length + 1);
  memcpy(heap, content, length);
  heap[length] = '\0';

  return allocate_string(heap, length);
}

void print_object(Value value) {
  switch (OBJECT_TYPE(value)) {
    case OBJECT_STRING:
      printf("%s", AS_STRING(value)->content);
      break;
  }
}