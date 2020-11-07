#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "gmp.h"

#define NULL_TERMINATOR '\0'

#define GMP_MAX_PRECISION 100024

#define MAXIMUM_PRECISION 18

#define FRAMES_MAX 128

#define STACK_MAX ( FRAMES_MAX * (UINT8_MAX + 1) )

#endif