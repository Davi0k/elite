#ifndef OBJECT_H
#define OBJECT_H

#include "common.h"

#include "natives/handler.h"
#include "natives/functions.h"
#include "natives/prototypes.h"
#include "utilities/table.h"
#include "utilities/chunk.h"
#include "types/value.h"

#define IS_NUMBER(value) validate(value, OBJECT_NUMBER)
#define IS_STRING(value) validate(value, OBJECT_STRING)
#define IS_UPVALUE(value) validate(value, OBJECT_UPVALUE)
#define IS_FUNCTION(value) validate(value, OBJECT_FUNCTION)
#define IS_CLOSURE(value) validate(value, OBJECT_CLOSURE)
#define IS_NATIVE_FUNCTION(value) validate(value, OBJECT_NATIVE_FUNCTION)
#define IS_NATIVE_METHOD(value) validate(value, OBJECT_NATIVE_METHOD)
#define IS_CLASS(value) validate(value, OBJECT_CLASS)
#define IS_INSTANCE(value) validate(value, OBJECT_INSTANCE)
#define IS_BOUND(value) validate(value, OBJECT_BOUND)

#define AS_NUMBER(value) ( (Number*)AS_OBJECT(value) )
#define AS_STRING(value) ( (String*)AS_OBJECT(value) )
#define AS_UPVALUE(value) ( (Upvalue*)AS_OBJECT(value) )
#define AS_FUNCTION(value) ( (Function*)AS_OBJECT(value) )
#define AS_CLOSURE(value) ( (Closure*)AS_OBJECT(value) )
#define AS_NATIVE_FUNCTION(value) ( (NativeFunction*)AS_OBJECT(value) )
#define AS_NATIVE_METHOD(value) ( (NativeMethod*)AS_OBJECT(value) )
#define AS_CLASS(value) ( (Class*)AS_OBJECT(value) )
#define AS_INSTANCE(value) ( (Instance*)AS_OBJECT(value) )
#define AS_BOUND(value) ( (Bound*)AS_OBJECT(value) )

#define OBJECT_TYPE(value) ( AS_OBJECT(value)->type )

typedef enum {
  OBJECT_NUMBER,
  OBJECT_STRING,
  OBJECT_UPVALUE,
  OBJECT_FUNCTION,
  OBJECT_CLOSURE,
  OBJECT_NATIVE_FUNCTION,
  OBJECT_NATIVE_METHOD,
  OBJECT_CLASS,
  OBJECT_INSTANCE,
  OBJECT_BOUND,
} Objects;

typedef struct Object {
  Objects type;
  struct Object* next;
  bool mark;
} Object;

typedef struct Number {
  Object object;
  Prototype* prototype;
  mpf_t content;
} Number;

typedef struct String {
  Object object;
  int length;
  char* content;
  uint32_t hash;
} String;

typedef struct Upvalue {
  Object object;
  Value* location;
  Value closed;
  struct Upvalue* next;
} Upvalue;

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

typedef struct NativeFunction {
  Object object;
  CFunction c_function;
} NativeFunction;

typedef struct NativeMethod {
  Object object;
  CMethod c_method;
} NativeMethod;

typedef struct Class {
  Object object;
  String* identifier;
  Table members;
  Table methods;
} Class;

typedef struct Instance {
  Object object;
  Class* class;
  Table fields;
} Instance;

typedef struct Bound {
  Object object;
  Value receiver;
  Closure* method;
} Bound;

Number* allocate_number_from_gmp(VM* vm, mpf_t value);
Number* allocate_number_from_double(VM* vm, double value);
Number* allocate_number_from_string(VM* vm, const char* value);

String* allocate_string(VM* vm, const char* content, int length, uint32_t hash);

String* copy_string(VM* vm, const char* content, int length);
String* take_string(VM* vm, const char* content, int length);

Upvalue* new_upvalue(VM* vm, Value* location);

Function* new_function(VM* vm);

Closure* new_closure(VM* vm, Function* function);

NativeFunction* new_native_function(VM* vm, CFunction c_function);

NativeMethod* new_native_method(VM* vm, CMethod c_method);

Class* new_class(VM* vm, String* identifier);

Instance* new_instance(VM* vm, Class* class);

Bound* new_bound(VM* vm, Value receiver, Closure* method);

void print_object(Value value);

static inline bool validate(Value value, Objects type) {
  return IS_OBJECT(value) && AS_OBJECT(value)->type == type;
}

#endif