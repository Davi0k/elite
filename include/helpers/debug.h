#ifndef DEBUG_H
#define DEBUG_H

#include "common.h"

#include "utilities/chunk.h"
#include "helpers/stack.h"

void disassemble_chunk(Chunk* chunk, const char* name);
int disassemble_instruction(Chunk* chunk, int offset);

void print_stack(Stack* stack);

#endif