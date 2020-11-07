#ifndef OBJECT_H
#define OBJECT_H

#include "common.h"
#include "vm.h"
#include "utilities/chunk.h"
#include "types/value.h"

#define OBJECT_TYPE(value) ( AS_OBJECT(value)->type )

#define IS_STRING(value) validate(value, OBJECT_STRING)
#define IS_FUNCTION(value) validate(value, OBJECT_FUNCTION);

#define AS_STRING(value) ( (String*)AS_OBJECT(value) )
#define AS_FUNCTION(value) ( (Function*)AS_OBJECT(value) )

typedef enum {
  OBJECT_STRING,
  OBJECT_FUNCTION
} Objects;

typedef struct Object {
  Objects type;
  struct Object* next;
} Object;

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
} Function;

String* allocate_string(VM* vm, const char* content, int length, uint32_t hash);

String* copy_string(VM* vm, const char* content, int length);
String* take_string(VM* vm, const char* content, int length);

Function* new_function(VM* vm);

void print_object(Value value);

static inline bool validate(Value value, Objects type) {
  return IS_OBJECT(value) && AS_OBJECT(value)->type == type;
}

#endif