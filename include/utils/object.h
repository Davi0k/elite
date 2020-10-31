#ifndef OBJECT_H
#define OBJECT_H

#include "common.h"
#include "utils/value.h"

#define OBJECT_TYPE(value) ( AS_OBJECT(value)->type )

#define IS_STRING(value) validate(value, OBJECT_STRING)

#define AS_STRING(value) ( (String*)AS_OBJECT(value) )

typedef enum {
  OBJECT_STRING
} Objects;

typedef struct Object {
  Objects type;
  struct Object* next;
} Object;

typedef struct String {
  Object Object;
  int length;
  char* content;
} String;

String* allocate_string(char* content, int length);

String* copy_string(const char* content, int length);

void print_object(Value value);

static inline bool validate(Value value, Objects type) {
  return IS_OBJECT(value) && AS_OBJECT(value)->type == type;
}

#endif