#ifndef VALUE_H
#define VALUE_H

#include "common.h"

typedef struct Object Object;
typedef struct Number Number;
typedef struct String String;
typedef struct Upvalue Upvalue;
typedef struct Function Function;
typedef struct Closure Closure;
typedef struct NativeFunction NativeFunction;
typedef struct NativeMethod NativeMethod;
typedef struct Class Class;
typedef struct Instance Instance;
typedef struct Bound Bound;

#define BOOLEAN(value) ( (Value){ VALUE_BOOLEAN, { .boolean = value } } )
#define OBJECT(value) ( (Value){ VALUE_OBJECT, { .object = (Object*)value } } )
#define VOID ( (Value){ VALUE_VOID } )
#define UNDEFINED ( (Value){ VALUE_UNDEFINED } )

#define IS_BOOLEAN(value) ( (value).type == VALUE_BOOLEAN )
#define IS_OBJECT(value) ( (value).type == VALUE_OBJECT )
#define IS_VOID(value) ( (value).type == VALUE_VOID )
#define IS_UNDEFINED(value) ( (value).type == VALUE_UNDEFINED )

#define AS_BOOLEAN(value) ( (value).content.boolean )
#define AS_OBJECT(value) ( (value).content.object )

typedef enum {
  VALUE_BOOLEAN,
  VALUE_OBJECT,
  VALUE_VOID,
  VALUE_UNDEFINED
} Values;

typedef struct {
  Values type;

  union {
    bool boolean;
    Object* object;
  } content;
} Value;

typedef struct {
  int capacity;
  int count;
  Value* values;

  VM* vm;
} Constants;

void initialize_constants(Constants* constants, VM* vm);
void free_constants(Constants* constants);
void write_constants(Constants* constants, Value value);

bool equal(Value left, Value right);

void print_value(Value value);

#endif