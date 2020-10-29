#ifndef VALUE_H
#define VALUE_H

#include "common.h"

#define BOOLEAN(value) ( (Value){ VALUE_BOOLEAN, { .boolean = value } } )
#define VOID ( (Value){ VALUE_VOID, { .number = 0 } } )

#define IS_NUMBER(value) ( (value).type == VALUE_NUMBER )
#define IS_BOOLEAN(value) ( (value).type == VALUE_BOOLEAN )
#define IS_VOID(value) ( (value).type == VALUE_VOID )

#define AS_NUMBER(value) ( (value).content.number )
#define AS_BOOLEAN(value) ( (value).content.boolean )

typedef enum {
  VALUE_NUMBER,
  VALUE_BOOLEAN,
  VALUE_VOID
} Values;

typedef struct {
  Values type;

  union {
    mpf_t number;
    bool boolean;
  } content;
} Value;

typedef struct {
  int capacity;
  int count;
  Value* values;
} Constants;

Value NUMBER(mpf_t number);

void initialize_constants(Constants* constants);
void free_constants(Constants* constants);
void write_constants(Constants* constants, Value value);

bool equal(Value left, Value right);

void print_value(Value value);

#endif