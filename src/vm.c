#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "helpers/debug.h"
#include "helpers/stack.h"
#include "utilities/memory.h"
#include "types/object.h"
#include "common.h"
#include "vm.h"

typedef enum {
  EXPECT_ARGUMENTS_NUMBER,
  STACK_OVERFLOW,
  CANNOT_CALL,
  MUST_BE_NUMBER,
  MUST_BE_NUMBERS,
  MUST_BE_NUMBERS_OR_STRINGS,
  UNDEFINED_VARIABLE
} RUN_TIME_ERRORS;

const char* run_time[] = {
  "Expected %d arguments but got %d.",
  "A Stack Overflow error has occured.",
  "Can only call functions and classes.",
  "Operand must be a Number",
  "Operands must be Numbers.",
  "Operands must be two Numbers or two Strings.",
  "Undefined variable '%s'.",
};

void initialize_VM(VM* vm) {
  reset_stack(vm);

  vm->objects = NULL;

  initialize_table(&vm->strings);
  initialize_table(&vm->globals);
}

void free_VM(VM* vm) {
  Object* object = vm->objects;

  while (object != NULL) {
    Object* next = object->next;
    free_object(object);
    object = next;
  }

  free_table(&vm->strings); 
  free_table(&vm->globals); 
}

void reset_stack(VM* vm) {
  vm->stack.top = vm->stack.content;

  vm->count = 0;
}

static bool falsey(Value value) {
  return IS_VOID(value) || 
         IS_UNDEFINED(value) || 
         (IS_BOOLEAN(value) && AS_BOOLEAN(value) == false);
}

static void error(VM* vm, const char* message, ...) {
  va_list list;

  va_start(list, message);

  vfprintf(stderr, message, list);

  va_end(list);

  fputs("\n", stderr);

  for (int i = vm->count - 1; i >= 0; i--) {
    Frame* frame = &vm->frames[i];

    Function* function = frame->function;

    size_t instruction = frame->ip - function->chunk.code - 1;

    fprintf(stderr, "[Line N°%d] in ", function->chunk.lines[instruction]);

    if (function->identifier == NULL)
      fprintf(stderr, "Script.\n");
    else fprintf(stderr, "'%s' Function\n", function->identifier->content);
  }

  reset_stack(vm);
}

static bool invoke(VM* vm, Function* function, int count) {
  if (count != function->arity) {
    error(vm,  run_time[EXPECT_ARGUMENTS_NUMBER], function->arity, count);
    return false;
  }

  if (vm->count == FRAMES_MAX) {
    error(vm, run_time[STACK_OVERFLOW]);
    return false;
  }

  Frame* frame = &vm->frames[vm->count++];
  frame->function = function;
  frame->ip = function->chunk.code;

  frame->slots = vm->stack.top - count - 1;

  return true;
}

static bool call(VM* vm, Value value, int count) {
  if (IS_OBJECT(value)) {
    switch (OBJECT_TYPE(value)) {
      case OBJECT_FUNCTION:
        return invoke(vm, AS_FUNCTION(value), count);
    }
  }

  error(vm, run_time[CANNOT_CALL]);
}

static Results run(VM* vm) {
  Frame* frame = &vm->frames[vm->count - 1];

  #define READ_BYTE() ( *frame->ip++ )

  #define READ_SHORT() ( frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]) )

  #define READ_CONSTANT() ( frame->function->chunk.constants.values[READ_BYTE()] )

  #define COMPUTE_NEXT() goto *jump_table[READ_BYTE()]

  #define BINARY_OPERATION(operator) \
    do { \
      if (!IS_NUMBER(peek(&vm->stack, 0)) || !IS_NUMBER(peek(&vm->stack, 1))) { \
        error(vm, run_time[MUST_BE_NUMBERS]); \
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
      error(vm, run_time[MUST_BE_NUMBERS_OR_STRINGS]); \
      return INTERPRET_RUNTIME_ERROR; \
    } while(false)

  static void* jump_table[] = {
    FOREACH(COMPUTED)
  };

  COMPUTE_NEXT();

  OP_CONSTANT: {   
    Value constant = READ_CONSTANT();
    push(&vm->stack, constant);
    COMPUTE_NEXT();
  } 

  OP_TRUE: push(&vm->stack, BOOLEAN(true)); COMPUTE_NEXT();

  OP_FALSE: push(&vm->stack, BOOLEAN(false)); COMPUTE_NEXT();

  OP_VOID: push(&vm->stack, VOID); COMPUTE_NEXT();

  OP_UNDEFINED: push(&vm->stack, UNDEFINED); COMPUTE_NEXT();

  OP_NEGATION:
    if (!IS_NUMBER(peek(&vm->stack, 0))) {
      error(vm, run_time[MUST_BE_NUMBER]);

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

    error(vm, run_time[MUST_BE_NUMBERS_OR_STRINGS]);

    return INTERPRET_RUNTIME_ERROR;

  OP_SUBTRACT: BINARY_OPERATION(mpf_sub); COMPUTE_NEXT();

  OP_MULTIPLY: BINARY_OPERATION(mpf_mul); COMPUTE_NEXT();

  OP_DIVIDE: BINARY_OPERATION(mpf_div); COMPUTE_NEXT();

  OP_POWER: 
    if (!IS_NUMBER(peek(&vm->stack, 0)) || !IS_NUMBER(peek(&vm->stack, 1))) { 
      error(vm, run_time[MUST_BE_NUMBER]); 
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

  OP_NOT: {
    bool result = falsey(pop(&vm->stack, 1));
    push(&vm->stack, BOOLEAN(result));
    COMPUTE_NEXT();
  }

  OP_EQUAL: {
    Value right = pop(&vm->stack, 1);
    Value left = pop(&vm->stack, 1);

    bool result = equal(left, right);

    push(&vm->stack, BOOLEAN(result));

    COMPUTE_NEXT();
  }

  OP_GREATER: BINARY_COMPARISON(>); COMPUTE_NEXT();

  OP_LESS: BINARY_COMPARISON(<); COMPUTE_NEXT();

  OP_PRINT:
    print_value(pop(&vm->stack, 1));
    printf("\n");
    COMPUTE_NEXT();

  OP_GLOBAL_INITIALIZE: {
    String* identifier = AS_STRING(READ_CONSTANT());
    table_set(&vm->globals, identifier, peek(&vm->stack, 0));
    pop(&vm->stack, 1);

    COMPUTE_NEXT();
  } 

  OP_GLOBAL_SET: {
    String* identifier = AS_STRING(READ_CONSTANT());

    if (table_set(&vm->globals, identifier, peek(&vm->stack, 0))) {
      table_delete(&vm->globals, identifier);
      error(vm, run_time[UNDEFINED_VARIABLE], identifier->content);
      return INTERPRET_RUNTIME_ERROR;
    }

    COMPUTE_NEXT();
  }

  OP_GLOBAL_GET: {
    String* identifier = AS_STRING(READ_CONSTANT());

    Value value;

    if (table_get(&vm->globals, identifier, &value) == false) {
      error(vm, run_time[UNDEFINED_VARIABLE], identifier->content);
      return INTERPRET_RUNTIME_ERROR;
    }

    push(&vm->stack, value);

    COMPUTE_NEXT();
  }

  OP_LOCAL_SET: {
    uint8_t slot = READ_BYTE();
    frame->slots[slot] = peek(&vm->stack, 0);
    COMPUTE_NEXT();
  }

  OP_LOCAL_GET: {
    uint8_t slot = READ_BYTE();
    push(&vm->stack, frame->slots[slot]);
    COMPUTE_NEXT();
  }

  OP_LOOP: {
    uint16_t offset = READ_SHORT();

    frame->ip = frame->ip - offset;
    
    COMPUTE_NEXT();
  }

  OP_LOOP_CONDITIONAL: {
    uint16_t offset = READ_SHORT();

    Value value = peek(&vm->stack, 0);

    frame->ip -= (falsey(value) ? 1 : 0) * offset;
    
    COMPUTE_NEXT();
  } 

  OP_JUMP: {
    uint16_t offset = READ_SHORT();

    frame->ip = frame->ip + offset;
    
    COMPUTE_NEXT();
  }

  OP_JUMP_CONDITIONAL: {
    uint16_t offset = READ_SHORT();

    Value value = peek(&vm->stack, 0);

    frame->ip += (falsey(value) ? 1 : 0) * offset;
    
    COMPUTE_NEXT();
  } 

  OP_POP: 
    pop(&vm->stack, 1); 
    COMPUTE_NEXT();

  OP_POP_N: {
    uint8_t count = READ_BYTE();

    pop(&vm->stack, count);

    COMPUTE_NEXT();
  } 

  OP_CALL: {
    int count = READ_BYTE();

    Value value = peek(&vm->stack, count);

    if (call(vm, value, count) == false)
      return INTERPRET_RUNTIME_ERROR;

    frame = &vm->frames[vm->count - 1];

    COMPUTE_NEXT();
  }

  OP_RETURN: {
    Value result = pop(&vm->stack, 1);

    vm->count--;

    if (vm->count == 0) {
      pop(&vm->stack, 1);
      return INTERPRET_OK;
    }

    vm->stack.top = frame->slots;

    push(&vm->stack, result);

    frame = &vm->frames[vm->count - 1];

    COMPUTE_NEXT();
  }

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
  Function* function = compile(vm, source);

  if (function == NULL) return INTERPRET_COMPILE_ERROR;

  push(&vm->stack, OBJECT(function));

  call(vm, OBJECT(function), 0);

  return run(vm);
}