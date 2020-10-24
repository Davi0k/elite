#include <stdlib.h>

#include "utils/chunk.h"
#include "utils/memory.h"

void initialize_chunk(Chunk* chunk) {
  initialize_constants(&chunk->constants);

  chunk->count = 0;
  chunk->capacity = 0;
  chunk->code = NULL;

  chunk->lines = NULL;
}

void free_chunk(Chunk* chunk) {
  free_constants(&chunk->constants);

  FREE_ARRAY(int, chunk->lines, chunk->capacity);
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  initialize_chunk(chunk);
}

void write_chunk(Chunk* chunk, uint8_t byte, int line) {
  if (chunk->capacity < chunk->count + 1) {
    int capacity = chunk->capacity;

    chunk->capacity = GROW_CAPACITY(capacity);

    chunk->code = GROW_ARRAY(uint8_t, chunk->code, capacity, chunk->capacity);
    chunk->lines = GROW_ARRAY(int, chunk->lines, capacity, chunk->capacity);
  }

  chunk->code[chunk->count] = byte;
  chunk->lines[chunk->count] = line;

  chunk->count++;
}

int add_constant(Chunk* chunk, Value value) {
  write_constants(&chunk->constants, value);
  return chunk->constants.count - 1;
}