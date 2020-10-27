#include <stdio.h>
#include <stdarg.h>

#include "common.h"
#include "compiler.h"
#include "vm.h"

#include "helpers/debug.h"

void initialize_VM(VM* vm) {
  reset_stack(&vm->stack);
}

void free_VM(VM* vm) {

}

void reset_stack(Stack* stack) {
  stack->top = stack->content;
}

void push(Stack* stack, Value value) {
  *stack->top = value;
  stack->top++;
}

Value pop(Stack* stack) {
  stack->top--;

  return *stack->top;
}

static Value peek(Stack* stack, int distance) {
  return stack->top[- 1 - distance];
}

static bool falsey(Value value) {
  return IS_VOID(value) || (IS_BOOLEAN(value) && AS_BOOLEAN(value) == false);
}

static void runtime(VM* vm, uint8_t* ip, const char* message, ...) {
  va_list list;

  va_start(list, message);

  vfprintf(stderr, message, list);

  va_end(list);

  fputs("\n", stderr);

  size_t instruction = ip - vm->chunk->code - 1;
  int line = vm->chunk->lines[instruction];
  fprintf(stderr, "[Line %d] in script\n", line);

  reset_stack(&vm->stack);
}

static Results run(VM* vm) {
  #define READ_BYTE() ( *++ip )

  #define READ_CONSTANT() ( vm->chunk->constants.values[READ_BYTE()] )

  #define COMPUTE_NEXT() goto *jump_table[READ_BYTE()]

  #define BINARY_OPERATION(type, operator) \
    do { \
      if (!IS_NUMBER(peek(&vm->stack, 0)) || !IS_NUMBER(peek(&vm->stack, 1))) { \
        runtime(vm, ip, "Operands must be Numbers."); \
        return INTERPRET_RUNTIME_ERROR; \
      } \
      double right = AS_NUMBER(pop(&vm->stack)); \
      double left = AS_NUMBER(pop(&vm->stack)); \
      push(&vm->stack, type(left operator right)); \
    } while(false) 
  
  register uint8_t* ip = vm->chunk->code;

  static void* jump_table[] = {
    FOREACH(COMPUTED)
  };

  goto *jump_table[*ip];

  OP_CONSTANT:
    do {
      Value constant = READ_CONSTANT();
      push(&vm->stack, constant);
      COMPUTE_NEXT();
    } while(false);

  OP_TRUE: push(&vm->stack, BOOLEAN(true)); COMPUTE_NEXT();

  OP_FALSE: push(&vm->stack, BOOLEAN(false)); COMPUTE_NEXT();

  OP_VOID: push(&vm->stack, VOID); COMPUTE_NEXT();

  OP_POSITIVE:
    if (!IS_NUMBER(peek(&vm->stack, 0))) {
      runtime(vm, ip, "Operand must be a Number.");
      return INTERPRET_RUNTIME_ERROR;
    }

    push(&vm->stack, NUMBER(+AS_NUMBER(pop(&vm->stack))));

    COMPUTE_NEXT();

  OP_NEGATIVE:
    if (!IS_NUMBER(peek(&vm->stack, 0))) {
      runtime(vm, ip, "Operand must be a Number.");
      return INTERPRET_RUNTIME_ERROR;
    }

    push(&vm->stack, NUMBER(-AS_NUMBER(pop(&vm->stack))));

    COMPUTE_NEXT();
  
  OP_ADD: BINARY_OPERATION(NUMBER, +); COMPUTE_NEXT();

  OP_SUBTRACT: BINARY_OPERATION(NUMBER, -); COMPUTE_NEXT();

  OP_MULTIPLY: BINARY_OPERATION(NUMBER, *); COMPUTE_NEXT();

  OP_DIVIDE: BINARY_OPERATION(NUMBER, /); COMPUTE_NEXT();

  OP_NOT:
    do {
      bool result = falsey(pop(&vm->stack));
      push(&vm->stack, BOOLEAN(result));
      COMPUTE_NEXT();
    } while(false);

  OP_EQUAL:
    do {
      Value right = pop(&vm->stack);
      Value left = pop(&vm->stack);

      bool result = equal(left, right);

      push(&vm->stack, BOOLEAN(result));

      COMPUTE_NEXT();
    } while(false);

  OP_GREATER: BINARY_OPERATION(BOOLEAN, >); COMPUTE_NEXT();

  OP_LESS: BINARY_OPERATION(BOOLEAN, <); COMPUTE_NEXT();

  OP_EXIT: 
    print_stack(&vm->stack);
    return INTERPRET_OK;

  #undef READ_BYTE
  #undef READ_CONSTANT
  #undef COMPUTE_NEXT
  #undef BINARY_OPERATION
}

Results interpret(VM* vm, const char* source) {
  Chunk chunk;

  initialize_chunk(&chunk);

  if(!compile(&chunk, source)) {
    free_chunk(&chunk);
    return INTERPRET_COMPILE_ERROR;
  }

  vm->chunk = &chunk;

  Results result = run(vm);

  free_chunk(&chunk);

  return result;
}