#ifndef HASH_H
#define HASH_H

#include "common.h"

#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
    #define BITS(d) (*((const uint16_t *) (d)))
#endif

#if !defined (BITS)
    #define BITS(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8) \
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

uint32_t hashing(const char* string, int length);

#endif