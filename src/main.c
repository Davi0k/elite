#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#include "vm.h"

#define VERSION "1.0.0"

static void file(VM* vm, const char* path) {
  int code;

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

  if (argc == 2)  {
    const char* parameter = argv[1];

    if (parameter[0] == '-') {
      switch (parameter[1]) {
        case 'v':
          printf("Elite %s", VERSION);
          break;
      }
    }
    else file(&vm, argv[1]);
  }

  if (argc != 1 && argc != 2) {
    fprintf(stderr, "The correct syntax is: elite [path] [-v]");
    exit(64);
  }

  free_VM(&vm);

  return 0;
}