#ifndef TABLE_H
#define TABLE_H

#include "common.h"

#include "types/value.h"

#define MAX_LOAD 0.75

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

#endif