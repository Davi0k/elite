#ifndef COMPILER_H
#define COMPILER_H

#include "vm.h"
#include "tokenizer.h"
#include "utilities/chunk.h"

#define INITIALIZE -1

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
} Local;

typedef struct {
  uint8_t index;
  bool local;
} Up;

typedef struct Compiler {
  struct Compiler* enclosing;

  Function* function;

  Positions position;

  Local locals[UINT8_COUNT];
  int count;
  int scope;

  Up ups[UINT8_COUNT];
} Compiler;

typedef struct {
  Tokenizer tokenizer;

  Token current;
  Token previous;

  bool error;
  bool panic;

  Compiler* compiler;

  VM* vm;
} Parser;

typedef void (*Execute)(Parser* parser, bool assign);

typedef struct {
  Execute prefix;
  Execute infix;
  Precedences precedence;
} Rule;

void set_compiler(Parser* parser, Compiler* compiler, Positions position);

Function* compile(VM* vm, const char* source);

#endif