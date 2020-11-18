#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "vm.h"

#define VERSION "1.0.0"

typedef enum {
  SYNTAX_ERROR,
  NO_OPTIONAL_PARAMETER,
  COULD_NOT_READ_FILE,
  COULD_NOT_OPEN_FILE,
  NOT_ENOUGH_MEMORY
} COMMAND_LINE_INTERFACE_ERRORS;

const char* errors[] = {
  [SYNTAX_ERROR] = "Expected %d arguments but got %d.",
  [NO_OPTIONAL_PARAMETER] = "A Stack Overflow error has occured.",
  [COULD_NOT_READ_FILE] = "Can only call functions and classes.",
  [COULD_NOT_OPEN_FILE] = "Could not open file <%s>.",
  [NOT_ENOUGH_MEMORY] = "Operand must be a Number or a String."
};

static char* read(const char* path) {
  FILE* file = fopen(path, "rb");

  if (file == NULL) {
    fprintf(stderr, errors[COULD_NOT_OPEN_FILE], path);
    exit(74);
  }

  fseek(file, 0L, SEEK_END);
  size_t size = ftell(file);
  rewind(file);

  char* buffer = (char*)malloc(size + 1);

  if (buffer == NULL) {
    fprintf(stderr, errors[NOT_ENOUGH_MEMORY], path);
    exit(74);
  }

  size_t bytes = fread(buffer, sizeof(char), size, file);

  if (bytes < size) {
    fprintf(stderr, errors[COULD_NOT_READ_FILE], path);
    exit(74);
  }

  buffer[bytes] = NULL_TERMINATOR;

  fclose(file);
  
  return buffer;
}

static void repl(VM* vm) {
  char* line = NULL;

  size_t size = 0;

  while(true) {
    printf("> ");

    getline(&line, &size, stdin);

    interpret(vm, line);
  }
}

static void file(VM* vm, const char* path) {
  char* source = read(path);

  Results result = interpret(vm, source);

  free(source);

  if (result == INTERPRET_COMPILE_ERROR) exit(65);
  if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, const char* argv[]) {
  mpf_set_default_prec(GMP_MAX_PRECISION);

  VM vm;

  initialize_VM(&vm);

  if (argc == 1) repl(&vm);

  if (argc == 2)  {
    const char* parameter = argv[1];

    if (parameter[0] == '-') {
      switch (parameter[1]) {
        case 'v':
          printf("Elite %s", VERSION);
          break;

        default:
          fprintf(stderr, errors[NO_OPTIONAL_PARAMETER], parameter);
          exit(64);
      }
    }
    else file(&vm, argv[1]);
  }

  if (argc != 1 && argc != 2) {
    fprintf(stderr, errors[SYNTAX_ERROR]);
    exit(64);
  }

  free_VM(&vm);

  return 0;
}