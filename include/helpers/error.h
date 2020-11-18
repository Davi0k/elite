
#ifndef ERROR_H
#define ERROR_H

typedef enum {
  TOO_MANY_CONSTANTS,
  TOO_MANY_LOCALS,
  CANNOT_READ_INITIALIZER,
  VARIABLE_ALREADY_DECLARE,
  EXPECT_OPEN_FUNCTION,
  EXPECT_CLOSE_FUNCTION,
  MAXIMUM_PARAMETERS,
  EXPECT_PARAMETER_IDENTIFIER,
  MAXIMUM_JUMP_BODY,
  MINIMUM_LOOP_BODY,
  EXPECT_VARIABLE_IDENTIFIER,
  EXPECT_SEMICOLON,
  EXPECT_FUNCTION_IDENTIFIER,
  EXPECT_BLOCK,
  EXPECT_ARGUMENTS,
  MAXIMUM_ARGUMENTS,
  CANNOT_RETURN_SCRIPT,
  EXPECT_EXPRESSION,
  INVALID_ASSIGNMENT_TARGET,
  EXPECT_COLON_CONDITION,
  EXPECT_COLON_STATEMENT,
  EXPECT_WHILE_STATEMENT,
  EXPECT_OPEN_FOR,
  EXPECT_CLOSE_FOR,
  TOO_MANY_CLOSURE_VARIABLES,
  EXPECT_SET_OR_DEFINE
} COMPILE_TIME_ERRORS;

typedef enum {
  EXPECT_ARGUMENTS_NUMBER,
  STACK_OVERFLOW,
  CANNOT_CALL,
  MUST_BE_NUMBER,
  MUST_BE_NUMBERS,
  MUST_BE_STRING,
  MUST_BE_STRINGS,
  MUST_BE_NUMBER_OR_STRING,
  MUST_BE_NUMBERS_OR_STRINGS,
  UNDEFINED_VARIABLE,
  UNDEFINED_ERROR
} RUN_TIME_ERRORS;

#endif