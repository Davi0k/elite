#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "gmp.h"

#define NULL_TERMINATOR '\0'

#define GMP_MAX_PRECISION 100024

#define UINT8_COUNT ( UINT8_MAX + 1 )

#define FRAMES_MAX 128

#define STACK_MAX ( FRAMES_MAX * UINT8_COUNT )

#define LINE_LENGTH_MAX 1024

extern const char* compile_time[];
extern const char* run_time[];
extern const char* command_line_interface[];

#endif