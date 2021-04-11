#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#include "vm.h"

#define VERSION \
  "Elite 1.0.0\n" \
  "Repository: https://github.com/Davi0k/elite\n" \
  "About me: https://davide.codes\n"

#define HELP \
  "Usage: elite [path] [-v] [-h]\n" \
  "\tpath: The path of the script you want to execute.\n" \
  "Options:\n" \
  "\t-v: Returns the current interpreter's version.\n" \
  "\t-h: Returns a list of the available settings and options for the interpreter.\n"

static void repl(VM* vm) {
  size_t size = 0;

  char* line = NULL;

  while(true) {
    printf("> ");

    if (getline(&line, &size, stdin) == -1) {
      printf("\n");
      break;
    }

    interpret(vm, line);
  }
}

static void file(VM* vm, const char* path) {
  int code = 0;

  char* source = read(path, &code);

  if (source == NULL) {
    switch (code) {
      case CANNOT_OPEN_FILE:
        fprintf(stderr, read_file_errors[CANNOT_OPEN_FILE], path);
        exit(74);

      case CANNOT_READ_FILE:
        fprintf(stderr, read_file_errors[CANNOT_READ_FILE], path);
        exit(74);

      case NOT_ENOUGH_MEMORY:
        fprintf(stderr, read_file_errors[NOT_ENOUGH_MEMORY]);
        exit(74);
    }
  }

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
        case 'V':
          printf(VERSION);
          break;

        case 'h':
        case 'H': 
          printf(HELP);
          break;
      }
    }
    else file(&vm, argv[1]);
  }

  if (argc != 1 && argc != 2) {
    fprintf(stderr, "The correct syntax is: elite [path] [-v] [-h]");
    exit(64);
  }

  free_VM(&vm);

  return 0;
}