#include <stdio.h>
#include <string.h>
#include <time.h>

#include "vm.h"

#include "natives/functions.h"

#include "types/object.h"

void load_native_function(VM* vm, const char* identifier, CFunction c_function) {
  String* string = copy_string(vm, identifier, (int)strlen(identifier));

  push(&vm->stack, OBJECT(string));
  push(&vm->stack, OBJECT(new_native_function(vm, c_function, string)));

  table_set(&vm->globals, AS_STRING(vm->stack.content[0]), vm->stack.content[1]);

  pop(&vm->stack, 2);
}

void load_default_native_functions(VM* vm) {
  load_native_function(vm, "stopwatch", stopwatch_native);
  load_native_function(vm, "number", number_native);
  load_native_function(vm, "print", print_native);
  load_native_function(vm, "input", input_native);
  load_native_function(vm, "length", length_native);
  load_native_function(vm, "type", type_native);
}

Value stopwatch_native(int count, Value* arguments, Handler* handler) {
  if (count == 0) {
    double seconds = (double)clock() / CLOCKS_PER_SEC;

    return OBJECT(allocate_number_from_double(handler->vm, seconds));
  } 

  return throw(handler, run_time_errors[EXPECT_ARGUMENTS_NUMBER], 2, 0, count);
}

Value number_native(int count, Value* arguments, Handler* handler) {
  if (count == 0) 
    return OBJECT(allocate_number_from_double(handler->vm, 0.0));

  if (count == 1) {
    Value argument = arguments[0];

    if (IS_NUMBER(argument))
      return argument;

    if (IS_STRING(argument))
      return OBJECT(allocate_number_from_string(handler->vm, AS_STRING(argument)->content));

    return throw(handler, run_time_errors[MUST_BE_NUMBER_OR_STRING], 0);
  }

  return throw(handler, run_time_errors[EXPECT_ARGUMENTS_NUMBER], 2, 1, count);
}

Value print_native(int count, Value* arguments, Handler* handler) {
  for (int i = 0; i < count; i++)
    print_value(arguments[i]);
  
  printf("\n");

  return UNDEFINED;
}

Value input_native(int count, Value* arguments, Handler* handler) {
  if (count == 0 || count == 1) {
    if (count == 1) {
      if (IS_STRING(arguments[0]) == true) 
        print_value(arguments[0]);
      else return throw(handler, run_time_errors[MUST_BE_STRING], 0);
    }

    char* content = NULL;

    size_t size = 0;

    ssize_t length = getline(&content, &size, stdin);

    return OBJECT(copy_string(handler->vm, content, length - 1));
  } 
  
  return throw(handler, run_time_errors[EXPECT_ARGUMENTS_NUMBER], 2, 1, count);
}

Value length_native(int count, Value* arguments, Handler* handler) {
  if (count == 1) {
    Value value = arguments[0];

    if (IS_STRING(value) == true) 
      return OBJECT(allocate_number_from_double(handler->vm, AS_STRING(value)->length));
    else return throw(handler, run_time_errors[MUST_BE_STRING], 0);
  } 
  
  return throw(handler, run_time_errors[EXPECT_ARGUMENTS_NUMBER], 2, 1, count);
}

Value type_native(int count, Value* arguments, Handler* handler) {
  if (count == 1) {
    Value value = arguments[0];

    char* type = NULL;

    switch (value.type) {
      case VALUE_BOOLEAN: type = "boolean"; break;
      case VALUE_VOID: type = "void"; break;
      case VALUE_UNDEFINED: type = "undefined"; break;
    }

    if (value.type == VALUE_OBJECT) {
      if (IS_NUMBER(value) == true) type = "number";
      if (IS_STRING(value) == true) type = "string";
      if (IS_FUNCTION(value) == true || IS_CLOSURE(value) == true) type = "function";
      if (IS_NATIVE_FUNCTION(value) == true) type = "native_function";
      if (IS_CLASS(value) == true) type = "class";
      if (IS_INSTANCE(value) == true) type = "instance";
      if (IS_BOUND(value) == true) type = "method";
      if (IS_NATIVE_BOUND(value) == true) type = "native_method";
    }

    return OBJECT(copy_string(handler->vm, type, strlen(type)));
  } 
  
  return throw(handler, run_time_errors[EXPECT_ARGUMENTS_NUMBER], 2, 1, count);
}