#ifndef DEBUG_H
#define DEBUG_H

#include "utils/chunk.h"
#include "vm.h"

void disassemble_chunk(Chunk* chunk, const char* name);
int disassemble_instruction(Chunk* chunk, int offset);

void print_stack(VM* vm);

#endif