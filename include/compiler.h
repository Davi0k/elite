#ifndef COMPILER_H
#define COMPILER_H

#include "common.h"

#include "tokenizer.h"
#include "utilities/chunk.h"

#define INITIALIZE -1

#define MAXIMUM_BOUND ( UINT8_MAX + 1 )

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

typedef enum {
  POSITION_FUNCTION,
  POSITION_SCRIPT
} Positions;

typedef struct {
  Token identifier;
  int depth;
  bool captured;
} Local;

typedef struct {
  uint8_t index;
  bool local;
} Up;

typedef struct Compiler {
  struct Compiler* enclosing;

  Function* function;

  Positions position;

  Up ups[MAXIMUM_BOUND];

  Local locals[MAXIMUM_BOUND];
  int count;
  int scope;
} Compiler;

typedef struct {
  Tokenizer tokenizer;

  Token previous, current;

  bool panic, error;

  Compiler* compiler;

  VM* vm;
} Parser;

typedef void (*Execute)(Parser* parser, bool assign);

typedef struct {
  Execute prefix;
  Execute infix;
  Precedences precedence;
} Rule;

Function* compile(VM* vm, const char* source);

#endif