#ifndef PROTOTYPES_H
#define PROTOTYPES_H

#include "common.h"

#include "natives/handler.h"

#include "utilities/table.h"

typedef struct Prototype {
  Table properties;
} Prototype;

void load_native_method(VM* vm, Prototype* prototype, const char* identifier, CMethod c_method);

void load_object_prototype(VM* vm, Prototype* prototype);
void load_number_prototype(VM* vm, Prototype* prototype);
void load_string_prototype(VM* vm, Prototype* prototype);

extern Prototype 
  OBJECT_PROTOTYPE,
  NUMBER_PROTOTYPE,
  STRING_PROTOTYPE;

#endif