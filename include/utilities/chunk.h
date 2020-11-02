#ifndef CHUNK_H
#define CHUNK_H

#include "common.h"
#include "types/value.h"

#define FOREACH(OPERATION) \
  OPERATION(OP_CONSTANT) \
  OPERATION(OP_TRUE) OPERATION(OP_FALSE) \
  OPERATION(OP_VOID) \
  OPERATION(OP_NEGATION) \
  OPERATION(OP_ADD) OPERATION(OP_SUBTRACT) \
  OPERATION(OP_MULTIPLY) OPERATION(OP_DIVIDE) \
  OPERATION(OP_POWER) \
  OPERATION(OP_NOT) \
  OPERATION(OP_EQUAL) \
  OPERATION(OP_GREATER) OPERATION(OP_LESS) \
  OPERATION(OP_EXIT) 

#define ENUMERATE(ENUMERATION) ENUMERATION,
#define STRINGIFY(STRING) #STRING,
#define COMPUTED(GOTO) &&GOTO,

typedef enum {
  FOREACH(ENUMERATE)
} Operations;

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