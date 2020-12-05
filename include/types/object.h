#ifndef OBJECT_H
#define OBJECT_H

#include "common.h"

#include "utilities/native.h"
#include "utilities/table.h"
#include "utilities/chunk.h"
#include "types/value.h"

#define GMP_NEUTRAL(vm) ( OBJECT(allocate_number_from_double(vm, 1.0)) )

#define OBJECT_TYPE(value) ( AS_OBJECT(value)->type )

#define NUMBER(vm, value) ( OBJECT(allocate_number(vm, value))  )

#define STRING(vm, content, length) ( OBJECT(copy_string(vm, content, length)) )

#define IS_NUMBER(value) validate(value, OBJECT_NUMBER)
#define IS_STRING(value) validate(value, OBJECT_STRING)
#define IS_FUNCTION(value) validate(value, OBJECT_FUNCTION)
#define IS_CLOSURE(value) validate(value, OBJECT_CLOSURE)
#define IS_CLASS(value) validate(value, OBJECT_CLASS)
#define IS_INSTANCE(value) validate(value, OBJECT_INSTANCE)
#define IS_NATIVE(value) validate(value, OBJECT_NATIVE)

#define AS_NUMBER(value) ( (Number*)AS_OBJECT(value) )
#define AS_STRING(value) ( (String*)AS_OBJECT(value) )
#define AS_FUNCTION(value) ( (Function*)AS_OBJECT(value) )
#define AS_CLOSURE(value) ( (Closure*)AS_OBJECT(value) )
#define AS_CLASS(value) ( (Class*)AS_OBJECT(value) )
#define AS_INSTANCE(value) ( (Instance*)AS_OBJECT(value) )
#define AS_NATIVE(value) ( (Native*)AS_OBJECT(value) )

typedef enum {
  OBJECT_UPVALUE,
  OBJECT_NUMBER,
  OBJECT_STRING,
  OBJECT_FUNCTION,
  OBJECT_CLOSURE,
  OBJECT_CLASS,
  OBJECT_INSTANCE,
  OBJECT_NATIVE
} Objects;

typedef struct Object {
  Objects type;
  struct Object* next;
  bool mark;
} Object;

typedef struct Upvalue {
  Object object;
  Value* location;
  Value closed;
  struct Upvalue* next;
} Upvalue;

typedef struct Number {
  Object object;
  mpf_t content;
} Number;

typedef struct String {
  Object object;
  int length;
  char* content;
  uint32_t hash;
} String;

typedef struct Function {
  Object object;
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

typedef struct Class {
  Object object;
  String* identifier;
} Class;

typedef struct Instance{
  Object object;
  Class* class;
  Table fields;
} Instance;

typedef struct Native {
  Object object;
  Internal internal;
} Native;

Upvalue* new_upvalue(VM* vm, Value* location);

Number* allocate_number(VM* vm, mpf_t value);

Number* allocate_number_from_double(VM* vm, double value);
Number* allocate_number_from_string(VM* vm, const char* value);

String* allocate_string(VM* vm, const char* content, int length, uint32_t hash);

String* copy_string(VM* vm, const char* content, int length);
String* take_string(VM* vm, const char* content, int length);

Function* new_function(VM* vm);

Closure* new_closure(VM* vm, Function* function);

Class* new_class(VM* vm, String* identifier);

Instance* new_instance(VM* vm, Class* class);

Native* new_native(VM* vm, Internal internal);

void print_object(Value value);

static inline bool validate(Value value, Objects type) {
  return IS_OBJECT(value) && AS_OBJECT(value)->type == type;
}

#endif