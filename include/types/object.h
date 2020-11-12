#ifndef OBJECT_H
#define OBJECT_H

#include "common.h"
#include "vm.h"
#include "utilities/native.h"
#include "utilities/chunk.h"
#include "types/value.h"

#define GMP_NEUTRAL(vm) ( OBJECT(number_from_double(vm, 1.0)) )

#define OBJECT_TYPE(value) ( AS_OBJECT(value)->type )

#define IS_NUMBER(value) validate(value, OBJECT_NUMBER)
#define IS_STRING(value) validate(value, OBJECT_STRING)
#define IS_FUNCTION(value) validate(value, OBJECT_FUNCTION)
#define IS_CLOSURE(value) validate(value, OBJECT_CLOSURE)
#define IS_NATIVE(value) validate(value, OBJECT_NATIVE)

#define AS_NUMBER(value) ( ( (Number*)AS_OBJECT(value) )->content )
#define AS_STRING(value) ( (String*)AS_OBJECT(value) )
#define AS_FUNCTION(value) ( (Function*)AS_OBJECT(value) )
#define AS_CLOSURE(value) ( (Closure*)AS_OBJECT(value) )
#define AS_NATIVE(value) ( ( (Native*)AS_OBJECT(value) )->internal )

#define NUMBER(vm, value) ( OBJECT(new_number(vm, value)) )

#define NUMBER_FROM_DOUBLE(vm, value) ( OBJECT(number_from_double(vm, value)) )
#define NUMBER_FROM_STRING(vm, value) ( OBJECT(number_from_string(vm, value)) )

typedef enum {
  OBJECT_UPVALUE,
  OBJECT_NUMBER,
  OBJECT_STRING,
  OBJECT_FUNCTION,
  OBJECT_CLOSURE,
  OBJECT_NATIVE
} Objects;

typedef struct Object {
  Objects type;
  struct Object* next;
} Object;

typedef struct Upvalue {
  Object object;
  Value* location;
} Upvalue;

typedef struct Number {
  Object Object;
  mpf_t content;
} Number;

typedef struct String {
  Object Object;
  int length;
  char* content;
  uint32_t hash;
} String;

typedef struct Function {
  Object Object;
  String* identifier;
  Chunk chunk;
  int arity;
  int count;
} Function;

typedef struct Closure {
  Object object;
  Function* function;
  Upvalue** upvalues;
  int count;
} Closure;

typedef struct Native {
  Object object;
  Internal internal;
} Native;

Upvalue* new_upvalue(VM* vm, Value* location);

Number* new_number(VM* vm, mpf_t number);

Number* number_from_double(VM* vm, double number);
Number* number_from_string(VM* vm, const char* number);

String* allocate_string(VM* vm, const char* content, int length, uint32_t hash);

String* copy_string(VM* vm, const char* content, int length);
String* take_string(VM* vm, const char* content, int length);

Function* new_function(VM* vm);

Closure* new_closure(VM* vm, Function* function);

Native* new_native(VM* vm, Internal internal);

void print_object(Value value);

static inline bool validate(Value value, Objects type) {
  return IS_OBJECT(value) && AS_OBJECT(value)->type == type;
}

#endif