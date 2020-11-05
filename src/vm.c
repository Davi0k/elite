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

  initialize_table(&vm->strings);
  initialize_table(&vm->globals);
}

void free_VM(VM* vm) {
  free_vm_objects(vm);

  free_table(&vm->strings); 
  free_table(&vm->globals); 
}

void reset_stack(Stack* stack) {
  stack->top = stack->content;
}

void push(Stack* stack, Value value) {
  *stack->top = value;
  stack->top++;
}

Value pop(Stack* stack, int distance) {
  stack->top = stack->top - distance;

  return *stack->top;
}

static Value peek(Stack* stack, int distance) {
  return stack->top[- 1 - distance];
}

static bool falsey(Value value) {
  return IS_VOID(value) || IS_UNDEFINED(value) || (IS_BOOLEAN(value) && AS_BOOLEAN(value) == false);
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

  #define READ_SHORT() ( ip += 2, (uint16_t)((ip[-1] << 8) | ip[0]) )

  #define READ_CONSTANT() ( vm->chunk->constants.values[READ_BYTE()] )

  #define COMPUTE_NEXT() goto *jump_table[READ_BYTE()]

  #define BINARY_OPERATION(operator) \
    do { \
      if (!IS_NUMBER(peek(&vm->stack, 0)) || !IS_NUMBER(peek(&vm->stack, 1))) { \
        runtime(vm, ip, "Operands must be Numbers."); \
        return INTERPRET_RUNTIME_ERROR; \
      } \
      Value right = pop(&vm->stack, 1); \
      Value left = pop(&vm->stack, 1); \
      mpf_t result; mpf_init(result); \
      operator(result, left.content.number, right.content.number); \
      push(&vm->stack, NUMBER(result)); \
    } while(false) 

  #define BINARY_COMPARISON(operator) \
    do { \
      if (IS_NUMBER(peek(&vm->stack, 0)) && IS_NUMBER(peek(&vm->stack, 1))) { \
        Value right = pop(&vm->stack, 1); \
        Value left = pop(&vm->stack, 1); \
        bool comparison = mpf_cmp(AS_NUMBER(left), AS_NUMBER(right)) operator 0; \
        push(&vm->stack, BOOLEAN(comparison)); \
        COMPUTE_NEXT(); \
      } \
      if (IS_STRING(peek(&vm->stack, 0)) && IS_STRING(peek(&vm->stack, 1))) { \
        String* right = AS_STRING(pop(&vm->stack, 1)); \
        String* left = AS_STRING(pop(&vm->stack, 1)); \
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

  OP_UNDEFINED: push(&vm->stack, UNDEFINED); COMPUTE_NEXT();

  OP_NEGATION:
    if (!IS_NUMBER(peek(&vm->stack, 0))) {
      runtime(vm, ip, "Operand must be a Number.");

      return INTERPRET_RUNTIME_ERROR;
    }

    mpf_t number; 
    
    mpf_init(number);

    mpf_neg(number, AS_NUMBER(pop(&vm->stack, 1)));

    push(&vm->stack, NUMBER(number));

    COMPUTE_NEXT();
  
  OP_ADD: 
    if (IS_NUMBER(peek(&vm->stack, 0)) && IS_NUMBER(peek(&vm->stack, 1))) {
      Value right = pop(&vm->stack, 1); 
      Value left = pop(&vm->stack, 1); 

      mpf_t result; 
      
      mpf_init(result); 

      mpf_add(result, left.content.number, right.content.number); 

      push(&vm->stack, NUMBER(result)); 

      COMPUTE_NEXT();
    }

    if (IS_STRING(peek(&vm->stack, 0)) && IS_STRING(peek(&vm->stack, 1))) {
      String* right = AS_STRING(pop(&vm->stack, 1));
      String* left = AS_STRING(pop(&vm->stack, 1));

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

    Value right = pop(&vm->stack, 1); 
    Value left = pop(&vm->stack, 1); 

    mpf_t result; 
      
    mpf_init(result); 

    unsigned long exponent = (unsigned long)(mpf_get_d(right.content.number));

    mpf_pow_ui(result, left.content.number, exponent);

    push(&vm->stack, NUMBER(result)); 

    COMPUTE_NEXT();

  OP_NOT:
    do {
      bool result = falsey(pop(&vm->stack, 1));
      push(&vm->stack, BOOLEAN(result));
      COMPUTE_NEXT();
    } while(false);

  OP_EQUAL:
    do {
      Value right = pop(&vm->stack, 1);
      Value left = pop(&vm->stack, 1);

      bool result = equal(left, right);

      push(&vm->stack, BOOLEAN(result));

      COMPUTE_NEXT();
    } while(false);

  OP_GREATER: BINARY_COMPARISON(>); COMPUTE_NEXT();

  OP_LESS: BINARY_COMPARISON(<); COMPUTE_NEXT();

  OP_PRINT:
    print_value(pop(&vm->stack, 1));
    printf("\n");
    COMPUTE_NEXT();

  OP_GLOBAL_INITIALIZE:
    do {
      String* identifier = AS_STRING(READ_CONSTANT());
      table_set(&vm->globals, identifier, peek(&vm->stack, 0));
      pop(&vm->stack, 1);

      COMPUTE_NEXT();
    } while(false);

  OP_GLOBAL_SET:
    do {
      String* identifier = AS_STRING(READ_CONSTANT());

      if (table_set(&vm->globals, identifier, peek(&vm->stack, 0))) {
        table_delete(&vm->globals, identifier);
        runtime(vm, ip, "Undefined variable '%s'.", identifier->content);
        return INTERPRET_RUNTIME_ERROR;
      }

      COMPUTE_NEXT();
    } while(false);

  OP_GLOBAL_GET:
    do {
      String* identifier = AS_STRING(READ_CONSTANT());

      Value value;

      if (table_get(&vm->globals, identifier, &value) == false) {
        runtime(vm, ip, "Undefined variable '%s'.", identifier->content);
        return INTERPRET_RUNTIME_ERROR;
      }

      push(&vm->stack, value);

      COMPUTE_NEXT();
    } while(false);

  OP_LOCAL_SET:
    do {
      uint8_t slot = READ_BYTE();
      vm->stack.content[slot] = peek(&vm->stack, 0);
      COMPUTE_NEXT();
    } while(false);

  OP_LOCAL_GET:
    do {
      uint8_t slot = READ_BYTE();
      push(&vm->stack, vm->stack.content[slot]);
      COMPUTE_NEXT();
    } while(false);

  OP_LOOP:
    do {
      uint16_t offset = READ_SHORT();

      ip = ip - offset;
      
      COMPUTE_NEXT();
    } while(false);

  OP_JUMP:
    do {
      uint16_t offset = READ_SHORT();

      ip = ip + offset;
     
      COMPUTE_NEXT();
    } while(false);

  OP_CONDITIONAL:
    do {
      uint16_t offset = READ_SHORT();

      Value value = peek(&vm->stack, 0);

      ip += (falsey(value) ? 1 : 0) * offset;
      
      COMPUTE_NEXT();
    } while(false);

  OP_POP: 
    pop(&vm->stack, 1); 
    COMPUTE_NEXT();

  OP_POP_N:
    do {
      uint8_t count = READ_BYTE();

      pop(&vm->stack, count);

      COMPUTE_NEXT();
    } while(false);

  OP_EMPTY: COMPUTE_NEXT();

  OP_EXIT: return INTERPRET_OK;

  #undef READ_BYTE
  #undef READ_SHORT
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