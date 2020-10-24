#ifndef MEMORY_H
#define MEMORY_H

#include "common.h"

#define MINIMUM 8
#define MULTIPLICATOR 2

#define FREE_ARRAY(type, pointer, old) \
    reallocate( pointer, sizeof(type) * (old), 0 )

#define GROW_CAPACITY(capacity) \
  ( (capacity) < MINIMUM ? MINIMUM : (capacity) * MULTIPLICATOR )

#define GROW_ARRAY(type, pointer, oldest, newest) \
    (type*)reallocate( pointer, sizeof(type) * (oldest), sizeof(type) * (newest) )

void* reallocate(void* pointer, size_t oldest, size_t newest);

#endif