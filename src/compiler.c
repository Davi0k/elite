#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "tokenizer.h"
#include "types/object.h"

#define GET_CURRENT_CHUNK(compiler) \
  ( &compiler->function->chunk )

#define EMIT_BYTE(parser, byte) \
  do { \
    Chunk* chunk = GET_CURRENT_CHUNK(parser->compiler); \
    int line = parser->previous.line; \
    write_chunk(chunk, byte, line); \
  } while(false)

static void stream(Parser* parser, int count, ...) {
  va_list list;

  va_start(list, count);

  for (int i = 0; i < count; i++) {
    uint8_t byte = (uint8_t)va_arg(list, int);

    EMIT_BYTE(parser, byte);
  }

  va_end(list);
}

static void set_compiler(Parser* parser, Compiler* compiler, Positions position) {
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
  local->captured = false;
  
  local->identifier.start = position == POSITION_FUNCTION ? NULL_TERMINATOR : "this";
  
  local->identifier.length = position == POSITION_FUNCTION ? 0 : 4;
}

static void grouping(Parser* parser, bool assign);
static void unary(Parser* parser, bool assign);
static void binary(Parser* parser, bool assign);

static void number(Parser* parser, bool assign);
static void string(Parser* parser, bool assign);
static void literal(Parser* parser, bool assign);

static void and(Parser* parser, bool assign);
static void or(Parser* parser, bool assign);

static void ternary(Parser* parser, bool assign);

static void variable(Parser* parser, bool assign);
static void function(Parser* parser, bool assign);

static void call(Parser* parser, bool assign);

static void accessor(Parser* parser, bool assign);

static void this(Parser* parser, bool assign);
static void super(Parser* parser, bool assign);

static void expression(Parser* parser);
static void block(Parser* parser);
static void statement(Parser* parser);
static void instruction(Parser* parser);

static void parse(Parser* parser, Precedences precedence);

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
  [ TOKEN_QUESTION ] = { NULL, ternary, PRECEDENCE_PRIMARY },
  [ TOKEN_COLON ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_DOT ] = { NULL, accessor,  PRECEDENCE_CALL },

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

  [ TOKEN_GLOBAL ] = { NULL, NULL, PRECEDENCE_NONE },
  
  [ TOKEN_IF ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_ELSE ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_WHILE ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_DO ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_FOR ] = { NULL, NULL, PRECEDENCE_NONE },

  [ TOKEN_DEFINE ] = { function, NULL, PRECEDENCE_NONE },
  [ TOKEN_RETURN ] = { NULL, NULL, PRECEDENCE_NONE },

  [ TOKEN_CLASS ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_THIS ] = { this, NULL, PRECEDENCE_NONE },
  [ TOKEN_STATIC ] = { NULL, NULL, PRECEDENCE_NONE },
  [ TOKEN_SUPER ] = { super, NULL, PRECEDENCE_NONE },

  [ TOKEN_SEMICOLON ] = { NULL, NULL, PRECEDENCE_NONE}
};

static Function* terminate(Parser* parser) {
  if (parser->compiler->position == POSITION_CONSTRUCTOR) {
    EMIT_BYTE(parser, OP_LOCAL_GET);
    EMIT_BYTE(parser, 0);
  }
  else EMIT_BYTE(parser, OP_UNDEFINED);

  EMIT_BYTE(parser, OP_RETURN);

  Function* function = parser->compiler->function;

  parser->compiler = parser->compiler->enclosing;

  return function;
}

static void error(Parser* parser, Token token, const char* error) {
  if (parser->panic) return;

  parser->panic = true;

  fprintf(stderr, "[Line N°%d] Error", token.line);

  fprintf(stderr, " at <%.*s>", token.length, token.start);

  fprintf(stderr, " -> %s\n", error);

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

static bool check(Parser* parser, Types type) {
  return parser->current.type == type;
}

static bool match(Parser* parser, Types type) {
  if (check(parser, type) == false) return false;
  advance(parser);
  return true;
}

static void consume(Parser* parser, Types type, const char* message) {
  if (parser->current.type == type) 
    return advance(parser);

  error(parser, parser->previous, message);
}

static Token synthetic(const char* text) {
  Token token;

  token.start = text;
  token.length = (int)strlen(text);

  return token;
}

static uint8_t make(Parser* parser, Value value) {
  int constant = add_constant(GET_CURRENT_CHUNK(parser->compiler), value);

  if (constant > UINT8_MAX) {
    error(parser, parser->previous, compile_time_errors[TOO_MANY_CONSTANTS]);
    return 0;
  }

  return (uint8_t)constant;
}

static void constant(Parser* parser, Value value) {
  uint8_t constant = make(parser, value);

  EMIT_BYTE(parser, OP_CONSTANT);
  EMIT_BYTE(parser, constant);
}

static bool identifiers_equality(Token* left, Token* right) {
  if (left->length != right->length) return false;
  return memcmp(left->start, right->start, left->length) == 0;
}

static uint8_t identify(Parser* parser, Token* token) {
  String* string = copy_string(parser->vm, token->start, token->length);
  Value value = OBJECT(string);

  return make(parser, value);
}

static void local(Parser* parser, Token identifier) {
  if (parser->compiler->count == MAXIMUM_LIMIT) {
    error(parser, parser->previous, compile_time_errors[TOO_MANY_LOCALS]);
    return;
  }

  Local* local = &parser->compiler->locals[parser->compiler->count++];
  local->identifier = identifier;
  local->depth = INITIALIZE;
  local->captured = false;
}

static int resolve(Parser* parser, Compiler* compiler, Token* identifier) {
  for (int i = compiler->count - 1; i >= 0; i--) {
    Local* local = &compiler->locals[i];
    
    if (identifiers_equality(identifier, &local->identifier)) {
      if (local->depth == INITIALIZE)
        error(parser, parser->previous, compile_time_errors[CANNOT_READ_INITIALIZER]);

      return i;
    }
  }

  return INITIALIZE;
}

static int up(Parser* parser, Compiler* compiler, uint8_t index, bool local) {
  int count = compiler->function->count;

  for (int i = 0; i < count; i++) {
    Up* up = &compiler->ups[i];

    if (up->index == index && up->local == local)
      return i;
  }

  if (count == MAXIMUM_LIMIT) {
    error(parser, parser->previous, compile_time_errors[TOO_MANY_CLOSURE_VARIABLES]);
    return 0;
  }

  compiler->ups[count].local = local;
  compiler->ups[count].index = index;

  return compiler->function->count++;
}

static int upvalue(Parser* parser, Compiler* compiler, Token* identifier) {
  if (compiler->enclosing == NULL) return INITIALIZE;

  int local = resolve(parser, compiler->enclosing, identifier);

  if (local != INITIALIZE) {
    compiler->enclosing->locals[local].captured = true;

    return up(parser, compiler, (uint8_t)local, true);
  }

  int over = upvalue(parser, compiler->enclosing, identifier);

  if (over != INITIALIZE)
    return up(parser, compiler, (uint8_t)over, false);

  return INITIALIZE;
}

static void mark(Parser* parser, bool force) {
  if (force == true || parser->compiler->scope == 0) return;

  parser->compiler->locals[parser->compiler->count - 1].depth = parser->compiler->scope;
}

static void initialize(Parser* parser, uint8_t global, bool force) {
  if (parser->compiler->scope == 0 || force == true) {
    EMIT_BYTE(parser, OP_GLOBAL_INITIALIZE);
    EMIT_BYTE(parser, global);
  }
  else mark(parser, force);
}

static void declaration(Parser* parser, bool force) {
  if (parser->compiler->scope == 0 || force == true) return;

  Token* identifier = &parser->previous;

  for (int i = parser->compiler->count - 1; i >= 0; i--) {
    Local* local = &parser->compiler->locals[i];

    if (local->depth != INITIALIZE && local->depth < parser->compiler->scope) break;

    if (identifiers_equality(identifier, &local->identifier))
      error(parser, parser->previous, compile_time_errors[VARIABLE_ALREADY_DECLARE]);
  }

  local(parser, *identifier);
}

static uint8_t definition(Parser* parser, const char* error, bool force) {
  consume(parser, TOKEN_IDENTIFIER, error);

  declaration(parser, force);

  if (parser->compiler->scope == 0 || force == true) 
    return identify(parser, &parser->previous);
  else return 0;
}

static void close_scope(Parser* parser) {
  parser->compiler->scope--;

  Compiler* compiler = parser->compiler;

  int counter = 0;

  while (compiler->count > 0 && compiler->locals[compiler->count - 1].depth > compiler->scope) {
    if (compiler->locals[compiler->count - 1].captured) {
      EMIT_BYTE(parser, OP_POP_N);
      EMIT_BYTE(parser, counter);

      EMIT_BYTE(parser, OP_CLOSE);

      counter = 0;
    }
    else counter++;

    compiler->count--;
  }

  EMIT_BYTE(parser, OP_POP_N);
  EMIT_BYTE(parser, counter);
}

static uint8_t arguments(Parser* parser) {
  uint8_t count = 0;

  if (check(parser, TOKEN_CLOSE_PARENTHESES) == false) {
    do {
      expression(parser);

      if (count == 255)
        error(parser, parser->previous, compile_time_errors[MAXIMUM_ARGUMENTS]);

      count++;
    } while (match(parser, TOKEN_COMMA) == true);
  }

  consume(parser, TOKEN_CLOSE_PARENTHESES, compile_time_errors[EXPECT_ARGUMENTS]);

  return count;
}

static void emit_variable(Parser* parser, Token identifier, bool assign) {
  uint8_t setter = 0, getter = 0;

  int argument = resolve(parser, parser->compiler, &identifier);

  if (argument >= 0) {
    setter = OP_LOCAL_SET;
    getter = OP_LOCAL_GET;
  } else {
    argument = upvalue(parser, parser->compiler, &identifier);

    if(argument != INITIALIZE) {
      setter = OP_UP_SET;
      getter = OP_UP_GET;
    } else {
      argument = identify(parser, &identifier);

      setter = OP_GLOBAL_SET;
      getter = OP_GLOBAL_GET;
    }
  }

  if (assign && match(parser, TOKEN_ASSIGN)) {
    expression(parser);
    EMIT_BYTE(parser, setter);
  }
  else if (assign && match(parser, TOKEN_INCREMENT)) {
    EMIT_BYTE(parser, getter);
    EMIT_BYTE(parser, argument);

    constant(parser, OBJECT(allocate_number_from_double(parser->vm, 1.0)));

    EMIT_BYTE(parser, OP_ADD);
    EMIT_BYTE(parser, setter);
  }
  else if (assign && match(parser, TOKEN_DECREMENT)) {
    EMIT_BYTE(parser, getter);
    EMIT_BYTE(parser, argument);

    constant(parser, OBJECT(allocate_number_from_double(parser->vm, 1.0)));

    EMIT_BYTE(parser, OP_SUBTRACT);
    EMIT_BYTE(parser, setter);
  }
  else EMIT_BYTE(parser, getter);

  EMIT_BYTE(parser, argument);
}

static void emit_function(Parser* parser, Positions position, bool assign) {
  Compiler compiler;

  set_compiler(parser, &compiler, position);

  parser->compiler->scope++;;

  if (match(parser, TOKEN_COLON) == false) {
    consume(parser, TOKEN_OPEN_PARENTHESES, compile_time_errors[EXPECT_OPEN_FUNCTION]);

    if (check(parser, TOKEN_CLOSE_PARENTHESES) == false) {
      do {
        parser->compiler->function->arity++;

        if (parser->compiler->function->arity > 255)
          error(parser, parser->current, compile_time_errors[MAXIMUM_PARAMETERS]);

        uint8_t parameter = definition(parser, compile_time_errors[EXPECT_PARAMETER_IDENTIFIER], false);
        
        initialize(parser, parameter, false);
      } while (match(parser, TOKEN_COMMA) == true);
    }

    consume(parser, TOKEN_CLOSE_PARENTHESES, compile_time_errors[EXPECT_CLOSE_FUNCTION]);
  }

  if (match(parser, TOKEN_OPEN_BRACES) == true)
    block(parser);
  else {
    expression(parser);
    EMIT_BYTE(parser, OP_RETURN);
  }

  Function* function = terminate(parser);

  if (assign == true)
    function->identifier = NULL;

  stream(parser, 2, OP_CLOSURE, make(parser, OBJECT(function)));

  for (int i = 0; i < function->count; i++) {
    EMIT_BYTE(parser, compiler.ups[i].local ? 1 : 0);
    EMIT_BYTE(parser, compiler.ups[i].index);
  }
}

static void patch(Parser* parser, int offset) {
  int jump = GET_CURRENT_CHUNK(parser->compiler)->count - offset - 2;

  if (jump > UINT16_MAX)
    error(parser, parser->previous, compile_time_errors[MAXIMUM_JUMP_BODY]);

  GET_CURRENT_CHUNK(parser->compiler)->code[offset] = (jump >> 8) & 0xff;
  GET_CURRENT_CHUNK(parser->compiler)->code[offset + 1] = jump & 0xff;
}

static int jump(Parser* parser, uint8_t instruction) {
  EMIT_BYTE(parser, instruction);

  EMIT_BYTE(parser, 0xff);
  EMIT_BYTE(parser, 0xff);

  return GET_CURRENT_CHUNK(parser->compiler)->count - 2;
}

static void loop(Parser* parser, int start) {
  EMIT_BYTE(parser, OP_LOOP);

  int offset = GET_CURRENT_CHUNK(parser->compiler)->count - start + 2;

  if (offset > UINT16_MAX) 
    error(parser, parser->previous, compile_time_errors[MINIMUM_CAPACITY_LOOP_BODY]);

  EMIT_BYTE(parser, (offset >> 8) & 0xff);
  EMIT_BYTE(parser, offset & 0xff);
}

static void grouping(Parser* parser, bool assign) {
  expression(parser);

  consume(parser, TOKEN_CLOSE_PARENTHESES, "Expect a close parentheses after expression.");
}

static void unary(Parser* parser, bool assign) {
  Types operator = parser->previous.type;

  parse(parser, PRECEDENCE_UNARY);

  switch (operator) {
    case TOKEN_MINUS: EMIT_BYTE(parser, OP_NEGATION); break;

    case TOKEN_NOT: EMIT_BYTE(parser, OP_NOT); break;

    default: return;
  }
}

static void binary(Parser* parser, bool assign) {
  Types operator = parser->previous.type;

  Rule* rule = &rules[operator];

  Precedences precedence = (Precedences)(rule->precedence + 1);

  parse(parser, precedence);

  switch (operator) {
    case TOKEN_PLUS: EMIT_BYTE(parser, OP_ADD); break;
    case TOKEN_MINUS: EMIT_BYTE(parser, OP_SUBTRACT); break;
    case TOKEN_ASTERISK: EMIT_BYTE(parser, OP_MULTIPLY); break;
    case TOKEN_SLASH: EMIT_BYTE(parser, OP_DIVIDE); break;

    case TOKEN_CARET: EMIT_BYTE(parser, OP_POWER); break;

    case TOKEN_EQUAL: EMIT_BYTE(parser, OP_EQUAL); break;
    case TOKEN_NOT_EQUAL: stream(parser, 2, OP_EQUAL, OP_NOT); break;

    case TOKEN_GREATER: EMIT_BYTE(parser, OP_GREATER); break;
    case TOKEN_GREATER_EQUAL: stream(parser, 2, OP_LESS, OP_NOT); break;
    case TOKEN_LESS: EMIT_BYTE(parser, OP_LESS); break;
    case TOKEN_LESS_EQUAL: stream(parser, 2, OP_GREATER, OP_NOT); break;

    default: return;
  }
}

static void number(Parser* parser, bool assign) {
  char string[parser->previous.length + 1];

  strncpy(string, parser->previous.start, parser->previous.length + 1);

  string[parser->previous.length] = '\0';

  Number* number = allocate_number_from_string(parser->vm, string);

  Value value = OBJECT(number);

  constant(parser, value);
}

static void string(Parser* parser, bool assign) {
  String* string = copy_string(parser->vm, parser->previous.start + 1, parser->previous.length - 2);

  Value value = OBJECT(string);

  constant(parser, value);
}

static void literal(Parser* parser, bool assign) {
  switch (parser->previous.type) {
    case TOKEN_TRUE: EMIT_BYTE(parser, OP_TRUE); break;
    case TOKEN_FALSE: EMIT_BYTE(parser, OP_FALSE); break;

    case TOKEN_VOID: EMIT_BYTE(parser, OP_VOID); break;

    case TOKEN_UNDEFINED: EMIT_BYTE(parser, OP_UNDEFINED); break;

    default: return;
  }
}

static void and(Parser* parser, bool assign) {
  int end = jump(parser, OP_JUMP_CONDITIONAL);

  EMIT_BYTE(parser, OP_POP);

  parse(parser, PRECEDENCE_AND);

  patch(parser, end);
}

static void or(Parser* parser, bool assign) {
  int otherwise = jump(parser, OP_JUMP_CONDITIONAL);

  int end = jump(parser, OP_JUMP);

  patch(parser, otherwise);

  EMIT_BYTE(parser, OP_POP);

  parse(parser, PRECEDENCE_OR);

  patch(parser, end);
}

static void ternary(Parser* parser, bool assign) {
  int then = jump(parser, OP_JUMP_CONDITIONAL);

  EMIT_BYTE(parser, OP_POP);

  expression(parser);

  int otherwise = jump(parser, OP_JUMP);

  patch(parser, then);

  EMIT_BYTE(parser, OP_POP);

  consume(parser, TOKEN_COLON, compile_time_errors[EXPECT_COLON_TERNARY]);

  expression(parser);

  patch(parser, otherwise);
}

static void variable(Parser* parser, bool assign) {
  emit_variable(parser, parser->previous, assign);
}

static void function(Parser* parser, bool assign) {
  emit_function(parser, POSITION_FUNCTION, assign);
};

static void call(Parser* parser, bool assign) {
  uint8_t count = arguments(parser);

  EMIT_BYTE(parser, OP_CALL);
  EMIT_BYTE(parser, count);
}

static void accessor(Parser* parser, bool assign) {
  consume(parser, TOKEN_IDENTIFIER, compile_time_errors[EXPECT_PROPERTY_IDENTIFIER]);

  uint8_t property = identify(parser, &parser->previous);

  if (assign && match(parser, TOKEN_ASSIGN)) {
    expression(parser);
    EMIT_BYTE(parser, OP_PROPERTY_SET);
    EMIT_BYTE(parser, property);
  }
  else if (match(parser, TOKEN_OPEN_PARENTHESES) == true) {
    uint8_t count = arguments(parser);

    EMIT_BYTE(parser, OP_INVOKE);
    EMIT_BYTE(parser, property);

    EMIT_BYTE(parser, count);
  }
  else {
    EMIT_BYTE(parser, OP_PROPERTY_GET);
    EMIT_BYTE(parser, property);
  }
}

static void this(Parser* parser, bool assign) {
  if (parser->entity == NULL)
    error(parser, parser->previous, compile_time_errors[CANNOT_USE_THIS]);
  else emit_variable(parser, parser->previous, false);
}

static void super(Parser* parser, bool assign) {
  if (parser->entity == NULL)
    error(parser, parser->previous, compile_time_errors[CANNOT_USE_SUPER]);
  else if (parser->entity->inheritance == false)
    error(parser, parser->previous, compile_time_errors[MUST_HAVE_SUPERCLASS]);

  consume(parser, TOKEN_DOT, compile_time_errors[EXPECT_DOT_AFTER_SUPER]);
  consume(parser, TOKEN_IDENTIFIER, compile_time_errors[EXPECT_SUPERCLASS_PROPERTY]);
  
  uint8_t identifier = identify(parser, &parser->previous);

  emit_variable(parser, synthetic("this"), false);
  emit_variable(parser, synthetic("super"), false);

  EMIT_BYTE(parser, OP_SUPER);
  EMIT_BYTE(parser, identifier);
}

static void set(Parser* parser, bool force) {
  do {
    uint8_t global = definition(parser, compile_time_errors[EXPECT_VARIABLE_IDENTIFIER], force);

    if (match(parser, TOKEN_COLON))
      expression(parser);
    else EMIT_BYTE(parser, OP_UNDEFINED);

    initialize(parser, global, force);
  } while(match(parser, TOKEN_COMMA) == true);

  consume(parser, TOKEN_SEMICOLON, compile_time_errors[EXPECT_SEMICOLON]);
}

static void define(Parser* parser, bool force) {
  do {
    uint8_t global = definition(parser, compile_time_errors[EXPECT_FUNCTION_IDENTIFIER], force);
    mark(parser, force);
    emit_function(parser, POSITION_FUNCTION, false);
    initialize(parser, global, force);
  } while(match(parser, TOKEN_COMMA) == true);

  consume(parser, TOKEN_SEMICOLON, compile_time_errors[EXPECT_SEMICOLON]);
}

static void class(Parser* parser, bool force) {
  consume(parser, TOKEN_IDENTIFIER, compile_time_errors[EXPECT_CLASS_IDENTIFIER]);

  Token class = parser->previous;

  uint8_t constant = identify(parser, &parser->previous);

  declaration(parser, force);

  EMIT_BYTE(parser, OP_CLASS);
  EMIT_BYTE(parser, constant);
  
  initialize(parser, constant, force);

  Entity entity;

  entity.enclosing = parser->entity;
  entity.identifier = parser->previous;
  entity.inheritance = check(parser, TOKEN_COLON);

  parser->entity = &entity;

  if (match(parser, TOKEN_COLON) == true) {
    consume(parser, TOKEN_IDENTIFIER, compile_time_errors[EXPECT_SUPER_IDENTIFIER]);

    emit_variable(parser, parser->previous, false);

    if (identifiers_equality(&parser->previous, &class) == true)
      error(parser, parser->previous, compile_time_errors[CANNOT_INHERIT_SELF]);

    Token token = synthetic("super");

    parser->compiler->scope++;
    local(parser, token);
    initialize(parser, 0, false);

    emit_variable(parser, class, false);

    EMIT_BYTE(parser, OP_INHERIT);
  }

  emit_variable(parser, class, false);

  if (match(parser, TOKEN_SEMICOLON) == false) {
    consume(parser, TOKEN_OPEN_BRACES, compile_time_errors[EXPECT_OPEN_CLASS]);

    while (check(parser, TOKEN_CLOSE_BRACES) == false) {
      if (check(parser, TOKEN_EOF) == true) break;

      if (check(parser, TOKEN_SET) == false && check(parser, TOKEN_DEFINE) == false) {
        error(parser, parser->current, compile_time_errors[CLASS_CAN_SET_DEFINE]);

        break;
      }

      if (match(parser, TOKEN_SET) == true) {
        do {
          consume(parser, TOKEN_IDENTIFIER, compile_time_errors[EXPECT_MEMBER_IDENTIFIER]);

          uint8_t constant = identify(parser, &parser->previous);

          if (match(parser, TOKEN_COLON) == true)
            expression(parser);
          else EMIT_BYTE(parser, OP_UNDEFINED);

          EMIT_BYTE(parser, OP_MEMBER);
          EMIT_BYTE(parser, constant);
        } while(match(parser, TOKEN_COMMA) == true);

        consume(parser, TOKEN_SEMICOLON, compile_time_errors[EXPECT_SEMICOLON]);
      }

      if (match(parser, TOKEN_DEFINE) == true) {
        do {
          Positions position;

          Token method = parser->current;

          if (memcmp(method.start, class.start, method.length > class.length ? method.length : class.length) == 0)
            position = POSITION_CONSTRUCTOR;
          else position = POSITION_METHOD;

          consume(parser, TOKEN_IDENTIFIER, compile_time_errors[EXPECT_METHOD_IDENTIFIER]);

          uint8_t constant = identify(parser, &parser->previous);

          emit_function(parser, position, false);

          EMIT_BYTE(parser, OP_METHOD);
          EMIT_BYTE(parser, constant);
        } while(match(parser, TOKEN_COMMA) == true);

        consume(parser, TOKEN_SEMICOLON, compile_time_errors[EXPECT_SEMICOLON]);
      }
    }

    consume(parser, TOKEN_CLOSE_BRACES, compile_time_errors[EXPECT_CLOSE_CLASS]);
  }

  EMIT_BYTE(parser, OP_POP);

  if (parser->entity->inheritance == true)
    close_scope(parser);

  parser->entity = parser->entity->enclosing;
}

static void conditional(Parser* parser) {
  expression(parser);

  consume(parser, TOKEN_COLON, compile_time_errors[EXPECT_COLON_CONDITION]);

  int then = jump(parser, OP_JUMP_CONDITIONAL);

  EMIT_BYTE(parser, OP_POP);

  statement(parser);

  int otherwise = jump(parser, OP_JUMP);

  patch(parser, then);

  EMIT_BYTE(parser, OP_POP);

  if (match(parser, TOKEN_ELSE)) {
    consume(parser, TOKEN_COLON, compile_time_errors[EXPECT_COLON_STATEMENT]);
    
    statement(parser);
  }

  patch(parser, otherwise);
}

static void iterative(Parser* parser) {
  int start = GET_CURRENT_CHUNK(parser->compiler)->count;

  expression(parser);

  consume(parser, TOKEN_COLON, compile_time_errors[EXPECT_COLON_CONDITION]);

  int exit = jump(parser, OP_JUMP_CONDITIONAL);

  EMIT_BYTE(parser, OP_POP);

  statement(parser);

  loop(parser, start);

  patch(parser, exit);

  EMIT_BYTE(parser, OP_POP);
}

static void repeat(Parser* parser) {
  int start = GET_CURRENT_CHUNK(parser->compiler)->count;

  statement(parser);

  consume(parser, TOKEN_WHILE, compile_time_errors[EXPECT_WHILE_STATEMENT]);

  expression(parser);

  int exit = jump(parser, OP_JUMP_CONDITIONAL);

  EMIT_BYTE(parser, OP_POP);

  loop(parser, start);

  patch(parser, exit);

  EMIT_BYTE(parser, OP_POP);
}

static void looping(Parser* parser) {
  parser->compiler->scope++;;

  consume(parser, TOKEN_OPEN_PARENTHESES, compile_time_errors[EXPECT_OPEN_FOR]);

  if (match(parser, TOKEN_SEMICOLON) == false) {
    if (match(parser, TOKEN_SET))
      set(parser, false);
    else {
      expression(parser);
      consume(parser, TOKEN_SEMICOLON, compile_time_errors[EXPECT_SEMICOLON]);
    }
  }

  int start = GET_CURRENT_CHUNK(parser->compiler)->count;

  int exit = -1;

  if (match(parser, TOKEN_SEMICOLON) == false) {
    expression(parser);

    consume(parser, TOKEN_SEMICOLON, compile_time_errors[EXPECT_SEMICOLON]);

    exit = jump(parser, OP_JUMP_CONDITIONAL);

    EMIT_BYTE(parser, OP_POP);
  }

  if (match(parser, TOKEN_CLOSE_PARENTHESES) == false) {
    int body = jump(parser, OP_JUMP);

    int increment = GET_CURRENT_CHUNK(parser->compiler)->count;

    expression(parser);

    EMIT_BYTE(parser, OP_POP);

    consume(parser, TOKEN_CLOSE_PARENTHESES, compile_time_errors[EXPECT_CLOSE_FOR]);

    loop(parser, start);

    start = increment;

    patch(parser, body);
  }

  statement(parser);

  loop(parser, start);

  if (exit >= 0) {
    patch(parser, exit);
    EMIT_BYTE(parser, OP_POP);
  }

  close_scope(parser);
}

static void synchronize(Parser* parser) {
  parser->panic = false;

  while (parser->current.type != TOKEN_EOF) {
    if (parser->previous.type == TOKEN_SEMICOLON) return;

    switch (parser->current.type) {
      case TOKEN_SET:
      case TOKEN_DEFINE:
      case TOKEN_CLASS:
      case TOKEN_FOR:
      case TOKEN_IF:
      case TOKEN_WHILE:
      case TOKEN_DO:
      case TOKEN_RETURN:
        return;
    }

    advance(parser);
  }
}

static void expression(Parser* parser) {
  parse(parser, PRECEDENCE_ASSIGNMENT);
}

static void block(Parser* parser) {
  while (check(parser, TOKEN_CLOSE_BRACES) == false && check(parser, TOKEN_EOF) == false) 
    instruction(parser);

  consume(parser, TOKEN_CLOSE_BRACES, compile_time_errors[EXPECT_BLOCK]);
}

static void statement(Parser* parser) {
  if (match(parser, TOKEN_SEMICOLON) == true) return;

  if (match(parser, TOKEN_OPEN_BRACES)) {
    parser->compiler->scope++;;
    block(parser);
    close_scope(parser);
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
      error(parser, parser->previous, compile_time_errors[CANNOT_RETURN_SCRIPT]);

    if (parser->compiler->position == POSITION_CONSTRUCTOR)
      error(parser, parser->previous, compile_time_errors[CANNOT_RETURN_CONSTRUCTOR]);

    if (check(parser, TOKEN_SEMICOLON) == false) 
      expression(parser);
    else EMIT_BYTE(parser, OP_UNDEFINED);

    consume(parser, TOKEN_SEMICOLON, compile_time_errors[EXPECT_SEMICOLON]);

    EMIT_BYTE(parser, OP_RETURN);
  }
  else if (match(parser, TOKEN_EMPTY)) {
    EMIT_BYTE(parser, OP_EMPTY);
    consume(parser, TOKEN_SEMICOLON, compile_time_errors[EXPECT_SEMICOLON]);
  }
  else if (match(parser, TOKEN_EXIT)) {
    EMIT_BYTE(parser, OP_EXIT);
    consume(parser, TOKEN_SEMICOLON, compile_time_errors[EXPECT_SEMICOLON]);
  }
  else {
    expression(parser);
    EMIT_BYTE(parser, OP_POP);
    consume(parser, TOKEN_SEMICOLON, compile_time_errors[EXPECT_SEMICOLON]);
  }
}

static void instruction(Parser* parser) {
  bool force = match(parser, TOKEN_GLOBAL);

  if (force == true) 
    if (
      check(parser, TOKEN_SET) == false && 
      check(parser, TOKEN_DEFINE) == false && 
      check(parser, TOKEN_CLASS) == false
    ) {
      error(parser, parser->previous, compile_time_errors[GLOBAL_CAN_SET_DEFINE_CLASS]);
      return;
    }

  if (match(parser, TOKEN_SET) == true)
    set(parser, force);
  else if (match(parser, TOKEN_DEFINE) == true)
    define(parser, force);
  else if (match(parser, TOKEN_CLASS) == true)
    class(parser, force);
  else statement(parser);

  if (parser->panic) 
    synchronize(parser);
}

static void parse(Parser* parser, Precedences precedence) {
  advance(parser);

  Execute prefix = rules[parser->previous.type].prefix;

  if (prefix == NULL) {
    error(parser, parser->previous, compile_time_errors[EXPECT_EXPRESSION]);
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
    error(parser, parser->previous, compile_time_errors[INVALID_ASSIGNMENT_TARGET]);
}

Function* compile(VM* vm, const char* source) {
  Parser parser; 

  parser.vm = vm;

  parser.entity = NULL;
  parser.compiler = NULL;

  parser.panic = false; parser.error = false;

  set_tokenizer(&parser.tokenizer, source);

  advance(&parser);

  Compiler compiler;

  set_compiler(&parser, &compiler, POSITION_SCRIPT);
  
  while (match(&parser, TOKEN_EOF) == false)
    instruction(&parser);

  if (parser.error == false)
    return terminate(&parser);
  else return NULL;
}