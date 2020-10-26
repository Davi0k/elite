#ifndef TOKENIZER_H
#define TOKENIZER_H

typedef enum {
  TOKEN_EOF, TOKEN_ERROR, TOKEN_EMPTY, TOKEN_EXIT,
  TOKEN_IDENTIFIER,
  TOKEN_NUMBER, TOKEN_STRING, TOKEN_VOID,
  TOKEN_TRUE, TOKEN_FALSE,
  TOKEN_PLUS, TOKEN_MINUS, TOKEN_ASTERISK, TOKEN_SLASH,
  TOKEN_AND, TOKEN_OR, TOKEN_NOT,
  TOKEN_EQUALS, TOKEN_NOT_EQUALS, 
  TOKEN_GREATER, TOKEN_LESS, TOKEN_GREATER_EQUALS, TOKEN_LESS_EQUALS,
  TOKEN_COMMA, TOKEN_COLON, TOKEN_DOT,
  TOKEN_OPEN_PARENTHESES, TOKEN_CLOSE_PARENTHESES,
  TOKEN_OPEN_BRACES, TOKEN_CLOSE_BRACES,
  TOKEN_ASSIGN, 
  TOKEN_INCREMENT, TOKEN_DECREMENT,
  TOKEN_SET, TOKEN_GLOBAL, TOKEN_GET, TOKEN_PRINT,
  TOKEN_IF, TOKEN_ELSE, TOKEN_WHILE, TOKEN_FOR,
  TOKEN_DEFINE, TOKEN_INVOKE, TOKEN_RETURN,
  TOKEN_SEMICOLON 
} Types;

typedef struct {
  Types type;
  const char* start;
  int length;
  int line;
} Token;

typedef struct {
  const char* start;
  const char* current;
  int line;
} Tokenizer;

void set_tokenizer(Tokenizer* tokenizer, const char* source);

Token scan(Tokenizer* tokenizer);

#endif