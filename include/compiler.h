#ifndef COMPILER_H
#define COMPILER_H

#include "vm.h"
#include "tokenizer.h"
#include "utils/chunk.h"

typedef void (*Function)();

typedef enum {
  PRECEDENCE_NONE,
  PRECEDENCE_ASSIGNMENT,
  PRECEDENCE_OR,
  PRECEDENCE_AND,
  PRECEDENCE_EQUALITY,
  PRECEDENCE_COMPARISON,
  PRECEDENCE_TERM,
  PRECEDENCE_FACTOR,
  PRECEDENCE_UNARY,
  PRECEDENCE_CALL,
  PRECEDENCE_PRIMARY
} Precedences;

typedef struct {
  Tokenizer tokenizer;

  Token current;
  Token previous;

  bool error;
  bool panic;

  Chunk* compiling;
} Parser;

typedef struct {
  Function prefix;
  Function infix;
  Precedences precedence;
} Rule;

bool compile(Chunk* chunk, const char* source);

#endif