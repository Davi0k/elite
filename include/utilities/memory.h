#ifndef MEMORY_H
#define MEMORY_H

#include "common.h"

#include "types/object.h"

#define MINIMUM_CAPACITY 8

#define LOAD_FACTOR 2

#define GARBAGE_COLLECTOR_GROW_FACTOR 2

#define DEFAULT_THRESHOLD 1024 * 64

#define GROW_CAPACITY(capacity) \
  ( (capacity) < MINIMUM_CAPACITY ? MINIMUM_CAPACITY : (capacity) * LOAD_FACTOR )

#define ALLOCATE(vm, type, count) \
  (type*)reallocate(vm, NULL, 0, sizeof(type) * count)

#define ALLOCATE_ARRAY(vm, type, pointer, oldest, newest) \
  (type*)reallocate( vm, pointer, sizeof(type) * (oldest), sizeof(type) * (newest) )

#define FREE(vm, type, pointer) \
  reallocate(vm, pointer, sizeof(type), 0)

#define FREE_ARRAY(vm, type, pointer, old) \
  reallocate( vm, pointer, sizeof(type) * (old), 0 )

typedef struct {
  int count;
  int capacity;
  Object** content;
} Parents;

void* reallocate(VM* vm, void* pointer, size_t oldest, size_t newest);

void recycle(VM* vm);

void roots(VM* vm, Parents* parents);
void traverse(VM* vm, Parents* parents);

void mark(Parents* parents, Value value);

void sweep(VM* vm);

void free_object(VM* vm, Object* object);

#endif