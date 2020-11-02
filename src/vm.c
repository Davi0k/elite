#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "vm.h"

#include "utilities/memory.h"
#include "types/object.h"

#include "helpers/debug.h"

void initialize_VM(VM* vm) {
  reset_stack(&vm->stack);

  vm->objects = NULL;

  initialize_table(&vm->table);
}

void free_VM(VM* vm) {
  free_vm_objects(vm);

  free_table(&vm->table); 
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

  #define BINARY_OPERATION(operator) \
    do { \
      if (!IS_NUMBER(peek(&vm->stack, 0)) || !IS_NUMBER(peek(&vm->stack, 1))) { \
        runtime(vm, ip, "Operands must be Numbers."); \
        return INTERPRET_RUNTIME_ERROR; \
      } \
      Value right = pop(&vm->stack); \
      Value left = pop(&vm->stack); \
      mpf_t result; mpf_init(result); \
      operator(result, left.content.number, right.content.number); \
      push(&vm->stack, NUMBER(result)); \
    } while(false) 

  #define BINARY_COMPARISON(operator) \
    do { \
      if (IS_NUMBER(peek(&vm->stack, 0)) && IS_NUMBER(peek(&vm->stack, 1))) { \
        Value right = pop(&vm->stack); \
        Value left = pop(&vm->stack); \
        bool comparison = mpf_cmp(AS_NUMBER(left), AS_NUMBER(right)) operator 0; \
        push(&vm->stack, BOOLEAN(comparison)); \
        COMPUTE_NEXT(); \
      } \
      if (IS_STRING(peek(&vm->stack, 0)) && IS_STRING(peek(&vm->stack, 1))) { \
        String* right = AS_STRING(pop(&vm->stack)); \
        String* left = AS_STRING(pop(&vm->stack)); \
        bool comparison = left->length operator right->length; \
        if (left->length == right->length) comparison = memcmp(left->content, right->content, left->length) operator 0; \
        push(&vm->stack, BOOLEAN(comparison)); \
        COMPUTE_NEXT(); \
      } \
      runtime(vm, ip, "Operands must be two Numbers or two Strings."); \
      return INTERPRET_RUNTIME_ERROR; \
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

  OP_NEGATION:
    if (!IS_NUMBER(peek(&vm->stack, 0))) {
      runtime(vm, ip, "Operand must be a Number.");

      return INTERPRET_RUNTIME_ERROR;
    }

    mpf_t number; 
    
    mpf_init(number);

    mpf_neg(number, AS_NUMBER(pop(&vm->stack)));

    push(&vm->stack, NUMBER(number));

    COMPUTE_NEXT();
  
  OP_ADD: 
    if (IS_NUMBER(peek(&vm->stack, 0)) && IS_NUMBER(peek(&vm->stack, 1))) {
      Value right = pop(&vm->stack); 
      Value left = pop(&vm->stack); 

      mpf_t result; 
      
      mpf_init(result); 

      mpf_add(result, left.content.number, right.content.number); 

      push(&vm->stack, NUMBER(result)); 

      COMPUTE_NEXT();
    }

    if (IS_STRING(peek(&vm->stack, 0)) && IS_STRING(peek(&vm->stack, 1))) {
      String* right = AS_STRING(pop(&vm->stack));
      String* left = AS_STRING(pop(&vm->stack));

      int length = left->length + right->length;

      char* content = ALLOCATE(char, length + 1);

      memcpy(content, left->content, left->length);
      memcpy(content + left->length, right->content, right->length);

      content[length] = '\0';

      String* result = take_string(vm, content, length);

      push(&vm->stack, OBJECT(result));

      COMPUTE_NEXT();
    }

    runtime(vm, ip, "Operands must be two Numbers or two Strings.");

    return INTERPRET_RUNTIME_ERROR;

  OP_SUBTRACT: BINARY_OPERATION(mpf_sub); COMPUTE_NEXT();

  OP_MULTIPLY: BINARY_OPERATION(mpf_mul); COMPUTE_NEXT();

  OP_DIVIDE: BINARY_OPERATION(mpf_div); COMPUTE_NEXT();

  OP_POWER: 
    if (!IS_NUMBER(peek(&vm->stack, 0)) || !IS_NUMBER(peek(&vm->stack, 1))) { 
      runtime(vm, ip, "Operands must be Numbers."); 
      return INTERPRET_RUNTIME_ERROR; 
    } 

    Value right = pop(&vm->stack); 
    Value left = pop(&vm->stack); 

    mpf_t result; 
      
    mpf_init(result); 

    unsigned long exponent = (unsigned long)(mpf_get_d(right.content.number));

    mpf_pow_ui(result, left.content.number, exponent);

    push(&vm->stack, NUMBER(result)); 

    COMPUTE_NEXT();

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

  OP_GREATER: BINARY_COMPARISON(>); COMPUTE_NEXT();

  OP_LESS: BINARY_COMPARISON(<); COMPUTE_NEXT();

  OP_EXIT: 
    print_stack(&vm->stack);
    return INTERPRET_OK;

  #undef READ_BYTE
  #undef READ_CONSTANT
  #undef COMPUTE_NEXT
  #undef BINARY_OPERATION
  #undef BINARY_COMPARISON
}

Results interpret(VM* vm, const char* source) {
  Chunk chunk;

  initialize_chunk(&chunk);

  if(!compile(vm, &chunk, source)) {
    free_chunk(&chunk);
    return INTERPRET_COMPILE_ERROR;
  }

  vm->chunk = &chunk;

  Results result = run(vm);

  free_chunk(&chunk);

  return result;
}