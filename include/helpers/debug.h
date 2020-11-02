#ifndef DEBUG_H
#define DEBUG_H

#include "utilities/chunk.h"
#include "types/value.h"
#include "tokenizer.h"
#include "vm.h"

void disassemble_chunk(Chunk* chunk, const char* name);
int disassemble_instruction(Chunk* chunk, int offset);

void print_stack(Stack* stack);

#endif