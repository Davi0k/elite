#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>

#include "vm.h"
#include "compiler.h"
#include "types/object.h"
#include "utilities/memory.h"
#include "natives/functions.h"
#include "natives/prototypes.h"

#define DEFAULT_THRESHOLD 1024 * 64

#define FRAME_INITIAL_CAPACITY 16

void initialize_VM(VM* vm) {
  vm->objects = NULL;
  vm->upvalues = NULL;

  vm->threshold = DEFAULT_THRESHOLD;

  vm->stack.top = vm->stack.content;

  vm->call.count = 0;
  vm->call.capacity = FRAME_INITIAL_CAPACITY;
  vm->call.frames = ALLOCATE_ARRAY(vm, Frame, vm->call.frames, 0, vm->call.capacity);

  initialize_table(&vm->strings, vm);
  initialize_table(&vm->globals, vm);
}

void free_VM(VM* vm) {
  Object* object = vm->objects;

  while (object != NULL) {
    Object* next = object->next;
    free_object(vm, object);
    object = next;
  }

  FREE_ARRAY(vm, Frame, vm->call.frames, vm->call.capacity);

  free_table(&vm->strings); 
  free_table(&vm->globals); 
}

static inline bool falsey(Value value) {
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

  size_t error = 0, counter = 0;

  for (int i = vm->call.count - 1; i >= 0; i--) {
    Frame* frame = &vm->call.frames[i];

    Function* function = frame->closure->function;

    size_t instruction = frame->ip - function->chunk.code - 1;

    if (error == instruction) {
      counter++;
      continue;
    }

    if (counter != 0) 
      fprintf(stderr, "- The above line repeats %d times. -\n", counter);

    error = instruction;

    fprintf(stderr, "[Line N°%d] in ", function->chunk.lines[error]);

    if (i == 0)
      fprintf(stderr, "Top-Level.\n");
    else if (function->identifier == NULL) 
      fprintf(stderr, "an anonymous Function\n");
    else 
      fprintf(stderr, "'%s' Function\n", function->identifier->content);

    counter = 0;
  }
}

static bool invoke_native_method(VM* vm, Value receiver, NativeMethod* native_method, int count) {
  Handler handler;

  set_handler(&handler, vm);

  CMethod c_method = native_method->c_method;

  Value result = c_method(receiver, count, vm->stack.top - count, &handler);

  vm->stack.top -= count + 1;

  push(&vm->stack, result);

  if (handler.error == true)
    error(vm, handler.message);
  
  return !handler.error;
}

static bool invoke(VM* vm, Closure* closure, int count) {
  if (count != closure->function->arity) {
    error(vm, run_time_errors[EXPECT_ARGUMENTS_NUMBER], closure->function->arity, count);
    return false;
  }

  if (vm->call.capacity < vm->call.count + 1) {
    if (GROW_CAPACITY(vm->call.capacity) > INT32_MAX / (vm->call.capacity == 0 ? 1 : vm->call.capacity)) {
      error(vm, run_time_errors[STACK_OVERFLOW]);
      return false;
    }

    int capacity = vm->call.capacity;

    vm->call.capacity = GROW_CAPACITY(capacity);

    vm->call.frames = ALLOCATE_ARRAY(vm, Frame, vm->call.frames, capacity, vm->call.capacity);
  }

  Frame* frame = &vm->call.frames[vm->call.count++];
  frame->closure = closure;
  frame->ip = closure->function->chunk.code;

  frame->slots = vm->stack.top - count - 1;

  return true;
}

static bool call(VM* vm, Value value, int count) {
  if (IS_OBJECT(value)) {
    if (OBJECT_TYPE(value) == OBJECT_CLOSURE)
      return invoke(vm, AS_CLOSURE(value), count);

    switch (OBJECT_TYPE(value)) {
      case OBJECT_NATIVE_FUNCTION: {
        Handler handler;

        set_handler(&handler, vm);

        CFunction c_function = AS_NATIVE_FUNCTION(value)->c_function;

        Value result = c_function(count, vm->stack.top - count, &handler);

        vm->stack.top -= count + 1;

        push(&vm->stack, result);

        if (handler.error == true)
          error(vm, handler.message);
        
        return !handler.error;
      }

      case OBJECT_CLASS: {
        Class* class = AS_CLASS(value);

        vm->stack.top[- count - 1] = OBJECT(new_instance(vm, class));

        Value constructor;

        if (table_get(&class->methods, class->identifier, &constructor))
          return invoke(vm, AS_CLOSURE(constructor), count);

        if (count != 0) {
          error(vm, run_time_errors[EXPECT_ARGUMENTS_NUMBER], 0, count);

          return false;
        }

        return true;
      } 

      case OBJECT_BOUND: {
        Bound* bound = AS_BOUND(value);

        vm->stack.top[- count - 1] = bound->receiver;
        
        return invoke(vm, bound->method, count);
      }
    }
  }

  error(vm, run_time_errors[CANNOT_CALL]);

  return false;
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

static bool bound(VM* vm, Table table, String* property) {
  Value method;

  if (table_get(&table, property, &method) == false)
    return false;

  Value receiver = pop(&vm->stack, 1);
  Closure* closure = AS_CLOSURE(method);
  Bound* bound = new_bound(vm, receiver, closure);

  push(&vm->stack, OBJECT(bound));

  return true;
}

static bool method(VM* vm, Class* class, String* identifier, int count) {
  Value method;

  if (table_get(&class->methods, identifier, &method) == false) {
    error(vm, run_time_errors[UNDEFINED_PROPERTY], identifier->content);

    return false;
  }

  return invoke(vm, AS_CLOSURE(method), count);
}

static Results run(VM* vm) {
  Frame* frame = &vm->call.frames[vm->call.count - 1];

  #define READ_BYTE() ( *frame->ip++ )

  #define READ_SHORT() ( frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]) )

  #define READ_CONSTANT() ( frame->closure->function->chunk.constants.values[READ_BYTE()] )

  #define COMPUTE_NEXT() goto *jump_table[READ_BYTE()]

  #define BINARY_OPERATION(operator, check) \
    do { \
      if (check) { \
        if (!IS_NUMBER(peek(&vm->stack, 0)) || !IS_NUMBER(peek(&vm->stack, 1))) { \
          error(vm, run_time_errors[MUST_BE_NUMBERS]); \
          return INTERPRET_RUNTIME_ERROR; \
        } \
      } \
      Value right = pop(&vm->stack, 1); Value left = pop(&vm->stack, 1); \
      mpf_t result; mpf_init(result); \
      operator(result, AS_NUMBER(left)->content, AS_NUMBER(right)->content); \
      push(&vm->stack, OBJECT(allocate_number_from_gmp(vm, result)) ); \
      mpf_clear(result); \
    } while(false) 

  #define BINARY_COMPARISON(operator, check) \
    do { \
      if (check) { \
        if (IS_NUMBER(peek(&vm->stack, 0)) && IS_NUMBER(peek(&vm->stack, 1))) { \
          Value right = pop(&vm->stack, 1); Value left = pop(&vm->stack, 1); \
          bool comparison = mpf_cmp(AS_NUMBER(left)->content, AS_NUMBER(right)->content) operator 0; \
          push(&vm->stack, BOOLEAN(comparison)); \
          COMPUTE_NEXT(); \
        } \
        if (IS_STRING(peek(&vm->stack, 0)) && IS_STRING(peek(&vm->stack, 1))) { \
          String* right = AS_STRING(pop(&vm->stack, 1)); String* left = AS_STRING(pop(&vm->stack, 1)); \
          bool comparison = left->length operator right->length; \
          if (left->length == right->length) comparison = memcmp(left->content, right->content, left->length) operator 0; \
          push(&vm->stack, BOOLEAN(comparison)); \
          COMPUTE_NEXT(); \
        } \
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

  OP_NEGATION: {
    if (!IS_NUMBER(peek(&vm->stack, 0))) {
      error(vm, run_time_errors[MUST_BE_NUMBER]);

      return INTERPRET_RUNTIME_ERROR;
    }

    mpf_t result; 
    
    mpf_init(result);

    mpf_neg(result, AS_NUMBER(pop(&vm->stack, 1))->content);

    push(&vm->stack, OBJECT(allocate_number_from_gmp(vm, result)) );

    mpf_clear(result);

    COMPUTE_NEXT();
  }
  
  OP_ADD: {
    if (IS_STRING(peek(&vm->stack, 0)) && IS_STRING(peek(&vm->stack, 1))) {
      String* right = AS_STRING(peek(&vm->stack, 0));
      String* left = AS_STRING(peek(&vm->stack, 1));

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

    if (IS_NUMBER(peek(&vm->stack, 0)) && IS_NUMBER(peek(&vm->stack, 1))) {
      BINARY_OPERATION(mpf_add, false);

      COMPUTE_NEXT();
    }

    error(vm, run_time_errors[MUST_BE_NUMBERS_OR_STRINGS]);

    return INTERPRET_RUNTIME_ERROR;
  }

  OP_SUBTRACT: BINARY_OPERATION(mpf_sub, true); COMPUTE_NEXT();

  OP_MULTIPLY: BINARY_OPERATION(mpf_mul, true); COMPUTE_NEXT();

  OP_DIVIDE: {
    if (!IS_NUMBER(peek(&vm->stack, 0)) || !IS_NUMBER(peek(&vm->stack, 1))) {
      error(vm, run_time_errors[MUST_BE_NUMBERS]);

      return INTERPRET_RUNTIME_ERROR;
    }

    if (mpf_cmp_ui(AS_NUMBER(peek(&vm->stack, 0))->content, 0) == 0) {
      error(vm, run_time_errors[CANNOT_DIVIDE_BY_ZERO]);

      return INTERPRET_RUNTIME_ERROR;
    }

    BINARY_OPERATION(mpf_div, false); 

    COMPUTE_NEXT();
  }

  OP_POWER: {
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

    push(&vm->stack, OBJECT(allocate_number_from_gmp(vm, result)) ); 

    mpf_clear(result);

    COMPUTE_NEXT();
  }

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

  OP_GREATER: BINARY_COMPARISON(>, true); COMPUTE_NEXT();

  OP_LESS: BINARY_COMPARISON(<, true); COMPUTE_NEXT();

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

    frame = &vm->call.frames[vm->call.count - 1];

    COMPUTE_NEXT();
  }

  OP_RETURN: {
    Value result = pop(&vm->stack, 1);

    close(vm, frame->slots);

    vm->call.count--;

    if (vm->call.count == 0) {
      pop(&vm->stack, 1);
      return INTERPRET_OK;
    }

    vm->stack.top = frame->slots;

    push(&vm->stack, result);

    frame = &vm->call.frames[vm->call.count - 1];

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

  OP_CLASS: {
    String* identifier = AS_STRING(READ_CONSTANT());

    Class* class = new_class(vm, identifier);

    push(&vm->stack, OBJECT(class));

    COMPUTE_NEXT();
  }

  OP_MEMBER: {
    Value property = peek(&vm->stack, 0);
    Class* class = AS_CLASS(peek(&vm->stack, 1));
    table_set(&class->members, AS_STRING(READ_CONSTANT()), property);
    pop(&vm->stack, 1);
    COMPUTE_NEXT();
  }

  OP_METHOD: {
    Value property = peek(&vm->stack, 0);
    Class* class = AS_CLASS(peek(&vm->stack, 1));
    table_set(&class->methods, AS_STRING(READ_CONSTANT()), property);
    pop(&vm->stack, 1);
    COMPUTE_NEXT();
  }

  OP_PROPERTY_SET: {
    if (IS_INSTANCE(peek(&vm->stack, 1)) == true) {
      Instance* instance = AS_INSTANCE(peek(&vm->stack, 1));

      String* property = AS_STRING(READ_CONSTANT());

      Value value = pop(&vm->stack, 1);

      if (table_set(&instance->fields, property, value) == false) {
        pop(&vm->stack, 1);
        push(&vm->stack, value);
        COMPUTE_NEXT();
      }

      if (bound(vm, instance->class->methods, property) == true)
        COMPUTE_NEXT();

      error(vm, run_time_errors[UNDEFINED_PROPERTY], property->content);

      return INTERPRET_RUNTIME_ERROR;
    }

    error(vm, run_time_errors[CANNOT_HAVE_PROPERTIES]);

    return INTERPRET_RUNTIME_ERROR;
  }

  OP_PROPERTY_GET: {
    if (IS_INSTANCE(peek(&vm->stack, 0)) == true) {
      Instance* instance = AS_INSTANCE(peek(&vm->stack, 0));

      String* property = AS_STRING(READ_CONSTANT());

      Value value;

      if (table_get(&instance->fields, property, &value) == true) {
        pop(&vm->stack, 1);
        push(&vm->stack, value);
        COMPUTE_NEXT();
      }

      if (bound(vm, instance->class->methods, property) == true)
        COMPUTE_NEXT();

      error(vm, run_time_errors[UNDEFINED_PROPERTY], property->content);

      return INTERPRET_RUNTIME_ERROR;
    }

    error(vm, run_time_errors[CANNOT_HAVE_PROPERTIES]);

    return INTERPRET_RUNTIME_ERROR;
  }

  OP_INVOKE: {
    String* identifier = AS_STRING(READ_CONSTANT());

    int count = READ_BYTE();

    Value receiver = peek(&vm->stack, count);

    if (IS_INSTANCE(receiver) == true) {
      Instance* instance = AS_INSTANCE(receiver);

      Value value;

      if (table_get(&instance->fields, identifier, &value) == true) {
        vm->stack.top[- count - 1] = value;
        
        return call(vm, value, count);
      }

      if (method(vm, instance->class, identifier, count) == false)
        return INTERPRET_RUNTIME_ERROR;

      frame = &vm->call.frames[vm->call.count - 1];

      COMPUTE_NEXT();
    }

    if (IS_NUMBER(receiver) == true) {
      Number* number = AS_NUMBER(receiver);

      Value value;

      if (table_get(&number->prototype->methods, identifier, &value) == true) {
        vm->stack.top[- count - 1] = value;

        if(invoke_native_method(vm, receiver, AS_NATIVE_METHOD(value), count) == false)
          return INTERPRET_RUNTIME_ERROR;

        COMPUTE_NEXT();
      }

      error(vm, run_time_errors[UNDEFINED_METHOD], identifier->content);

      return INTERPRET_RUNTIME_ERROR;
    }

    error(vm, run_time_errors[DONT_SUPPORT_METHODS]);

    return INTERPRET_RUNTIME_ERROR;
  }

  OP_INHERIT: {
    Class* superclass = AS_CLASS(peek(&vm->stack, 1));
    Class* subclass = AS_CLASS(peek(&vm->stack, 0));

    table_append(&superclass->members, &subclass->members);
    table_append(&superclass->methods, &subclass->methods);

    pop(&vm->stack, 1);

    COMPUTE_NEXT();
  }

  OP_SUPER: {
    String* identifier = AS_STRING(READ_CONSTANT());

    Class* superclass = AS_CLASS(pop(&vm->stack, 1));

    if (bound(vm, superclass->methods, identifier) == true)
      COMPUTE_NEXT();

    error(vm, run_time_errors[UNDEFINED_PROPERTY], identifier->content);

    return INTERPRET_RUNTIME_ERROR;
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
  mpf_set_default_prec(GMP_MAX_PRECISION);

  load_default_native_functions(vm);
  load_default_native_methods(vm);

  Function* function = compile(vm, source);

  if (function == NULL) return INTERPRET_COMPILE_ERROR;

  push(&vm->stack, OBJECT(function));

  Closure* closure = new_closure(vm, function);

  Value value = OBJECT(closure);

  pop(&vm->stack, 1);
  push(&vm->stack, value);

  invoke(vm, closure, 0);

  return run(vm);
}