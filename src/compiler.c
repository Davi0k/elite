#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "tokenizer.h"
#include "vm.h"

static void espression(Parser* parser);
static void parse(Parser* parser, Precedences precedence);

static void grouping(Parser* parser);
static void unary(Parser* parser);
static void binary(Parser* parser);

static void number(Parser* parser);
static void literal(Parser* parser);

Rule rules[] = { 
  [ TOKEN_EOF ] = { NULL, NULL, PRECEDENCE_NONE },

  [ TOKEN_ERROR ] = { NULL, NULL, PRECEDENCE_NONE },

  [ TOKEN_EMPTY ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_EXIT ] = { NULL, NULL, PRECEDENCE_NONE },

  [ TOKEN_IDENTIFIER ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_NUMBER ] = { number, NULL, PRECEDENCE_NONE },
  [ TOKEN_STRING ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_VOID ] = { literal, NULL, PRECEDENCE_NONE },

  [ TOKEN_TRUE ] = { literal, NULL, PRECEDENCE_NONE },
  [ TOKEN_FALSE ] = { literal, NULL, PRECEDENCE_NONE },

  [ TOKEN_PLUS ] = { unary, binary, PRECEDENCE_TERM },
  [ TOKEN_MINUS ] = { unary, binary, PRECEDENCE_TERM },
  [ TOKEN_ASTERISK ] = { NULL, binary, PRECEDENCE_FACTOR },
  [ TOKEN_SLASH ] = { NULL, binary, PRECEDENCE_FACTOR },

  [ TOKEN_AND ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_OR ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_NOT ] = { unary, NULL, PRECEDENCE_NONE },

  [ TOKEN_EQUAL ] = { NULL, binary, PRECEDENCE_EQUALITY },
  [ TOKEN_NOT_EQUAL ] = { NULL, binary, PRECEDENCE_EQUALITY },

  [ TOKEN_GREATER ] = { NULL, binary, PRECEDENCE_COMPARISON },
  [ TOKEN_LESS ] = { NULL, binary, PRECEDENCE_COMPARISON },
  [ TOKEN_GREATER_EQUAL ] = { NULL, binary, PRECEDENCE_COMPARISON },
  [ TOKEN_LESS_EQUAL ] = { NULL, binary, PRECEDENCE_COMPARISON },

  [ TOKEN_COMMA ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_COLON ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_DOT ] = { NULL, NULL, PRECEDENCE_NONE },

  [ TOKEN_OPEN_PARENTHESES ] = { grouping, NULL, PRECEDENCE_NONE },
  [ TOKEN_CLOSE_PARENTHESES ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_OPEN_BRACKETS ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_CLOSE_BRACKETS ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_OPEN_BRACES ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_CLOSE_BRACES ] = { NULL, NULL, PRECEDENCE_NONE },

  [ TOKEN_ASSIGN ] = { NULL, NULL, PRECEDENCE_NONE },

  [ TOKEN_INCREMENT ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_DECREMENT ] = { NULL, NULL, PRECEDENCE_NONE },

  [ TOKEN_SET ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_GET ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_PRINT ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_IF ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_ELSE ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_WHILE ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_FOR ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_DEFINE ] = { NULL, NULL, PRECEDENCE_NONE },

  [ TOKEN_RETURN ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_SEMICOLON ] = { NULL, NULL, PRECEDENCE_NONE}
};

static void error(Parser* parser, Token token, const char* message) {
  if (parser->panic) return;

  parser->panic = true;

  fprintf(stderr, "[Line %d] Error", token.line);

  fprintf(stderr, " at <%.*s>", token.length, token.start);

  fprintf(stderr, ": %s\n", message);

  parser->error = true;
}

static void advance(Parser* parser) {
  parser->previous = parser->current;

  while(true) {
    parser->current = scan(&parser->tokenizer);
    if (parser->current.type != TOKEN_ERROR) break;

    error(parser, parser->current, parser->current.start);
  }
}

static void consume(Parser* parser, Types type, const char* message) {
  if (parser->current.type == type) 
    return advance(parser);

  error(parser, parser->current, message);
}

static void emit(Parser* parser, uint8_t byte) {
  write_chunk(parser->compiling, byte, parser->previous.line);
}

static void stream(Parser* parser, int count, ...) {
  va_list list;

  va_start(list, count);

  for (int i = 0; i < count; i++) {
    uint8_t byte = (uint8_t)va_arg(list, int);
    emit(parser, byte);
  }

  va_end(list);
}

static uint8_t make(Parser* parser, Value value) {
  int constant = add_constant(parser->compiling, value);

  if (constant > UINT8_MAX) {
    error(parser, parser->previous, "Too many Constants in one single Chunk.");
    return 0;
  }

  return (uint8_t)constant;
}

static void constant(Parser* parser, Value value) {
  uint8_t constant = make(parser, value);

  emit(parser, OP_CONSTANT);
  emit(parser, constant);
}

static void number(Parser* parser) {
  char string[parser->previous.length + 1];

  strncpy(string, parser->previous.start, parser->previous.length + 1);

  string[parser->previous.length] = '\0';

  Value value;

  value.type = VALUE_NUMBER;

  mpf_init_set_str(value.content.number, string, 10);

  constant(parser, value);
}

static void literal(Parser* parser) {
  switch (parser->previous.type) {
    case TOKEN_TRUE: emit(parser, OP_TRUE); break;
    case TOKEN_FALSE: emit(parser, OP_FALSE); break;

    case TOKEN_VOID: emit(parser, OP_VOID); break;

    default: return;
  }
}

static void expression(Parser* parser) {
  parse(parser, PRECEDENCE_ASSIGNMENT);
}

static void grouping(Parser* parser) {
  expression(parser);
  consume(parser, TOKEN_CLOSE_PARENTHESES, "Expect a close parentheses after expression.");
}

static void unary(Parser* parser) {
  Types operator = parser->previous.type;

  parse(parser, PRECEDENCE_UNARY);

  switch (operator) {
    case TOKEN_MINUS: emit(parser, OP_NEGATION); break;

    case TOKEN_NOT: emit(parser, OP_NOT); break;

    default: return;
  }
}

static void binary(Parser* parser) {
  Types operator = parser->previous.type;

  Rule* rule = &rules[operator];

  Precedences precedence = (Precedences)(rule->precedence + 1);

  parse(parser, precedence);

  switch (operator) {
    case TOKEN_PLUS: emit(parser, OP_ADD); break;
    case TOKEN_MINUS: emit(parser, OP_SUBTRACT); break;
    case TOKEN_ASTERISK: emit(parser, OP_MULTIPLY); break;
    case TOKEN_SLASH: emit(parser, OP_DIVIDE); break;

    case TOKEN_EQUAL: emit(parser, OP_EQUAL); break;
    case TOKEN_NOT_EQUAL: stream(parser, 2, OP_EQUAL, OP_NOT); break;

    case TOKEN_GREATER: emit(parser, OP_GREATER); break;
    case TOKEN_GREATER_EQUAL: stream(parser, 2, OP_LESS, OP_NOT); break;
    case TOKEN_LESS: emit(parser, OP_LESS); break;
    case TOKEN_LESS_EQUAL: stream(parser, 2, OP_GREATER, OP_NOT); break;

    default: return;
  }
}

static void parse(Parser* parser, Precedences precedence) {
  advance(parser);

  Function prefix = rules[parser->previous.type].prefix;

  if (prefix == NULL) {
    error(parser, parser->previous, "Expect an expression.");
    return;
  }

  prefix(parser);

  while (precedence <= rules[parser->current.type].precedence) {
    advance(parser);
    Function infix = rules[parser->previous.type].infix;
    infix(parser);
  }
}

bool compile(Chunk* chunk, const char* source) {
  Parser parser; 

  parser.compiling = chunk;

  set_tokenizer(&parser.tokenizer, source);

  parser.error = false; parser.panic = false;

  advance(&parser);
  
  expression(&parser);

  consume(&parser, TOKEN_EOF, "Expect end of expression.");

  emit(&parser, OP_EXIT);

  return !parser.error;
}