#ifndef PROTOTYPES_H
#define PROTOTYPES_H

#include "common.h"

#include "natives/handler.h"

#include "utilities/table.h"

typedef struct Prototype {
  Table methods;
} Prototype;

void load_default_native_methods(VM* vm);

extern Prototype number_prototype;

#endif