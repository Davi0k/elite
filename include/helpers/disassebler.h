#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include "utilities/chunk.h"

void disassemble_chunk(Chunk* chunk, const char* name);

int disassemble_instruction(Chunk* chunk, int offset);

#endif