#include <stdlib.h>

#include "utilities/chunk.h"
#include "utilities/memory.h"

void initialize_chunk(Chunk* chunk, VM* vm) {
  initialize_constants(&chunk->constants, vm);

  chunk->count = 0;
  chunk->capacity = 0;
  chunk->code = NULL;

  chunk->lines = NULL;
}

void free_chunk(Chunk* chunk) {
  free_constants(&chunk->constants);

  FREE_ARRAY(chunk->constants.vm, int, chunk->lines, chunk->capacity);
  FREE_ARRAY(chunk->constants.vm, uint8_t, chunk->code, chunk->capacity);
}

void write_chunk(Chunk* chunk, uint8_t byte, int line) {
  if (chunk->capacity < chunk->count + 1) {
    int capacity = chunk->capacity;

    chunk->capacity = GROW_CAPACITY(capacity);

    chunk->code = GROW_ARRAY(chunk->constants.vm, uint8_t, chunk->code, capacity, chunk->capacity);
    chunk->lines = GROW_ARRAY(chunk->constants.vm, int, chunk->lines, capacity, chunk->capacity);
  }

  chunk->code[chunk->count] = byte;
  chunk->lines[chunk->count] = line;

  chunk->count++;
}

int add_constant(Chunk* chunk, Value value) {
  push(&chunk->constants.vm->stack, value);
  write_constants(&chunk->constants, value);
  pop(&chunk->constants.vm->stack, 1);

  return chunk->constants.count - 1;
}