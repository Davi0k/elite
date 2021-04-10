#ifndef FUNCTIONS_H
#define FUNCTIONS_H
 
#include "common.h"

#include "natives/handler.h"

Value stopwatch_native(int count, Value* arguments, Handler* handler);
Value number_native(int count, Value* arguments, Handler* handler);
Value print_native(int count, Value* arguments, Handler* handler);
Value input_native(int count, Value* arguments, Handler* handler);
Value length_native(int count, Value* arguments, Handler* handler);
Value type_native(int count, Value* arguments, Handler* handler);

#endif