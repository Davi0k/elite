#include <stdlib.h>

#include "utils/memory.h"

void* reallocate(void* pointer, size_t oldest, size_t newest) {
  if (newest == 0) {
    free(pointer);
    return NULL;
  }

  void* result = realloc(pointer, newest);

  if (result == NULL) exit(1);

  return result;
}