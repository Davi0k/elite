#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "tokenizer.h"
#include "vm.h"

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

  error(parser, parser->current, parser->current.start);
}

bool compile(Chunk* chunk, const char* source) {
  Parser parser;

  set_tokenizer(&parser.tokenizer, source);

  parser.error = false; parser.panic = false;

  advance(&parser);

  consume(&parser, TOKEN_EOF, "Expect end of expression.");

  return !parser.error;
}