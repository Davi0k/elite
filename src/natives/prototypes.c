#include <string.h>

#include "natives/prototypes.h"
#include "natives/methods/number.h"

#include "types/object.h"

Prototype number_prototype;

static void load_native_method(VM* vm, Prototype* prototype, const char* identifier, CMethod c_method) {
  NativeMethod* native_method = new_native_method(vm, c_method);

  String* name = copy_string(vm, identifier, (int)strlen(identifier));

  table_set(&prototype->methods, name, OBJECT(native_method));
}

void load_default_native_methods(VM* vm) {
  initialize_table(&number_prototype.methods, vm);

  load_native_method(vm, &number_prototype, "truncate", lol);
}