#include <stdlib.h>

#include "utilities/memory.h"

void* reallocate(void* pointer, size_t oldest, size_t newest) {
  if (newest == 0) {
    free(pointer);
    return NULL;
  }

  void* result = realloc(pointer, newest);

  if (result == NULL) exit(1);

  return result;
}

void free_object(Object* object) {
  switch (object->type) {
    case OBJECT_STRING: {
      String* string = (String*)object;
      FREE_ARRAY(char, string->content, string->length + 1);
      FREE(String, object);
      break;
    }
  }
}

void free_vm_objects(VM* vm) {
  Object* object = vm->objects;

  while (object != NULL) {
    Object* next = object->next;
    free_object(object);
    object = next;
  }
}