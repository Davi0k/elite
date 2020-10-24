#include <stdio.h>

#include "common.h"
#include "helpers/debug.h"
#include "vm.h"

void initialize_VM(VM* vm) {
  vm->top = vm->stack;
}

void free_VM(VM* vm) {

}

void push(VM* vm, Value value) {
  *vm->top = value;
  vm->top++;
}

Value pop(VM* vm) {
  vm->top--;

  return *vm->top;
}

static Results run(VM* vm) {
  #define READ_BYTE() ( *++ip )
  #define READ_CONSTANT() ( vm->chunk->constants.values[READ_BYTE()] )
  
  static void* computed_operations[] = {
    FOREACH(COMPUTED)
  };

  register uint8_t* ip = vm->chunk->code;

  goto *computed_operations[*ip];

  OP_CONSTANT:
    do {
      Value constant = READ_CONSTANT();
      push(vm, constant);
      goto *computed_operations[READ_BYTE()];
    } while(false);
  
  OP_RETURN:
    return INTERPRET_OK;

  #undef READ_BYTE
  #undef READ_CONSTANT
}

Results interpret(VM* vm, Chunk* chunk) {
  vm->chunk = chunk;

  return run(vm);
}