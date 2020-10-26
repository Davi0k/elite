#ifndef DEBUG_H
#define DEBUG_H

#include "utils/chunk.h"
#include "utils/value.h"
#include "tokenizer.h"
#include "vm.h"

void disassemble_chunk(Chunk* chunk, const char* name);
int disassemble_instruction(Chunk* chunk, int offset);

void print_value(Value value);

void print_stack(Stack* stack);

#endif