#include <stdio.h>

#include "helpers/debug.h"
#include "utils/chunk.h"
#include "utils/value.h"

static int simple_representation(const char* name, int offset) {
  printf("%s", name); printf("\n");

  return offset + 1;
}

static int constant_representation(const char* name, Chunk* chunk, int offset) {
  uint8_t constant = chunk->code[offset + 1];

  printf("%s - %d - ", name, constant);
  print_value(chunk->constants.values[constant]);
  printf("\n");

  return offset + 2;
}

void disassemble_chunk(Chunk* chunk, const char* name) {
  printf("< %s >", name); printf("\n");

  int offset = 0;

  while (offset < chunk->count)
    offset = disassemble_instruction(chunk, offset);
}

int disassemble_instruction(Chunk* chunk, int offset) {
  printf("%04d ", offset);

  printf("%d - ", chunk->lines[offset]);

  uint8_t instruction = chunk->code[offset];

  switch (instruction) {
    case OP_CONSTANT:
      return constant_representation(stringified_operations[instruction], chunk, offset);

    case OP_RETURN:
      return simple_representation(stringified_operations[instruction], offset);

    default:
      printf("Unknown Operation Code %d", instruction); printf("\n");
      return offset + 1;
  }
}

void print_stack(VM* vm) {
  for (Value* slot = vm->stack; slot < vm->top; slot++) {
    printf("[ ");
    print_value(*slot);
    printf(" ]");

    printf("\n");
  }

  printf("\n");
}