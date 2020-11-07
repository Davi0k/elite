#include "helpers/error.h"

const char* compile_time[] = {
  "Too many Constants in one single Chunk.",
  "Too many local variables in Function.",
  "Cannot read local variable in its own initializer.",
  "A variable with this identifier has already been declared in this scope.",
  "Expect '(' after Function identifier.",
  "Expect ')' after Function parameters.",
  "Cannot have more than 255 parameters.",
  "Expect a parameter identifier.",
  "Too much code to jump over.",
  "Loop body too large.",
  "Expect a variable identifier.",
  "Expect ';' after instruction.",
  "Expect a Function identifier.",
  "Expect '}' after block.",
  "Expect ')' after arguments.",
  "Cannot have more than 255 arguments.",
  "Cannot return from outside of a Function.",
  "Expect an expression.",
  "Invalid assignment Target.",
  "Expect ':' after condition.",
  "Expect ':' after else statement.",
  "Expect 'while' statement after instructions.",
  "Expect '(' before 'for' branch.",
  "Expect '(' after 'for' branch."
};

const char* run_time[] = {
  "Expected %d arguments but got %d.",
  "A Stack Overflow error has occured.",
  "Can only call functions and classes.",
  "Operand must be a Number",
  "Operands must be Numbers.",
  "Operands must be two Numbers or two Strings.",
  "Undefined variable '%s'.",
};