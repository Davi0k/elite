#ifndef PROTOTYPES_OBJECT_H
#define PROTOTYPES_OBJECT_H

#include "common.h"

#include "natives/handler.h"

Value equals_object_method(Value receiver, int count, Value* arguments, Handler* handler);

#endif