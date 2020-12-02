#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>

#include "vm.h"
#include "types/object.h"
#include "utilities/memory.h"
#include "helpers/stack.h"

void reset(VM* vm) {
  vm->count = 0;
  vm->capacity = FRAME_DEFAULT_SIZE;
  vm->frames = GROW_ARRAY(vm, Frame, vm->frames, 0 , vm->capacity);
  
  vm->upvalues = NULL;

  vm->stack.top = vm->stack.content;
}

void initialize_VM(VM* vm) {
  vm->frames = NULL;
  vm->objects = NULL;

  vm->allocate = 0;

  vm->garbage = DEFAULT_THRESHOLD;

  reset(vm);

  initialize_table(&vm->strings, vm);
  initialize_table(&vm->globals, vm);

  native(vm, "stopwatch", stopwatch_native);
  native(vm, "number", number_native);
  native(vm, "print", print_native);
  native(vm, "input", input_native);
}

void free_VM(VM* vm) {
  Object* object = vm->objects;

  while (object != NULL) {
    Object* next = object->next;
    free_object(vm, object);
    object = next;
  }

  free_table(&vm->strings); 
  free_table(&vm->globals); 

  FREE_ARRAY(vm, Frame, vm->frames, vm->capacity);
}

void native(VM* vm, const char* identifier, Internal internal) {
  push(&vm->stack, OBJECT(copy_string(vm, identifier, (int)strlen(identifier))));
  push(&vm->stack, OBJECT(new_native(vm, internal)));

  table_set(&vm->globals, AS_STRING(vm->stack.content[0]), vm->stack.content[1]);

  pop(&vm->stack, 2);
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

  int previous = 0, counter = 0;

  for (int i = vm->count - 1; i >= 0; i--) {
    Frame* frame = &vm->frames[i];

    Function* function = frame->closure->function;

    size_t instruction = frame->ip - function->chunk.code - 1;

    if (previous == function->chunk.lines[instruction]) {
      counter = counter + 1;

      continue;
    }

    if (counter != 0) {
      fprintf(stderr, "- The above line repeats %d times. -\n", counter);
      counter = 0;
    }

    previous = function->chunk.lines[instruction];

    fprintf(stderr, "[Line N°%d] in ", previous);

    if (i == 0)
      fprintf(stderr, "Script.\n");
    else if (function->identifier == NULL) 
      fprintf(stderr, "an anonymous Function\n");
    else 
      fprintf(stderr, "'%s' Function\n", function->identifier->content);
  }

  reset(vm);
}

static bool invoke(VM* vm, Closure* closure, int count) {
  if (count != closure->function->arity) {
    error(vm, run_time_errors[EXPECT_ARGUMENTS_NUMBER], closure->function->arity, count);
    return false;
  }

  if (vm->capacity < vm->count + 1) {
    if (GROW_CAPACITY(vm->capacity) > INT32_MAX / (vm->capacity == 0 ? 1 : vm->capacity)) {
      error(vm, run_time_errors[STACK_OVERFLOW]);
      return false;
    }

    int capacity = vm->capacity;

    vm->capacity = GROW_CAPACITY(capacity);

    vm->frames = GROW_ARRAY(vm, Frame, vm->frames, capacity, vm->capacity);
  }

  Frame* frame = &vm->frames[vm->count++];
  frame->closure = closure;
  frame->ip = closure->function->chunk.code;

  frame->slots = vm->stack.top - count - 1;

  return true;
}

static bool call(VM* vm, Value value, int count) {
  if (IS_OBJECT(value)) {
    switch (OBJECT_TYPE(value)) {
      case OBJECT_CLOSURE:
        return invoke(vm, AS_CLOSURE(value), count);

      case OBJECT_NATIVE: {
        Handler handler;

        set_handler(&handler, vm);

        Internal internal = AS_NATIVE(value);

        Value result = internal(count, vm->stack.top - count, &handler);

        vm->stack.top -= count + 1;

        push(&vm->stack, result);

        if (handler.error == true)
          error(vm, handler.message);
        
        return !handler.error;
      }
    }
  }

  error(vm, run_time_errors[CANNOT_CALL]);
}

static Upvalue* capture(VM* vm, Value* local) {
  Upvalue* previous = NULL;

  Upvalue* upvalue = vm->upvalues;

  while (upvalue != NULL && upvalue->location > local) {
    previous = upvalue;
    upvalue = upvalue->next;
  }

  if (upvalue != NULL && upvalue->location == local)
    return upvalue;

  Upvalue* new = new_upvalue(vm, local);

  new->next = upvalue;

  if (previous == NULL)
    vm->upvalues = new;
  else previous->next = new;

  return new;
}

static void close(VM* vm, Value* last) {
  while (vm->upvalues != NULL && vm->upvalues->location >= last) {
    Upvalue* upvalue = vm->upvalues;
    upvalue->closed = *upvalue->location;
    upvalue->location = &upvalue->closed;
    vm->upvalues = upvalue->next;
  }
}

static Results run(VM* vm) {
  Frame* frame = &vm->frames[vm->count - 1];

  #define READ_BYTE() ( *frame->ip++ )

  #define READ_SHORT() ( frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]) )

  #define READ_CONSTANT() ( frame->closure->function->chunk.constants.values[READ_BYTE()] )

  #define COMPUTE_NEXT() goto *jump_table[READ_BYTE()]

  #define BINARY_OPERATION(operator) \
    do { \
      if (!IS_NUMBER(peek(&vm->stack, 0)) || !IS_NUMBER(peek(&vm->stack, 1))) { \
        error(vm, run_time_errors[MUST_BE_NUMBERS]); \
        return INTERPRET_RUNTIME_ERROR; \
      } \
      Value right = pop(&vm->stack, 1); \
      Value left = pop(&vm->stack, 1); \
      mpf_t result; mpf_init(result); \
      operator(result, AS_NUMBER(left)->content, AS_NUMBER(right)->content); \
      push(&vm->stack, NUMBER(vm, result)); \
      mpf_clear(result); \
    } while(false) 

  #define BINARY_COMPARISON(operator) \
    do { \
      if (IS_NUMBER(peek(&vm->stack, 0)) && IS_NUMBER(peek(&vm->stack, 1))) { \
        Value right = pop(&vm->stack, 1); \
        Value left = pop(&vm->stack, 1); \
        bool comparison = mpf_cmp(AS_NUMBER(left)->content, AS_NUMBER(right)->content) operator 0; \
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
      error(vm, run_time_errors[MUST_BE_NUMBERS_OR_STRINGS]); \
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
      error(vm, run_time_errors[MUST_BE_NUMBER]);

      return INTERPRET_RUNTIME_ERROR;
    }

    mpf_t number; 
    
    mpf_init(number);

    mpf_neg(number, AS_NUMBER(pop(&vm->stack, 1))->content);

    push(&vm->stack, NUMBER(vm, number));

    mpf_clear(number);

    COMPUTE_NEXT();
  
  OP_ADD: 
    if (IS_NUMBER(peek(&vm->stack, 0)) && IS_NUMBER(peek(&vm->stack, 1))) {
      BINARY_OPERATION(mpf_add);

      COMPUTE_NEXT();
    }

    if (IS_STRING(peek(&vm->stack, 0)) && IS_STRING(peek(&vm->stack, 1))) {
      String* right = AS_STRING(peek(&vm->stack, 0));
      String* left = AS_STRING(peek(&vm->stack, 0));

      int length = left->length + right->length;

      char* content = ALLOCATE(vm, char, length + 1);

      memcpy(content, left->content, left->length);
      memcpy(content + left->length, right->content, right->length);

      content[length] = '\0';

      String* result = take_string(vm, content, length);

      pop(&vm->stack, 2);

      push(&vm->stack, OBJECT(result));

      COMPUTE_NEXT();
    }

    error(vm, run_time_errors[MUST_BE_NUMBERS_OR_STRINGS]);

    return INTERPRET_RUNTIME_ERROR;

  OP_SUBTRACT: BINARY_OPERATION(mpf_sub); COMPUTE_NEXT();

  OP_MULTIPLY: BINARY_OPERATION(mpf_mul); COMPUTE_NEXT();

  OP_DIVIDE: BINARY_OPERATION(mpf_div); COMPUTE_NEXT();

  OP_POWER: 
    if (!IS_NUMBER(peek(&vm->stack, 0)) || !IS_NUMBER(peek(&vm->stack, 1))) { 
      error(vm, run_time_errors[MUST_BE_NUMBER]); 
      return INTERPRET_RUNTIME_ERROR; 
    } 

    Value right = pop(&vm->stack, 1); 
    Value left = pop(&vm->stack, 1); 

    mpf_t result; 
      
    mpf_init(result); 

    unsigned long exponent = (unsigned long)(mpf_get_d(AS_NUMBER(right)->content));

    mpf_pow_ui(result, AS_NUMBER(left)->content, exponent);

    push(&vm->stack, NUMBER(vm, result)); 

    mpf_clear(result);

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
      error(vm, run_time_errors[UNDEFINED_VARIABLE], identifier->content);
      return INTERPRET_RUNTIME_ERROR;
    }

    COMPUTE_NEXT();
  }

  OP_GLOBAL_GET: {
    String* identifier = AS_STRING(READ_CONSTANT());

    Value value;

    if (table_get(&vm->globals, identifier, &value) == false) {
      error(vm, run_time_errors[UNDEFINED_VARIABLE], identifier->content);
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

  OP_UP_SET: {
    uint8_t slot = READ_BYTE();
    *frame->closure->upvalues[slot]->location = peek(&vm->stack, 0);
    COMPUTE_NEXT();
  }

  OP_UP_GET: {
    uint8_t slot = READ_BYTE();
    push(&vm->stack, *frame->closure->upvalues[slot]->location);
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

    close(vm, frame->slots);

    vm->count--;

    if (vm->capacity > FRAME_DEFAULT_SIZE && vm->count < vm->capacity / 2) {
      int capacity = vm->capacity;

      vm->capacity = vm->capacity / 2;

      vm->frames = GROW_ARRAY(vm, Frame, vm->frames, capacity, vm->capacity);
    }

    if (vm->count == 0) {
      pop(&vm->stack, 1);
      return INTERPRET_OK;
    }

    vm->stack.top = frame->slots;

    push(&vm->stack, result);

    frame = &vm->frames[vm->count - 1];

    COMPUTE_NEXT();
  }

  OP_CLOSURE: {
    Function* function = AS_FUNCTION(READ_CONSTANT());
    
    Closure* closure = new_closure(vm, function);

    push(&vm->stack, OBJECT(closure));

    for (int i = 0; i < closure->count; i++) {
      uint8_t local = READ_BYTE();
      uint8_t index = READ_BYTE();

      if (local)
        closure->upvalues[i] = capture(vm, frame->slots + index);
      else closure->upvalues[i] = frame->closure->upvalues[index];
    }

    COMPUTE_NEXT();
  }

  OP_CLOSE: {
    close(vm, vm->stack.top - 1);
    pop(&vm->stack, 1);
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

  Closure* closure = new_closure(vm, function);

  Value value = OBJECT(closure);

  pop(&vm->stack, 1);
  push(&vm->stack, value);

  call(vm, value, 0);

  return run(vm);
}