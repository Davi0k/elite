#include <stdlib.h>
#include <string.h>

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

void table_copy(Table* from, Table* to) {
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