#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "tokenizer.h"
#include "types/object.h"
#include "helpers/error.h"

void set_compiler(Parser* parser, Compiler* compiler, Positions position) {
  compiler->enclosing = parser->compiler;

  compiler->function = NULL;
  compiler->position = position;

  compiler->count = 0;
  compiler->scope = 0;

  compiler->function = new_function(parser->vm);

  parser->compiler = compiler;

  if (position != POSITION_SCRIPT)
    parser->compiler->function->identifier = copy_string(parser->vm, parser->previous.start, parser->previous.length);

  Local* local = &parser->compiler->locals[parser->compiler->count++];
  local->depth = 0;
  local->identifier.start = NULL_TERMINATOR;
  local->identifier.length = 0;
}

static inline Chunk* get_current_chunk(Compiler* compiler) {
  return &compiler->function->chunk;
}

static void emit(Parser* parser, uint8_t byte) {
  write_chunk(get_current_chunk(parser->compiler), byte, parser->previous.line);
}

static void close(Parser* parser) {
  emit(parser, OP_UNDEFINED);
  emit(parser, OP_RETURN);
}

static Function* terminate_compiler(Parser* parser) {
  close(parser);

  Function* function = parser->compiler->function;

  parser->compiler = parser->compiler->enclosing;

  return function;
}

static void espression(Parser* parser);
static void parse(Parser* parser, Precedences precedence);

static void grouping(Parser* parser, bool assign);
static void unary(Parser* parser, bool assign);
static void binary(Parser* parser, bool assign);

static void number(Parser* parser, bool assign);
static void string(Parser* parser, bool assign);
static void literal(Parser* parser, bool assign);

static void and(Parser* parser, bool assign);
static void or(Parser* parser, bool assign);

static void variable(Parser* parser, bool assign);

static void call(Parser* parser, bool assign);

static void instruction(Parser* parser);
static void statement(Parser* parser);

Rule rules[] = { 
  [ TOKEN_EOF ] = { NULL, NULL, PRECEDENCE_NONE },

  [ TOKEN_ERROR ] = { NULL, NULL, PRECEDENCE_NONE },

  [ TOKEN_EMPTY ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_EXIT ] = { NULL, NULL, PRECEDENCE_NONE },

  [ TOKEN_IDENTIFIER ] = { variable, NULL, PRECEDENCE_NONE },
  [ TOKEN_NUMBER ] = { number, NULL, PRECEDENCE_NONE },
  [ TOKEN_STRING ] = { string, NULL, PRECEDENCE_NONE },
  [ TOKEN_VOID ] = { literal, NULL, PRECEDENCE_NONE },
  [ TOKEN_UNDEFINED ] = { literal, NULL, PRECEDENCE_NONE },

  [ TOKEN_TRUE ] = { literal, NULL, PRECEDENCE_NONE },
  [ TOKEN_FALSE ] = { literal, NULL, PRECEDENCE_NONE },

  [ TOKEN_PLUS ] = { unary, binary, PRECEDENCE_TERM },
  [ TOKEN_MINUS ] = { unary, binary, PRECEDENCE_TERM },
  [ TOKEN_ASTERISK ] = { NULL, binary, PRECEDENCE_FACTOR },
  [ TOKEN_SLASH ] = { NULL, binary, PRECEDENCE_FACTOR },

  [ TOKEN_CARET ] = { NULL, binary, PRECEDENCE_FACTOR },

  [ TOKEN_AND ] = { NULL, and, PRECEDENCE_AND },
  [ TOKEN_OR ] = { NULL, or, PRECEDENCE_OR },
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

  [ TOKEN_OPEN_PARENTHESES ] = { grouping, call, PRECEDENCE_CALL },
  [ TOKEN_CLOSE_PARENTHESES ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_OPEN_BRACKETS ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_CLOSE_BRACKETS ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_OPEN_BRACES ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_CLOSE_BRACES ] = { NULL, NULL, PRECEDENCE_NONE },

  [ TOKEN_ASSIGN ] = { NULL, NULL, PRECEDENCE_NONE },

  [ TOKEN_INCREMENT ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_DECREMENT ] = { NULL, NULL, PRECEDENCE_NONE },

  [ TOKEN_SET ] = { NULL, NULL, PRECEDENCE_NONE },

  [ TOKEN_PRINT ] = { NULL, NULL, PRECEDENCE_NONE },
  
  [ TOKEN_IF ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_ELSE ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_WHILE ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_DO ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_FOR ] = { NULL, NULL, PRECEDENCE_NONE },

  [ TOKEN_DEFINE ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_RETURN ] = { NULL, NULL, PRECEDENCE_NONE },

  [ TOKEN_SEMICOLON ] = { NULL, NULL, PRECEDENCE_NONE}
};

static void error(Parser* parser, Token token, const char* error) {
  if (parser->panic) return;

  parser->panic = true;

  fprintf(stderr, "[Line N°%d] Error", token.line);

  fprintf(stderr, " at <%.*s>", token.length, token.start);

  fprintf(stderr, " -> %s\n", error);

  parser->error = true;
}

static bool check(Parser* parser, Types type) {
  return parser->current.type == type;
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

  error(parser, parser->previous, message);
}

static bool match(Parser* parser, Types type) {
  if (check(parser, type) == false) return false;
  advance(parser);
  return true;
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
  int constant = add_constant(get_current_chunk(parser->compiler), value);

  if (constant > UINT8_MAX) {
    error(parser, parser->previous, compile_time[TOO_MANY_CONSTANTS]);
    return 0;
  }

  return (uint8_t)constant;
}

static void constant(Parser* parser, Value value) {
  uint8_t constant = make(parser, value);

  emit(parser, OP_CONSTANT);
  emit(parser, constant);
}

static void number(Parser* parser, bool assign) {
  char string[parser->previous.length + 1];

  strncpy(string, parser->previous.start, parser->previous.length + 1);

  string[parser->previous.length] = '\0';

  Value value;

  value.type = VALUE_NUMBER;

  mpf_init_set_str(value.content.number, string, 10);

  constant(parser, value);
}

static void string(Parser* parser, bool assign) {
  String* string = copy_string(parser->vm, parser->previous.start + 1, parser->previous.length - 2);

  Value value = OBJECT(string);

  constant(parser, value);
}

static void literal(Parser* parser, bool assign) {
  switch (parser->previous.type) {
    case TOKEN_TRUE: emit(parser, OP_TRUE); break;
    case TOKEN_FALSE: emit(parser, OP_FALSE); break;

    case TOKEN_VOID: emit(parser, OP_VOID); break;

    case TOKEN_UNDEFINED: emit(parser, OP_UNDEFINED); break;

    default: return;
  }
}

static void expression(Parser* parser) {
  parse(parser, PRECEDENCE_ASSIGNMENT);
}

static void grouping(Parser* parser, bool assign) {
  expression(parser);

  consume(parser, TOKEN_CLOSE_PARENTHESES, "Expect a close parentheses after expression.");
}

static void unary(Parser* parser, bool assign) {
  Types operator = parser->previous.type;

  parse(parser, PRECEDENCE_UNARY);

  switch (operator) {
    case TOKEN_MINUS: emit(parser, OP_NEGATION); break;

    case TOKEN_NOT: emit(parser, OP_NOT); break;

    default: return;
  }
}

static void binary(Parser* parser, bool assign) {
  Types operator = parser->previous.type;

  Rule* rule = &rules[operator];

  Precedences precedence = (Precedences)(rule->precedence + 1);

  parse(parser, precedence);

  switch (operator) {
    case TOKEN_PLUS: emit(parser, OP_ADD); break;
    case TOKEN_MINUS: emit(parser, OP_SUBTRACT); break;
    case TOKEN_ASTERISK: emit(parser, OP_MULTIPLY); break;
    case TOKEN_SLASH: emit(parser, OP_DIVIDE); break;

    case TOKEN_CARET: emit(parser, OP_POWER); break;

    case TOKEN_EQUAL: emit(parser, OP_EQUAL); break;
    case TOKEN_NOT_EQUAL: stream(parser, 2, OP_EQUAL, OP_NOT); break;

    case TOKEN_GREATER: emit(parser, OP_GREATER); break;
    case TOKEN_GREATER_EQUAL: stream(parser, 2, OP_LESS, OP_NOT); break;
    case TOKEN_LESS: emit(parser, OP_LESS); break;
    case TOKEN_LESS_EQUAL: stream(parser, 2, OP_GREATER, OP_NOT); break;

    default: return;
  }
}

static void synchronize(Parser* parser) {
  parser->panic = false;

  while (parser->current.type != TOKEN_EOF) {
    if (parser->previous.type == TOKEN_SEMICOLON) return;

    switch (parser->current.type) {
      case TOKEN_DEFINE:
      case TOKEN_SET:
      case TOKEN_FOR:
      case TOKEN_IF:
      case TOKEN_WHILE:
      case TOKEN_PRINT:
      case TOKEN_RETURN:
        return;
    }

    advance(parser);
  }
}

static bool identifiers_equality(Token* left, Token* right) {
  if (left->length != right->length) return false;
  return memcmp(left->start, right->start, left->length) == 0;
}

static uint8_t identifier(Parser* parser, Token* token) {
  String* string = copy_string(parser->vm, token->start, token->length);
  Value value = OBJECT(string);

  return make(parser, value);
}

static void local(Parser* parser, Token identifier) {
  if (parser->compiler->count == UINT8_MAX + 1) {
    error(parser, parser->previous, compile_time[TOO_MANY_LOCALS]);
    return;
  }

  Local* local = &parser->compiler->locals[parser->compiler->count++];
  local->identifier = identifier;
  local->depth = INITIALIZE;
}

static int resolve(Parser* parser, Token* identifier) {
  for (int i = parser->compiler->count - 1; i >= 0; i--) {
    Local* local = &parser->compiler->locals[i];
    
    if (identifiers_equality(identifier, &local->identifier)) {
      if (local->depth == INITIALIZE)
        error(parser, parser->previous, compile_time[CANNOT_READ_INITIALIZER]);

      return i;
    }
  }

  return INITIALIZE;
}

static void mark(Parser* parser) {
  if (parser->compiler->scope == 0) return;

  parser->compiler->locals[parser->compiler->count - 1].depth = parser->compiler->scope;
}

static void initialize(Parser* parser, uint8_t global) {
  if (parser->compiler->scope == 0) {
    emit(parser, OP_GLOBAL_INITIALIZE);
    emit(parser, global);
  }
  else mark(parser);
}

static void declaration(Parser* parser) {
  if (parser->compiler->scope == 0) return;

  Token* identifier = &parser->previous;

  for (int i = parser->compiler->count - 1; i >= 0; i--) {
    Local* local = &parser->compiler->locals[i];

    if (local->depth != INITIALIZE && local->depth < parser->compiler->scope) break;

    if (identifiers_equality(identifier, &local->identifier))
      error(parser, parser->previous, compile_time[VARIABLE_ALREADY_DECLARE]);
  }

  local(parser, *identifier);
}

static uint8_t definition(Parser* parser, const char* error) {
  consume(parser, TOKEN_IDENTIFIER, error);

  declaration(parser);

  if (parser->compiler->scope > 0) return 0;

  return identifier(parser, &parser->previous);
}

static void begin(Parser* parser) {
  parser->compiler->scope++;
}

static void end(Parser* parser) {
  parser->compiler->scope--;

  Compiler* compiler = parser->compiler;

  int count = compiler->count;

  while (count > 0 && compiler->locals[count - 1].depth > compiler->scope)
    count--;

  emit(parser, OP_POP_N);
  emit(parser, compiler->count - count);

  compiler->count = count;
}

static void variable(Parser* parser, bool assign) {
  uint8_t GET, ASSIGN;

  int argument = resolve(parser, &parser->previous);

  if (argument >= 0) {
    GET = OP_LOCAL_GET;
    ASSIGN = OP_LOCAL_SET;
  } else {
    argument = identifier(parser, &parser->previous);

    GET = OP_GLOBAL_GET;
    ASSIGN = OP_GLOBAL_SET;
  }

  uint8_t operation;

  if (assign && match(parser, TOKEN_ASSIGN)) {
    expression(parser);
    operation = ASSIGN;
  }
  else if (assign && match(parser, TOKEN_INCREMENT)) {
    emit(parser, GET);
    emit(parser, argument);

    constant(parser, MPF_NEUTRAL);

    emit(parser, OP_ADD);
    operation = ASSIGN;
  }
  else if (assign && match(parser, TOKEN_DECREMENT)) {
    emit(parser, GET);
    emit(parser, argument);

    constant(parser, MPF_NEUTRAL);

    emit(parser, OP_SUBTRACT);
    operation = ASSIGN;
  }
  else operation = GET;

  emit(parser, operation);
  emit(parser, argument);
}

static void function(Parser* parser, Positions position) {
  Compiler compiler;

  set_compiler(parser, &compiler, position);

  begin(parser);

  if (match(parser, TOKEN_COLON) == false) {
    consume(parser, TOKEN_OPEN_PARENTHESES, compile_time[EXPECT_OPEN_FUNCTION]);

    if (check(parser, TOKEN_CLOSE_PARENTHESES) == false) {
      do {
        parser->compiler->function->arity++;

        if (parser->compiler->function->arity > 255)
          error(parser, parser->current, compile_time[MAXIMUM_PARAMETERS]);

        uint8_t parameter = definition(parser, compile_time[EXPECT_PARAMETER_IDENTIFIER]);
        
        initialize(parser, parameter);
      } while (match(parser, TOKEN_COMMA) == true);
    }

    consume(parser, TOKEN_CLOSE_PARENTHESES, compile_time[EXPECT_CLOSE_FUNCTION]);
  }

  statement(parser);

  Function* function = terminate_compiler(parser);

  emit(parser, OP_CONSTANT);
  emit(parser, make(parser, OBJECT(function)));
}

static int jump(Parser* parser, uint8_t instruction) {
  emit(parser, instruction);

  emit(parser, 0xff);
  emit(parser, 0xff);

  return get_current_chunk(parser->compiler)->count - 2;
}

static void patch(Parser* parser, int offset) {
  int jump = get_current_chunk(parser->compiler)->count - offset - 2;

  if (jump > UINT16_MAX)
    error(parser, parser->previous, compile_time[MAXIMUM_JUMP_BODY]);

  get_current_chunk(parser->compiler)->code[offset] = (jump >> 8) & 0xff;
  get_current_chunk(parser->compiler)->code[offset + 1] = jump & 0xff;
}

static void loop(Parser* parser, int start) {
  emit(parser, OP_LOOP);

  int offset = get_current_chunk(parser->compiler)->count - start + 2;

  if (offset > UINT16_MAX) 
    error(parser, parser->previous, compile_time[MINIMUM_LOOP_BODY]);

  emit(parser, (offset >> 8) & 0xff);
  emit(parser, offset & 0xff);
}

static void and(Parser* parser, bool assign) {
  int end = jump(parser, OP_JUMP_CONDITIONAL);

  emit(parser, OP_POP);

  parse(parser, PRECEDENCE_AND);

  patch(parser, end);
}

static void or(Parser* parser, bool assign) {
  int otherwise = jump(parser, OP_JUMP_CONDITIONAL);

  int end = jump(parser, OP_JUMP);

  patch(parser, otherwise);

  emit(parser, OP_POP);

  parse(parser, PRECEDENCE_OR);

  patch(parser, end);
}

static void set(Parser* parser) {
  do {
    uint8_t global = definition(parser, compile_time[EXPECT_VARIABLE_IDENTIFIER]);

    if (match(parser, TOKEN_COLON))
      expression(parser);
    else emit(parser, OP_UNDEFINED);

    initialize(parser, global);
  } while(match(parser, TOKEN_COMMA));

  consume(parser, TOKEN_SEMICOLON, compile_time[EXPECT_SEMICOLON]);
}

static void define(Parser* parser) {
  uint8_t global = definition(parser, compile_time[EXPECT_FUNCTION_IDENTIFIER]);
  mark(parser);
  function(parser, POSITION_FUNCTION);
  initialize(parser, global);
}

static void block(Parser* parser) {
  while (check(parser, TOKEN_CLOSE_BRACES) == false && check(parser, TOKEN_EOF) == false) 
    instruction(parser);

  consume(parser, TOKEN_CLOSE_BRACES, compile_time[EXPECT_BLOCK]);
}

static uint8_t arguments(Parser* parser) {
  uint8_t count = 0;

  if (check(parser, TOKEN_CLOSE_PARENTHESES) == false) {
    do {
      expression(parser);

      if (count == 255)
        error(parser, parser->previous, compile_time[MAXIMUM_ARGUMENTS]);

      count++;
    } while (match(parser, TOKEN_COMMA) == true);
  }

  consume(parser, TOKEN_CLOSE_PARENTHESES, compile_time[EXPECT_ARGUMENTS]);

  return count;
}

static void call(Parser* parser, bool assign) {
  uint8_t count = arguments(parser);

  emit(parser, OP_CALL);
  emit(parser, count);
}

static void conditional(Parser* parser) {
  expression(parser);

  consume(parser, TOKEN_COLON, compile_time[EXPECT_COLON_CONDITION]);

  int then = jump(parser, OP_JUMP_CONDITIONAL);

  emit(parser, OP_POP);

  statement(parser);

  int otherwise = jump(parser, OP_JUMP);

  patch(parser, then);

  emit(parser, OP_POP);

  if (match(parser, TOKEN_ELSE)) {
    consume(parser, TOKEN_COLON, compile_time[EXPECT_COLON_STATEMENT]);
    
    statement(parser);
  }

  patch(parser, otherwise);
}

static void iterative(Parser* parser) {
  int start = get_current_chunk(parser->compiler)->count;

  expression(parser);

  consume(parser, TOKEN_COLON, compile_time[EXPECT_COLON_CONDITION]);

  int exit = jump(parser, OP_JUMP_CONDITIONAL);

  emit(parser, OP_POP);

  statement(parser);

  loop(parser, start);

  patch(parser, exit);

  emit(parser, OP_POP);
}

static void repeat(Parser* parser) {
  int start = get_current_chunk(parser->compiler)->count;

  statement(parser);

  consume(parser, TOKEN_WHILE, compile_time[EXPECT_WHILE_STATEMENT]);

  expression(parser);

  int exit = jump(parser, OP_JUMP_CONDITIONAL);

  emit(parser, OP_POP);

  loop(parser, start);

  patch(parser, exit);

  emit(parser, OP_POP);
}

static void looping(Parser* parser) {
  begin(parser);

  consume(parser, TOKEN_OPEN_PARENTHESES, compile_time[EXPECT_OPEN_FOR]);

  if (match(parser, TOKEN_SEMICOLON) == false) {
    if (match(parser, TOKEN_SET) == true) set(parser);
    else {
      expression(parser);
      consume(parser, TOKEN_SEMICOLON, compile_time[EXPECT_SEMICOLON]);
    }
  }

  int start = get_current_chunk(parser->compiler)->count;

  int exit = -1;

  if (match(parser, TOKEN_SEMICOLON) == false) {
    expression(parser);

    consume(parser, TOKEN_SEMICOLON, compile_time[EXPECT_SEMICOLON]);

    exit = jump(parser, OP_JUMP_CONDITIONAL);

    emit(parser, OP_POP);
  }

  if (match(parser, TOKEN_CLOSE_PARENTHESES) == false) {
    int body = jump(parser, OP_JUMP);

    int increment = get_current_chunk(parser->compiler)->count;

    expression(parser);

    emit(parser, OP_POP);

    consume(parser, TOKEN_CLOSE_PARENTHESES, compile_time[EXPECT_CLOSE_FOR]);

    loop(parser, start);

    start = increment;

    patch(parser, body);
  }

  statement(parser);

  loop(parser, start);

  if (exit >= 0) {
    patch(parser, exit);
    emit(parser, OP_POP);
  }

  end(parser);
}

static void statement(Parser* parser) {
  if (match(parser, TOKEN_SEMICOLON) == true) return;

  if (match(parser, TOKEN_OPEN_BRACES)) {
    begin(parser);
    block(parser);
    end(parser);
  } 
  else if (match(parser, TOKEN_PRINT)) {
    expression(parser);
    emit(parser, OP_PRINT);
    consume(parser, TOKEN_SEMICOLON, compile_time[EXPECT_SEMICOLON]);
  } 
  else if (match(parser, TOKEN_IF)) 
    conditional(parser);
  else if (match(parser, TOKEN_WHILE))
    iterative(parser);
  else if (match(parser, TOKEN_DO))
    repeat(parser);
  else if (match(parser, TOKEN_FOR))
    looping(parser);
  else if (match(parser, TOKEN_RETURN)) {
    if (parser->compiler->position == POSITION_SCRIPT)
      error(parser, parser->previous, compile_time[CANNOT_RETURN_SCRIPT]);

    if (match(parser, TOKEN_SEMICOLON) == false) {
      expression(parser);
      consume(parser, TOKEN_SEMICOLON, compile_time[EXPECT_SEMICOLON]);
      emit(parser, OP_RETURN);
    } else close(parser);
  }
  else if (match(parser, TOKEN_EMPTY)) {
    emit(parser, OP_EMPTY);
    consume(parser, TOKEN_SEMICOLON, compile_time[EXPECT_SEMICOLON]);
  }
  else if (match(parser, TOKEN_EXIT)) {
    emit(parser, OP_EXIT);
    consume(parser, TOKEN_SEMICOLON, compile_time[EXPECT_SEMICOLON]);
  }
  else {
    expression(parser);
    emit(parser, OP_POP);
    consume(parser, TOKEN_SEMICOLON, compile_time[EXPECT_SEMICOLON]);
  }
}

static void instruction(Parser* parser) {
  if (match(parser, TOKEN_SET))
    set(parser);
  else if (match(parser, TOKEN_DEFINE))
    define(parser);
  else statement(parser);

  if (parser->panic) 
    synchronize(parser);
}

static void parse(Parser* parser, Precedences precedence) {
  advance(parser);

  Execute prefix = rules[parser->previous.type].prefix;

  if (prefix == NULL) {
    error(parser, parser->previous, compile_time[EXPECT_EXPRESSION]);
    return;
  }

  bool assign = precedence <= PRECEDENCE_ASSIGNMENT;

  prefix(parser, assign);

  while (precedence <= rules[parser->current.type].precedence) {
    advance(parser);
    Execute infix = rules[parser->previous.type].infix;
    infix(parser, assign);
  }

  if (assign && match(parser, TOKEN_EQUAL))
    error(parser, parser->previous, compile_time[INVALID_ASSIGNMENT_TARGET]);
}

Function* compile(VM* vm, const char* source) {
  Parser parser; 

  parser.vm = vm;

  parser.error = false; parser.panic = false;

  set_tokenizer(&parser.tokenizer, source);

  Compiler compiler;

  set_compiler(&parser, &compiler, POSITION_SCRIPT);

  advance(&parser);
  
  while (match(&parser, TOKEN_EOF) == false)
    instruction(&parser);

  Function* function = terminate_compiler(&parser);

  return parser.error ? NULL : function;
}