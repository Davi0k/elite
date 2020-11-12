#include <stdio.h>

#include "helpers/debug.h"
#include "utilities/chunk.h"
#include "types/object.h"
#include "types/value.h"
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

static int byte_representation(const char* name, Chunk* chunk, int offset) {
  uint8_t slot = chunk->code[offset + 1];
  printf("%-16s %4d\n", name, slot);
  return offset + 2; 
}

static int jump_representation(const char* name, int sign, Chunk* chunk, int offset) {
  uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);
  jump |= chunk->code[offset + 2];
  printf("%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);
  return offset + 3;
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
    case OP_LOOP:
    case OP_LOOP_CONDITIONAL:
    case OP_JUMP:
    case OP_JUMP_CONDITIONAL:
      return jump_representation(strings[instruction], 1, chunk, offset);

    case OP_LOCAL_SET:
    case OP_LOCAL_GET:
    case OP_UP_SET:
    case OP_UP_GET:
    case OP_CALL:
      return byte_representation(strings[instruction], chunk, offset);

    case OP_CONSTANT:
    case OP_GLOBAL_INITIALIZE:
    case OP_GLOBAL_SET:
    case OP_GLOBAL_GET:
      return constant_representation(strings[instruction], chunk, offset);

    case OP_TRUE:
    case OP_FALSE:
    case OP_VOID:
    case OP_UNDEFINED:
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
    case OP_POP:
    case OP_POP_N:
    case OP_RETURN:
    case OP_EXIT:
      return simple_representation(strings[instruction], offset);

    case OP_CLOSURE: {
      offset++;
      
      uint8_t constant = chunk->code[offset++];

      printf("%-16s %4d ", strings[instruction], constant);
      print_value(chunk->constants.values[constant]);
      printf("\n");

      Function* function = AS_FUNCTION(chunk->constants.values[constant]);

      for (int i = 0; i < function->count; i++) {
        int local = chunk->code[offset++];
        int index = chunk->code[offset++];
        printf("%04d      |                     %s %d\n", offset - 2, local ? "local" : "upvalue", index);
      }

      return offset;
    }

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