#ifndef PROTOTYPES_STRING_H
#define PROTOTYPES_STRING_H

#include "common.h"

#include "natives/handler.h"

Value contains_string_method(Value receiver, int count, Value* arguments, Handler* handler);
Value upper_string_method(Value receiver, int count, Value* arguments, Handler* handler);
Value lower_string_method(Value receiver, int count, Value* arguments, Handler* handler);

#endif