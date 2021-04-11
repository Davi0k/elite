#include <string.h>

#include "natives/methods.h"
#include "natives/prototypes/object.h"
#include "natives/prototypes/number.h"
#include "natives/prototypes/string.h"

#include "types/object.h"

#include "vm.h"

static void load_native_method(VM* vm, Prototype* prototype, const char* identifier, CMethod c_method) {
  NativeMethod* native_method = new_native_method(vm, c_method, copy_string(vm, identifier, (int)strlen(identifier)));

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
  load_object_prototype(vm, &vm->prototypes.object);
  load_object_prototype(vm, &vm->prototypes.number);
  load_object_prototype(vm, &vm->prototypes.string);

  load_number_prototype(vm, &vm->prototypes.number);
  load_string_prototype(vm, &vm->prototypes.string);
}

void initialize_prototypes(VM* vm) {
  initialize_table(&vm->prototypes.object.properties, vm);
  initialize_table(&vm->prototypes.number.properties, vm);
  initialize_table(&vm->prototypes.string.properties, vm);
}

void free_prototypes(VM* vm) {
  free_table(&vm->prototypes.object.properties);
  free_table(&vm->prototypes.number.properties);
  free_table(&vm->prototypes.string.properties);
}