#ifndef COMPILER_H
#define COMPILER_H

#include "vm.h"
#include "tokenizer.h"

typedef struct {
  Tokenizer tokenizer;
  Token current;
  Token previous;
  bool error;
  bool panic;
} Parser;

bool compile(Chunk* chunk, const char* source);

#endif