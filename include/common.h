#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "gmp.h"

#include "helpers/generic.h"

#define NULL_TERMINATOR '\0'

#define GMP_MAX_PRECISION 100024

extern const char* compile_time_errors[];
extern const char* run_time_errors[];
extern const char* read_file_errors[];

typedef struct VM VM;

#endif