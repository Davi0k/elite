#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include "common.h"
#include "vm.h"

#define MAX_CHARACTERS 1024

static char* read(const char* path) {
  FILE* file = fopen(path, "rb");

  if (file == NULL) {
    fprintf(stderr, "Could not open file <%s>.", path);
    exit(74);
  }

  fseek(file, 0L, SEEK_END);
  size_t size = ftell(file);
  rewind(file);

  char* buffer = (char*)malloc(size + 1);

  if (buffer == NULL) {
    fprintf(stderr, "Not enough memory to read <%s>.", path);
    exit(74);
  }

  size_t bytes = fread(buffer, sizeof(char), size, file);

  if (bytes < size) {
    fprintf(stderr, "Could not read file <%s>.", path);
    exit(74);
  }

  buffer[bytes] = NULL_TERMINATOR;

  fclose(file);
  
  return buffer;
}

static void repl(VM* vm) {
  char line[MAX_CHARACTERS];

  while(true) {
    printf("> ");

    if (fgets(line, sizeof(line), stdin) == NULL) {
      printf("\n");
      break;
    }

    interpret(vm, line);
  }
}

static void execute_file(VM* vm, const char* path) {
  char* source = read(path);

  clock_t begin = clock();
  Results result = interpret(vm, source);
  clock_t end = clock();

  printf("\nExecution Time: %f Seconds\n", (double)(end - begin) / CLOCKS_PER_SEC);

  free(source);

  if (result == INTERPRET_COMPILE_ERROR) exit(65);
  if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, const char* argv[]) {
  VM vm;

  initialize_VM(&vm);

  if (argc == 1) repl(&vm);

  if (argc == 2) execute_file(&vm, argv[1]);

  if (argc != 1 && argc != 2) {
    fprintf(stderr, "The correct Syntax is: elite [path].");
    exit(64);
  }

  free_VM(&vm);

  return 0;
}