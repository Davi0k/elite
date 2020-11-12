#include "helpers/error.h"

const char* compile_time[] = {
  [TOO_MANY_CONSTANTS] = "Too many Constants in one single Chunk.",
  [TOO_MANY_LOCALS] = "Too many local variables in Function.",
  [CANNOT_READ_INITIALIZER] = "Cannot read local variable in its own initializer.",
  [VARIABLE_ALREADY_DECLARE] = "A variable with this identifier has already been declared in this scope.",
  [EXPECT_OPEN_FUNCTION] = "Expect '(' after Function identifier.",
  [EXPECT_CLOSE_FUNCTION] = "Expect ')' after Function parameters.",
  [MAXIMUM_PARAMETERS] = "Cannot have more than 255 parameters.",
  [EXPECT_PARAMETER_IDENTIFIER] = "Expect a parameter identifier.",
  [MAXIMUM_JUMP_BODY] = "Too much code to jump over.",
  [MINIMUM_LOOP_BODY] = "Loop body too large.",
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
  [TOO_MANY_CLOSURE_VARIABLES] = "Too many closure variables in Function."
};

const char* run_time[] = {
  [EXPECT_ARGUMENTS_NUMBER] = "Expected %d arguments but got %d.",
  [STACK_OVERFLOW] = "A Stack Overflow error has occured.",
  [CANNOT_CALL] = "Can only call functions and classes.",
  [MUST_BE_NUMBER] = "Operand must be a Number.",
  [MUST_BE_NUMBER_OR_STRING] = "Operand must be a Number or a String.",
  [MUST_BE_NUMBERS] = "Operands must be Numbers.",
  [MUST_BE_NUMBERS_OR_STRINGS] = "Operands must be two Numbers or two Strings.",
  [UNDEFINED_VARIABLE] = "Undefined variable '%s'.",
  [UNDEFINED_ERROR] = "Undefined Error Message."
};