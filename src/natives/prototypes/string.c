#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "natives/prototypes/string.h"

#include "types/object.h"

Value contains_string_method(Value receiver, int count, Value* arguments, Handler* handler) {
  if (count == 1) {
    Value argument = arguments[0];

    if (IS_STRING(argument) == false)
      return throw(handler, run_time_errors[MUST_BE_STRING], 0);

    String* string = AS_STRING(receiver);

    if (strstr(string->content, AS_STRING(argument)->content) != NULL)
      return BOOLEAN(true);
    else return BOOLEAN(false);
  } 
  
  return throw(handler, run_time_errors[EXPECT_ARGUMENTS_NUMBER], 2, 1, count);
}

Value upper_string_method(Value receiver, int count, Value* arguments, Handler* handler) {
  if (count == 0) {
    String* string = AS_STRING(receiver);

    if (string->content[0] == '\0')
      return UNDEFINED;

    char* name = strtok(string->content, ":");

    char* character = name;

    while (*character) {
      *character = toupper((unsigned char) *character);
      character++;
    }

    return UNDEFINED;
  } 
  
  return throw(handler, run_time_errors[EXPECT_ARGUMENTS_NUMBER], 2, 0, count);
}

Value lower_string_method(Value receiver, int count, Value* arguments, Handler* handler) {
  if (count == 0) {
    String* string = AS_STRING(receiver);

    if (string->content[0] == '\0')
      return UNDEFINED;

    char* name = strtok(string->content, ":");

    char* character = name;

    while (*character) {
      *character = tolower((unsigned char) *character);
      character++;
    }

    return UNDEFINED;
  } 
  
  return throw(handler, run_time_errors[EXPECT_ARGUMENTS_NUMBER], 2, 0, count);
}