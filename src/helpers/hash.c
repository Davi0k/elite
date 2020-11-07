#include "helpers/hash.h"

uint32_t hashing(const char* string, int length) {
  uint32_t tmp, hash = length;

  if (length <= 0 || string == NULL) return 0;

  int rem = length & 3;

  length >>= 2;

  while(length > 0) {
    hash += BITS(string);

    tmp = (BITS(string + 2) << 11) ^ hash;

    hash = (hash << 16) ^ tmp;

    string += 2 * sizeof(uint16_t);

    hash += hash >> 11;

    length--;
  }

  switch (rem) {
    case 1: 
      hash += (signed char)*string;
      hash ^= hash << 10;
      hash += hash >> 1;
      break;

    case 2: 
      hash += BITS(string);
      hash ^= hash << 11;
      hash += hash >> 17;
      break;

    case 3: 
      hash += BITS(string);
      hash ^= hash << 16;
      hash ^= ( (signed char)string[sizeof(uint16_t)] ) << 18;
      hash += hash >> 11;
      break;
  }

  hash ^= hash << 3;
  hash += hash >> 5;

  hash ^= hash << 4;
  hash += hash >> 17;

  hash ^= hash << 25;
  hash += hash >> 6;

  return hash;
}