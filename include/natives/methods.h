#ifndef PROTOTYPES_H
#define PROTOTYPES_H

#include "common.h"

#include "natives/handler.h"

#include "utilities/table.h"

typedef struct Prototype {
  Table properties;
} Prototype;

void load_default_native_methods(VM* vm);

extern Prototype 
  OBJECT_PROTOTYPE,
  NUMBER_PROTOTYPE,
  STRING_PROTOTYPE;

#endif