#include "common.h"
#include "helpers/debug.h"
#include "utils/chunk.h"
#include "vm.h"

int main(int argc, const char* argv[]) {
  VM vm;

  initialize_VM(&vm);

  Chunk chunk;

  initialize_chunk(&chunk);

  int first = add_constant(&chunk, 1.2);
  write_chunk(&chunk, OP_CONSTANT, 1);
  write_chunk(&chunk, first, 1);

  int second = add_constant(&chunk, 125000.505);
  write_chunk(&chunk, OP_CONSTANT, 2);
  write_chunk(&chunk, second, 2);

  write_chunk(&chunk, OP_RETURN, 3);

  interpret(&vm, &chunk);

  free_VM(&vm);

  free_chunk(&chunk);

  return 0;
}