#ifndef COMPILER_H
#define COMPILER_H

#include "vm.h"
#include "tokenizer.h"
#include "utilities/chunk.h"

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
  Token token;
  int depth;
} Local;

typedef struct {
  Local locals[UINT8_MAX + 1];
  int count;
  int scope;
} Compiler;

typedef struct {
  Tokenizer tokenizer;

  Token current;
  Token previous;

  bool error;
  bool panic;

  Chunk* compiling;

  Compiler* compiler;

  VM* vm;
} Parser;

typedef void (*Function)(Parser* parser, bool assign);

typedef struct {
  Function prefix;
  Function infix;
  Precedences precedence;
} Rule;

void set_compiler(Parser* parser, Compiler* compiler);

bool compile(VM* vm, Chunk* chunk, const char* source);

#endif