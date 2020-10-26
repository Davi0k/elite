#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "tokenizer.h"
#include "vm.h"

#include "helpers/debug.h"

void compile(VM* vm, const char* source) {
  Tokenizer tokenizer;

  initialize_tokenizer(&tokenizer, source);

  print_tokenizer(source);
}