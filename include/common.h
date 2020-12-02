#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "gmp.h"

#include "utilities/generic.h"

#define NULL_TERMINATOR '\0'

#define LINE_LENGTH_MAX 1024

#define DEFAULT_THRESHOLD 1024 * 8

#define GMP_MAX_PRECISION 100024

#define UINT8_COUNT ( UINT8_MAX + 1 )

extern const char* compile_time_errors[];
extern const char* run_time_errors[];
extern const char* read_file_errors[];

typedef struct VM VM;

#endif