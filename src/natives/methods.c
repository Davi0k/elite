#include <string.h>

#include "natives/methods.h"
#include "natives/prototypes/object.h"
#include "natives/prototypes/number.h"
#include "natives/prototypes/string.h"

#include "types/object.h"

Prototype 
  OBJECT_PROTOTYPE,
  NUMBER_PROTOTYPE,
  STRING_PROTOTYPE;

static void load_native_method(VM* vm, Prototype* prototype, const char* identifier, CMethod c_method) {
  NativeMethod* native_method = new_native_method(vm, c_method);

  String* name = copy_string(vm, identifier, (int)strlen(identifier));

  table_set(&prototype->properties, name, OBJECT(native_method));
}

static void load_object_prototype(VM* vm, Prototype* prototype) {
  load_native_method(vm, prototype, "equals", equals_object_method);
}

static void load_number_prototype(VM* vm, Prototype* prototype) {
  load_native_method(vm, prototype, "truncate", truncate_number_method);
  load_native_method(vm, prototype, "ceil", ceil_number_method);
  load_native_method(vm, prototype, "floor", floor_number_method);
}

static void load_string_prototype(VM* vm, Prototype* prototype) {
  load_native_method(vm, prototype, "contains", contains_string_method);
  load_native_method(vm, prototype, "upper", upper_string_method);
  load_native_method(vm, prototype, "lower", lower_string_method);
}

void load_default_native_methods(VM* vm) {
  initialize_table(&OBJECT_PROTOTYPE.properties, vm);
  initialize_table(&NUMBER_PROTOTYPE.properties, vm);
  initialize_table(&STRING_PROTOTYPE.properties, vm);

  load_object_prototype(vm, &OBJECT_PROTOTYPE);
  load_object_prototype(vm, &NUMBER_PROTOTYPE);
  load_object_prototype(vm, &STRING_PROTOTYPE);

  load_number_prototype(vm, &NUMBER_PROTOTYPE);
  load_string_prototype(vm, &STRING_PROTOTYPE);
}