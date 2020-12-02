#include <stdio.h>
#include <string.h>

#include "tokenizer.h"

void set_tokenizer(Tokenizer* tokenizer, const char* source) {
  tokenizer->start = source;
  tokenizer->current = source;
  tokenizer->line = 1;
}

static Token make(Tokenizer* tokenizer, Types type) {
  Token token;

  token.type = type;

  token.start = tokenizer->start;
  token.length = (int)(tokenizer->current - tokenizer->start);
  token.line = tokenizer->line;

  return token;
}

static Token error(Tokenizer* tokenizer, const char* message) {
  Token token;

  token.type = TOKEN_ERROR;

  token.start = message;
  token.length = (int)strlen(message);

  token.line = tokenizer->line;

  return token;
}

static char advance(Tokenizer* tokenizer) {
  tokenizer->current++;
  return tokenizer->current[-1];
}

static char match(Tokenizer* tokenizer, char expected) {
  if (*tokenizer->current == NULL_TERMINATOR) return false;

  if (*tokenizer->current != expected) return false;

  tokenizer->current++;

  return true;
}

static void skip_whitespace(Tokenizer* tokenizer) {
  while(true) {
    char character = *tokenizer->current;

    switch(character) {
      case ' ':
      case '\r':
      case '\t':
        advance(tokenizer);
        break;

      case '\n':
        tokenizer->line++;
        advance(tokenizer);
        break;

      case '#':
        while (*tokenizer->current != '\n' && *tokenizer->current != NULL_TERMINATOR) 
          advance(tokenizer);

        break;

      case '/':
        if (tokenizer->current[1] == '*') {
          advance(tokenizer); advance(tokenizer);

          while (true) {
            if (*tokenizer->current == '*' && tokenizer->current[1] == '/') {
              advance(tokenizer); advance(tokenizer);
              break;
            }
            
            if (*tokenizer->current == NULL_TERMINATOR)
              break;

            if (*tokenizer->current == '\n')
              tokenizer->line++;

            advance(tokenizer);
          }
        
          break;
        } 
        else return;

      default: return;
    }
  }
}

static bool alpha(char character) {
  return (character >= 'a' && character <= 'z') ||
         (character >= 'A' && character <= 'Z') ||
         character == '_' || 
         character == '$';
}

static bool digit(char character) {
  return character >= '0' && character <= '9';
}

static Types keyword(Tokenizer* tokenizer, int start, int length, const char* rest, Types type) {
  if (tokenizer->current - tokenizer->start == start + length)
    if(memcmp(tokenizer->start + start, rest, length) == 0)
      return type;

  return TOKEN_IDENTIFIER;
}

static Types trie(Tokenizer* tokenizer) {
  switch (tokenizer->start[0]) {
    case 'a': return keyword(tokenizer, 1, 2, "nd", TOKEN_AND);
    case 'c': return keyword(tokenizer, 1, 4, "lass", TOKEN_CLASS);

    case 'd': 
      if (tokenizer->current - tokenizer->start > 1) {
        switch (tokenizer->start[1]) {
          case 'e': return keyword(tokenizer, 2, 4, "fine", TOKEN_DEFINE);
          case 'o': return keyword(tokenizer, 2, 0, NULL_TERMINATOR, TOKEN_DO);
        }
      }

      break;

    case 'e': 
      if (tokenizer->current - tokenizer->start > 1) {
        switch (tokenizer->start[1]) {
          case 'l': return keyword(tokenizer, 2, 2, "se", TOKEN_ELSE);
          case 'm': return keyword(tokenizer, 2, 3, "pty", TOKEN_EMPTY);
          case 'x': return keyword(tokenizer, 2, 2, "it", TOKEN_EXIT);
        }
      }

      break;

    case 'f': 
      if (tokenizer->current - tokenizer->start > 1) {
        switch (tokenizer->start[1]) {
          case 'a': return keyword(tokenizer, 2, 3, "lse", TOKEN_FALSE);
          case 'o': return keyword(tokenizer, 2, 1, "r", TOKEN_FOR);
        }
      }

      break;

    case 'g': return keyword(tokenizer, 1, 5, "lobal", TOKEN_GLOBAL);

    case 'i': return keyword(tokenizer, 1, 1, "f", TOKEN_IF);
    case 'n': return keyword(tokenizer, 1, 2, "ot", TOKEN_NOT);
    case 'o': return keyword(tokenizer, 1, 1, "r", TOKEN_OR);
    case 'p': return keyword(tokenizer, 1, 5, "arent", TOKEN_PARENT);
    case 'r': return keyword(tokenizer, 1, 5, "eturn", TOKEN_RETURN);
    case 's': return keyword(tokenizer, 1, 2, "et", TOKEN_SET);

    case 't':
      if (tokenizer->current - tokenizer->start > 1) {
        switch (tokenizer->start[1]) {
          case 'h': return keyword(tokenizer, 2, 2, "is", TOKEN_THIS);
          case 'r': return keyword(tokenizer, 2, 2, "ue", TOKEN_TRUE);
        }
      }

      break;

    case 'u': return keyword(tokenizer, 1, 8, "ndefined", TOKEN_UNDEFINED);
    case 'v': return keyword(tokenizer, 1, 3, "oid", TOKEN_VOID);
    case 'w': return keyword(tokenizer, 1, 4, "hile", TOKEN_WHILE);
  }

  return TOKEN_IDENTIFIER;
}

static Token identifier(Tokenizer* tokenizer) {
  while (alpha(*tokenizer->current))
    advance(tokenizer);

  Types type = trie(tokenizer);

  return make(tokenizer, type);
} 

static Token number(Tokenizer* tokenizer) {
  while (digit(*tokenizer->current))
    advance(tokenizer);

  if (*tokenizer->current == '.' && digit(tokenizer->current[1])) {
    advance(tokenizer);

    while (digit(*tokenizer->current))
      advance(tokenizer);
  }

  return make(tokenizer, TOKEN_NUMBER);
}

static Token string(Tokenizer* tokenizer, char quote) {
  while (*tokenizer->current != quote && *tokenizer->current != NULL_TERMINATOR) {
    if (*tokenizer->current == '\n') tokenizer->line++;
    advance(tokenizer);
  }

  if (*tokenizer->current == NULL_TERMINATOR) return error(tokenizer, "Unterminated string literal.");

  advance(tokenizer);

  return make(tokenizer, TOKEN_STRING);
}

Token scan(Tokenizer* tokenizer) {
  skip_whitespace(tokenizer);

  tokenizer->start = tokenizer->current;

  if (*tokenizer->current == NULL_TERMINATOR) return make(tokenizer, TOKEN_EOF);

  char character = advance(tokenizer);
    
  if (alpha(character)) return identifier(tokenizer);
  if (digit(character)) return number(tokenizer);

  switch (character) {
    case '(': return make(tokenizer, TOKEN_OPEN_PARENTHESES);
    case ')': return make(tokenizer, TOKEN_CLOSE_PARENTHESES);

    case '[': return make(tokenizer, TOKEN_OPEN_BRACKETS);
    case ']': return make(tokenizer, TOKEN_CLOSE_BRACKETS);

    case '{': return make(tokenizer, TOKEN_OPEN_BRACES);
    case '}': return make(tokenizer, TOKEN_CLOSE_BRACES);

    case ';': return make(tokenizer, TOKEN_SEMICOLON);
    case ',': return make(tokenizer, TOKEN_COMMA);
    case ':': return make(tokenizer, TOKEN_COLON);
    case '.': return make(tokenizer, TOKEN_DOT);

    case '+': return make(tokenizer, match(tokenizer, '+') ? TOKEN_INCREMENT : TOKEN_PLUS);
    case '-': return make(tokenizer, match(tokenizer, '-') ? TOKEN_DECREMENT : TOKEN_MINUS);
    case '*': return make(tokenizer, TOKEN_ASTERISK);
    case '/': return make(tokenizer, TOKEN_SLASH);

    case '^': return make(tokenizer, TOKEN_CARET);

    case '&': return make(tokenizer, TOKEN_AND);
    case '|': return make(tokenizer, TOKEN_OR);

    case '!': return make(tokenizer, match(tokenizer, '=') ? TOKEN_NOT_EQUAL : TOKEN_NOT);

    case '<': return make(tokenizer, match(tokenizer, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>': return make(tokenizer, match(tokenizer, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);

    case '=': return make(tokenizer, match(tokenizer, '=') ? TOKEN_EQUAL : TOKEN_ASSIGN);

    case '\'':
    case '"':
    case '`':
      return string(tokenizer, character);
  }

  return error(tokenizer, "Unexpected character.");
}
