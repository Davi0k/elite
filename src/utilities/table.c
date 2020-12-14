#include <stdlib.h>
#include <string.h>

#include "vm.h"
#include "utilities/table.h"
#include "utilities/memory.h"

void initialize_table(Table* table, VM* vm) {
  table->count = 0;
  table->capacity = 0;
  table->entries = NULL;

  table->vm = vm;
}

void free_table(Table* table) {
  FREE_ARRAY(table->vm, Entry, table->entries, table->capacity);
}

static Entry* find_entry(Entry* entries, int capacity, String* key) {
  uint32_t index = key->hash % capacity;

  Entry* tombstone = NULL;

  while(true) {
    Entry* entry = &entries[index];

    if (entry->key == key || entry->key == NULL)
      return entry;

    if (entry->key == NULL) {
      if (IS_UNDEFINED(entry->value)) 
        return tombstone != NULL ? tombstone : entry;

      if (tombstone == NULL) tombstone = entry;
    } 
    
    if (entry->key == key)
      return entry;

    index = (index + 1) % capacity;
  }
}

static void fix_capacity(Table* table, int capacity) {
  Entry* entries = ALLOCATE(table->vm, Entry, capacity);

  for (int i = 0; i < capacity; i++) {
    entries[i].key = NULL;
    entries[i].value = UNDEFINED;
  }

  table->count = 0;

  for (int i = 0; i < table->capacity; i++) {
    Entry* entry = &table->entries[i];
    if (entry->key == NULL) continue;

    Entry* new = find_entry(entries, capacity, entry->key);
    new->key = entry->key;
    new->value = entry->value;

    table->count++;
  }

  FREE_ARRAY(table->vm, Entry, table->entries, table->capacity);

  table->entries = entries;
  table->capacity = capacity;
}

bool table_set(Table* table, String* key, Value value) {
  if (table->count + 1 > table->capacity * MAX_LOAD) {
    int capacity = GROW_CAPACITY(table->capacity);
    fix_capacity(table, capacity);
  }

  Entry* entry = find_entry(table->entries, table->capacity, key);

  bool new = entry->key == NULL;

  if (new && IS_UNDEFINED(entry->value)) table->count++;

  entry->key = key;
  entry->value = value;

  return new;
}

bool table_get(Table* table, String* key, Value* value) {
  if (table->count == 0) return false;

  Entry* entry = find_entry(table->entries, table->capacity, key);

  if (entry->key == NULL) return false;

  *value = entry->value;

  return true;
}

bool table_delete(Table* table, String* key) {
  if (table->count == 0) return false;

  Entry* entry = find_entry(table->entries, table->capacity, key);

  if (entry->key == NULL) return false;

  entry->key = NULL;
  entry->value = VOID;

  return true;
}

void table_clear(Table* table) {
  for (int i = 0; i < table->capacity; i++) {
    Entry* entry = &table->entries[i];

    if (entry->key != NULL && entry->key->object.mark == false)
      table_delete(table, entry->key);
  }
}

void table_append(Table* from, Table* to) {
  for (int i = 0; i < from->capacity; i++) {
    Entry* entry = &from->entries[i];

    if (entry->key != NULL) 
      table_set(to, entry->key, entry->value);
  }
}

String* table_find_string(Table* table, const char* content, int length, uint32_t hash) {
  if (table->count == 0) return NULL;

  uint32_t index = hash % table->capacity;

  while (true) {
    Entry* entry = &table->entries[index];

    if (entry->key == NULL) 
      if (IS_UNDEFINED(entry->value)) return NULL;
    
    if (entry->key->length == length && entry->key->hash == hash && memcmp(entry->key->content, content, length) == 0)
      return entry->key;

    index = (index + 1) % table->capacity;
  }
}

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