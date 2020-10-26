#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "vm.h"

void initialize_VM(VM* vm) {
  initialize_stack(&vm->stack);
}

void free_VM(VM* vm) {
  free_stack(&vm->stack);
}

void initialize_stack(Stack* stack) {
  stack->top = stack->content;
}

void free_stack(Stack* stack) {
  stack->top = 0;
}

void push(Stack* stack, Value value) {
  *stack->top = value;
  stack->top++;
}

Value pop(Stack* stack) {
  stack->top--;

  return *stack->top;
}

static Results run(VM* vm) {
  #define READ_BYTE() ( *++ip )

  #define READ_CONSTANT() ( vm->chunk->constants.values[READ_BYTE()] )

  #define COMPUTE_NEXT() goto *jump_table[READ_BYTE()]

  #define BINARY_OPERATION(operator) \
    do { \
      Value right = pop(&vm->stack); \
      Value left = pop(&vm->stack); \
      push(&vm->stack, left operator right); \
    } while(false) 
  
  static void* jump_table[] = {
    FOREACH(COMPUTED)
  };

  register uint8_t* ip = vm->chunk->code;

  goto *jump_table[*ip];

  OP_CONSTANT:
    do {
      Value constant = READ_CONSTANT();
      push(&vm->stack, constant);
      COMPUTE_NEXT();
    } while(false);

  OP_POSITIVE:
    do {
      Value constant = +pop(&vm->stack);
      push(&vm->stack, constant);
      COMPUTE_NEXT();
    } while(false);

  OP_NEGATIVE:
    do {
      Value constant = -pop(&vm->stack);
      push(&vm->stack, constant);
      COMPUTE_NEXT();
    } while(false);
  
  OP_ADD:
    BINARY_OPERATION(+);
    COMPUTE_NEXT();

  OP_SUBTRACT:
    BINARY_OPERATION(-);
    COMPUTE_NEXT();

  OP_MULTIPLY:
    BINARY_OPERATION(*);
    COMPUTE_NEXT();

  OP_DIVIDE:
    BINARY_OPERATION(/);
    COMPUTE_NEXT();

  OP_RETURN:
    return INTERPRET_OK;

  #undef READ_BYTE
  #undef READ_CONSTANT
  #undef COMPUTE_NEXT
  #undef BINARY_OPERATION
}

Results interpret(VM* vm, const char* source) {
  compile(vm, source);

  return INTERPRET_OK;
  
  return run(vm);
}