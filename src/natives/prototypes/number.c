#include <stdio.h>
#include <string.h>

#include "natives/prototypes/number.h"

#include "types/object.h"

Value truncate_number_method(Value receiver, int count, Value* arguments, Handler* handler) {  
  if (count == 0) {
    mpf_t truncate; 
    
    mpf_init(truncate);
    mpf_trunc(truncate, AS_NUMBER(receiver)->content);

    Value result = OBJECT(allocate_number_from_gmp(handler->vm, truncate));

    mpf_clear(truncate);

    return result;
  } 
  
  return throw(handler, run_time_errors[EXPECT_ARGUMENTS_NUMBER], 2, 0, count);
}

Value ceil_number_method(Value receiver, int count, Value* arguments, Handler* handler) {  
  if (count == 0) {
    mpf_t ceil; 
    
    mpf_init(ceil);
    mpf_ceil(ceil, AS_NUMBER(receiver)->content);

    Value result = OBJECT(allocate_number_from_gmp(handler->vm, ceil));

    mpf_clear(ceil);

    return result;
  } 
  
  return throw(handler, run_time_errors[EXPECT_ARGUMENTS_NUMBER], 2, 0, count);
}

Value floor_number_method(Value receiver, int count, Value* arguments, Handler* handler) {  
  if (count == 0) {
    mpf_t floor; 
    
    mpf_init(floor);
    mpf_floor(floor, AS_NUMBER(receiver)->content);

    Value result = OBJECT(allocate_number_from_gmp(handler->vm, floor));

    mpf_clear(floor);

    return result;
  } 
  
  return throw(handler, run_time_errors[EXPECT_ARGUMENTS_NUMBER], 2, 0, count);
}