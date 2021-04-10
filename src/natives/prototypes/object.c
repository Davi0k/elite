#include <stdio.h>
#include <string.h>

#include "natives/prototypes/object.h"

#include "types/object.h"

Value equals_object_method(Value receiver, int count, Value* arguments, Handler* handler) {
  if (count == 1) {
    Object* object = AS_OBJECT(receiver);

    if (object == AS_OBJECT(arguments[0]))
      return BOOLEAN(true);
    else return BOOLEAN(false);
  } 
  
  return throw(handler, run_time_errors[EXPECT_ARGUMENTS_NUMBER], 2, 1, count);
}