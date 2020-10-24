#ifndef CHUNK_H
#define CHUNK_H

#include "common.h"
#include "utils/value.h"

#define FOREACH(OPERATION) \
  OPERATION(OP_CONSTANT) \
  OPERATION(OP_RETURN) 

#define ENUMERATE(ENUMERATION) ENUMERATION,
#define STRINGIFY(STRING) #STRING,
#define COMPUTED(GOTO) &&GOTO,

typedef enum {
  FOREACH(ENUMERATE)
} Operations;

static const char* stringified_operations[] = {
  FOREACH(STRINGIFY)
};

typedef struct {
  int count;
  int capacity;
  uint8_t* code;

  int* lines;

  Constants constants;
} Chunk;

void initialize_chunk(Chunk* chunk);
void free_chunk(Chunk* chunk);
void write_chunk(Chunk* chunk, uint8_t byte, int line);

int add_constant(Chunk* chunk, Value value);

#endif