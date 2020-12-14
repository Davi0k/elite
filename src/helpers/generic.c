#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers/generic.h"

const char* compile_time_errors[] = {
  [TOO_MANY_CONSTANTS] = "Too many Constants in one single Chunk.",
  [TOO_MANY_LOCALS] = "Too many local variables in Function.",
  [CANNOT_READ_INITIALIZER] = "Cannot read local variable in its own initializer.",
  [VARIABLE_ALREADY_DECLARE] = "A variable with this identifier has already been declared in this scope.",
  [EXPECT_OPEN_FUNCTION] = "Expect '(' after Function identifier.",
  [EXPECT_CLOSE_FUNCTION] = "Expect ')' after Function parameters.",
  [MAXIMUM_PARAMETERS] = "Cannot have more than 255 parameters.",
  [EXPECT_PARAMETER_IDENTIFIER] = "Expect a parameter identifier.",
  [MAXIMUM_JUMP_BODY] = "Too much code to jump over.",
  [MINIMUM_CAPACITY_LOOP_BODY] = "Loop body too large.",
  [EXPECT_VARIABLE_IDENTIFIER] = "Expect a variable identifier.",
  [EXPECT_SEMICOLON] = "Expect ';' after instruction.",
  [EXPECT_FUNCTION_IDENTIFIER] = "Expect a Function identifier.",
  [EXPECT_BLOCK] = "Expect '}' after block.",
  [EXPECT_ARGUMENTS] = "Expect ')' after arguments.",
  [MAXIMUM_ARGUMENTS] = "Cannot have more than 255 arguments.",
  [CANNOT_RETURN_SCRIPT] = "Cannot return from outside of a Function.",
  [EXPECT_EXPRESSION] = "Expect an expression.",
  [INVALID_ASSIGNMENT_TARGET] = "Invalid assignment Target.",
  [EXPECT_COLON_CONDITION] = "Expect ':' after condition.",
  [EXPECT_COLON_STATEMENT] = "Expect ':' after else statement.",
  [EXPECT_WHILE_STATEMENT] = "Expect 'while' statement after instructions.",
  [EXPECT_OPEN_FOR] = "Expect '(' before 'for' branch.",
  [EXPECT_CLOSE_FOR] = "Expect '(' after 'for' branch.",
  [TOO_MANY_CLOSURE_VARIABLES] = "Too many closure variables in Function.",
  [CAN_USE_SET_DEFINE_CLASS] = "Expect 'set', 'define' or 'class' statement after 'global' modifier.",
  [EXPECT_CLASS_IDENTIFIER] = "Expect Class identifier.",
  [EXPECT_OPEN_CLASS] = "Expect '{' before Class body.",
  [EXPECT_CLOSE_CLASS] = "Expect '}' after Class body.",
  [EXPECT_PROPERTY_IDENTIFIER] = "Expect property identifier.",
  [EXPECT_MEMBER_IDENTIFIER] = "Expect member identifier.",
  [EXPECT_METHOD_IDENTIFIER] = "Expect method identifier.",
  [CANNOT_USE_THIS] = "Cannot use 'this' outiside of a class method.",
  [CANNOT_RETURN_CONSTRUCTOR] = "Cannot return a value from a constructor.",
  [EXPECT_COLON_TERNARY] = "Expect ':' in ternary operator.",
  [CAN_USE_SET_DEFINE] = "Expect 'set' or 'define' statement in Class definition."
};

const char* run_time_errors[] = {
  [EXPECT_ARGUMENTS_NUMBER] = "Expected %d arguments but got %d.",
  [STACK_OVERFLOW] = "A Stack Overflow error has occured.",
  [CANNOT_CALL] = "Can only call functions and classes.",
  [MUST_BE_NUMBER] = "Operand must be a Number.",
  [MUST_BE_NUMBERS] = "Operands must be Numbers.",
  [MUST_BE_STRING] = "Operand must be a String.",
  [MUST_BE_STRINGS] = "Operands must be Strings.",
  [MUST_BE_NUMBER_OR_STRING] = "Operand must be a Number or a String.",
  [MUST_BE_NUMBERS_OR_STRINGS] = "Operands must be two Numbers or two Strings.",
  [UNDEFINED_VARIABLE] = "Undefined variable '%s'.",
  [UNDEFINED_ERROR] = "Undefined Error Message.",
  [MUST_INCLUDE_STRING] = "Expect String after 'include' statement.",
  [CANNOT_HAVE_PROPERTIES] = "Only instances can have fields.",
  [UNDEFINED_PROPERTY] = "Undefined property '%s'.",
  [ONLY_INSTANCES_HAVE_METHODS] = "Only Instances have methods."
};

const char* read_file_errors[] = {
  [CANNOT_OPEN_FILE] = "Could not open file <%s>.",
  [CANNOT_READ_FILE] = "Could not read file <%s>.",
  [NOT_ENOUGH_MEMORY] = "Not enough memory to read file."
};

char* read(const char* path, int* error) {
  FILE* file = fopen(path, "rb");

  if (file) {
    fseek(file, 0L, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(size + 1);

    if (buffer) {
      size_t bytes = fread(buffer, sizeof(char), size, file);

      if (bytes >= size) {
        buffer[bytes] = '\0';

        fclose(file);
        
        return buffer;
      }

      error = (int*)NOT_ENOUGH_MEMORY;
    
      return NULL;
    }

    error = (int*)CANNOT_READ_FILE;

    return NULL;
  }

  error = (int*)CANNOT_OPEN_FILE;

  return NULL;
}