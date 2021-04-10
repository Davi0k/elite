#include <stdio.h>
#include <string.h>

#include "vm.h"
#include "types/object.h"
#include "utilities/memory.h"

#define ALLOCATE_OBJECT(vm, object, type, prototype) \
  (object*)allocate_object(vm, sizeof(object), type, prototype)

static Object* allocate_object(VM* vm, size_t size, Objects type, Prototype* prototype) {
  Object* object = (Object*)reallocate(vm, NULL, 0, size);
  object->type = type;
  object->prototype = prototype;
  object->mark = false;

  object->next = vm->objects;
  vm->objects = object;

  return object;
}

Number* allocate_number_from_gmp(VM* vm, mpf_t value) {
  Number* number = ALLOCATE_OBJECT(vm, Number, OBJECT_NUMBER, &NUMBER_PROTOTYPE);
  mpf_init_set(number->content, value);
  return number;
}

Number* allocate_number_from_double(VM* vm, double value) {
  Number* number = ALLOCATE_OBJECT(vm, Number, OBJECT_NUMBER, &NUMBER_PROTOTYPE);
  mpf_init_set_d(number->content, value);
  return number;
}

Number* allocate_number_from_string(VM* vm, const char* value) {
  Number* number = ALLOCATE_OBJECT(vm, Number, OBJECT_NUMBER, &NUMBER_PROTOTYPE);
  mpf_init_set_str(number->content, value, 10);
  return number;
}

String* allocate_string(VM* vm, const char* content, int length, uint32_t hash) {
  String* string = ALLOCATE_OBJECT(vm, String, OBJECT_STRING, &STRING_PROTOTYPE);
  string->content = (char*)content;
  string->length = length;

  string->hash = hash;

  push(&vm->stack, OBJECT(string));
  table_set(&vm->strings, string, UNDEFINED);
  pop(&vm->stack, 1);

  return string;
}

String* copy_string(VM* vm, const char* content, int length) {
  uint32_t hash = hashing(content, length);

  String* intern = table_find_string(&vm->strings, content, length, hash);

  if (intern != NULL) return intern;

  char* heap = ALLOCATE(vm, char, length + 1);
  memcpy(heap, content, length);
  heap[length] = '\0';

  return allocate_string(vm, heap, length, hash);
}

String* take_string(VM* vm, const char* content, int length) {
  uint32_t hash = hashing(content, length);

  String* intern = table_find_string(&vm->strings, content, length, hash);

  if (intern != NULL) {
    FREE_ARRAY(vm, char, (char*)content, length + 1);
    return intern;
  }

  return allocate_string(vm, content, length, hash);
}

Upvalue* new_upvalue(VM* vm, Value* location) {
  Upvalue* upvalue = ALLOCATE_OBJECT(vm, Upvalue, OBJECT_UPVALUE, &OBJECT_PROTOTYPE);

  upvalue->location = location;
  upvalue->closed = UNDEFINED;
  upvalue->next = NULL;

  return upvalue;
}

Function* new_function(VM* vm) {
  Function* function = ALLOCATE_OBJECT(vm, Function, OBJECT_FUNCTION, &OBJECT_PROTOTYPE);

  function->arity = 0;
  function->count = 0;
  function->identifier = NULL;

  initialize_chunk(&function->chunk, vm);

  return function;
}

Closure* new_closure(VM* vm, Function* function) {
  int count = function->count;

  Upvalue** upvalues = ALLOCATE(vm, Upvalue*, count);

  for (int i = 0; i < count; i++)
    upvalues[i] = NULL;

  Closure* closure = ALLOCATE_OBJECT(vm, Closure, OBJECT_CLOSURE, &OBJECT_PROTOTYPE);
  closure->function = function;
  closure->upvalues = upvalues;
  closure->count = count;

  return closure;
}

NativeFunction* new_native_function(VM* vm, CFunction c_function) {
  NativeFunction* native_function = ALLOCATE_OBJECT(vm, NativeFunction, OBJECT_NATIVE_FUNCTION, &OBJECT_PROTOTYPE);
  native_function->c_function = c_function;
  return native_function;
}

NativeMethod* new_native_method(VM* vm, CMethod c_method) {
  NativeMethod* native_method = ALLOCATE_OBJECT(vm, NativeMethod, OBJECT_NATIVE_METHOD, &OBJECT_PROTOTYPE);
  native_method->c_method = c_method;
  return native_method;
}

Class* new_class(VM* vm, String* identifier) {
  Class* class = ALLOCATE_OBJECT(vm, Class, OBJECT_CLASS, &OBJECT_PROTOTYPE);

  class->identifier = identifier;

  initialize_table(&class->members, vm);
  initialize_table(&class->methods, vm);

  return class;
}

Instance* new_instance(VM* vm, Class* class) {
  Instance* instance = ALLOCATE_OBJECT(vm, Instance, OBJECT_INSTANCE, &OBJECT_PROTOTYPE);

  instance->class = class;

  initialize_table(&instance->fields, vm);

  table_append(&class->members, &instance->fields);

  return instance;
}

Bound* new_bound(VM* vm, Value receiver, Closure* method) {
  Bound* bound = ALLOCATE_OBJECT(vm, Bound, OBJECT_BOUND, &OBJECT_PROTOTYPE);

  bound->receiver = receiver;
  bound->method = method;

  return bound;
}

NativeBound* new_native_bound(VM* vm, Value receiver, NativeMethod* method) {
  NativeBound* native_bound = ALLOCATE_OBJECT(vm, NativeBound, OBJECT_NATIVE_BOUND, &OBJECT_PROTOTYPE);

  native_bound->receiver = receiver;
  native_bound->method = method;

  return native_bound;
}

void print_object(Value value) {
  switch (OBJECT_TYPE(value)) {
    case OBJECT_NUMBER: gmp_printf("%.Ff", AS_NUMBER(value)->content); break;

    case OBJECT_STRING: printf("%s", AS_STRING(value)->content); break;

    case OBJECT_FUNCTION:
    case OBJECT_CLOSURE: {
      Function* function;     

      if (OBJECT_TYPE(value) == OBJECT_FUNCTION)
        function = AS_FUNCTION(value);
      else function = AS_CLOSURE(value)->function;

      if (function->identifier == NULL)
        printf("<Anonymous Function>");
      else printf("<Function %s>", function->identifier->content);

      break;
    }

    case OBJECT_NATIVE_FUNCTION: printf("<NativeFunction>"); break;

    case OBJECT_CLASS: printf("<Class %s>", AS_CLASS(value)->identifier->content); break;

    case OBJECT_INSTANCE: printf("<Instance of %s>", AS_INSTANCE(value)->class->identifier->content); break; 

    case OBJECT_BOUND: printf("<Method %s>", AS_BOUND(value)->method->function->identifier->content); break;

    case OBJECT_NATIVE_BOUND: printf("<NativeMethod>"); break;
  }
}