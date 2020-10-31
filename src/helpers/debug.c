#include <stdio.h>

#include "helpers/debug.h"
#include "utils/chunk.h"
#include "utils/value.h"
#include "tokenizer.h"

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

  static char* strings[] = {
    FOREACH(STRINGIFY)
  };

  uint8_t instruction = chunk->code[offset];

  switch (instruction) {
    case OP_CONSTANT:
      return constant_representation(strings[instruction], chunk, offset);

    case OP_TRUE:
    case OP_FALSE:
    case OP_VOID:
    case OP_NEGATION:
    case OP_ADD:
    case OP_SUBTRACT:
    case OP_MULTIPLY:
    case OP_DIVIDE:
    case OP_POWER:
    case OP_NOT:
    case OP_EQUAL:
    case OP_GREATER:
    case OP_LESS:
    case OP_EXIT:
      return simple_representation(strings[instruction], offset);

    default:
      printf("Unknown Operation Code %d", instruction); printf("\n");
      return offset + 1;
  }
}

void print_stack(Stack* stack) {
  for (Value* slot = stack->content; slot < stack->top; slot++) {
    printf("[ ");
    print_value(*slot);
    printf(" ]");
  }

  printf("\n");
}