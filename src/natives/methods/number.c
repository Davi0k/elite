#include <stdio.h>

#include "natives/methods/number.h"

#include "types/object.h"

Value lol(Value receiver, int count, Value* arguments, Handler* handler) {  
  if (count == 0) {
    mpf_t result; 
    
    mpf_init(result);

    mpf_trunc(result, AS_NUMBER(receiver)->content);

    Value truncate = OBJECT(allocate_number_from_gmp(handler->vm, result));

    mpf_clear(result);

    return truncate;
  } 
  
  return throw(handler, run_time_errors[EXPECT_ARGUMENTS_NUMBER], 2, 0, count);
}