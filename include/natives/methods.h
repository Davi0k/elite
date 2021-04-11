#ifndef PROTOTYPES_H
#define PROTOTYPES_H

#include "common.h"

#include "natives/handler.h"

#include "utilities/table.h"

typedef struct Prototype {
  Table properties;
} Prototype;

typedef struct Prototypes {
  Prototype object;
  Prototype number;
  Prototype string;
} Prototypes;

void load_default_native_methods(VM* vm);

void initialize_prototypes(VM* vm);
void free_prototypes(VM* vm);

#endif