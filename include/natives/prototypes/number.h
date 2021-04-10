#ifndef PROTOTYPES_NUMBER_H
#define PROTOTYPES_NUMBER_H

#include "common.h"

#include "natives/handler.h"

Value truncate_number_method(Value receiver, int count, Value* arguments, Handler* handler);
Value ceil_number_method(Value receiver, int count, Value* arguments, Handler* handler);
Value floor_number_method(Value receiver, int count, Value* arguments, Handler* handler);

#endif