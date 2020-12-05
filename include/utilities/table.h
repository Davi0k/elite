#ifndef TABLE_H
#define TABLE_H

#include "common.h"

#include "types/value.h"

#define MAX_LOAD 0.75

#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
    #define BITS(d) (*((const uint16_t *) (d)))
#endif

#if !defined (BITS)
    #define BITS(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8) \
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

typedef struct {
  String* key;
  Value value;
} Entry;

typedef struct {
  int count;
  int capacity;
  Entry* entries;

  VM* vm;
} Table;

void initialize_table(Table* table, VM* vm);
void free_table(Table* table);

bool table_set(Table* table, String* key, Value value);
bool table_get(Table* table, String* key, Value* value);

bool table_delete(Table* table, String* key);

void table_clear(Table* table);

void table_copy(Table* from, Table* to);

String* table_find_string(Table* table, const char* content, int length, uint32_t hash);

uint32_t hashing(const char* string, int length);

#endif