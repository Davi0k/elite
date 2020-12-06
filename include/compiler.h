#ifndef COMPILER_H
#define COMPILER_H

#include "common.h"

#include "tokenizer.h"
#include "utilities/chunk.h"

#define INITIALIZE -1

#define MAXIMUM_LIMIT ( UINT8_MAX + 1 )

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
  POSITION_METHOD,
  POSITION_CONSTRUCTOR,
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

  Up ups[MAXIMUM_LIMIT];

  Local locals[MAXIMUM_LIMIT];
  int count;
  int scope;
} Compiler;

typedef struct Current {
  struct Current* enclosing;
  Token name;
  bool inheritance;
} Entity;

typedef struct {
  Tokenizer tokenizer;

  Token previous, current;

  bool panic, error;

  Compiler* compiler;

  Entity* entity;

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