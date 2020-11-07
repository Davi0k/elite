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

    case OBJECT_FUNCTION: {
      Function* function = (Function*)object;
      free_chunk(&function->chunk);
      FREE(Function, object);
      break;
    }
  }
}