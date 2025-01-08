#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <gtk/gtk.h>

#include "config.h"
#include "common.h"
#include "table.h"

typedef struct {
  char *key;
  void *value;
  bool enabled;
} Entry;

typedef struct {
  int capacity;
  int count;
  Entry *entries;
} Table;

Table variables;
Table namedWidgets;

static void initTable(Table *table) {
  table->capacity = 0;
  table->count = 0;
  table->entries = NULL;
}

static void freeTable(Table *table) {
  free(table->entries);
  initTable(table);
}

static uint32_t hashString(const char *chars) {
  int length = strlen(chars);
  uint32_t hash = 2166136261u;
  for (int i = 0; i < length; i++) {
    hash ^= (uint8_t) chars[i];
    hash *= 16777619;
  }
  return hash;
}

static Entry *findEntry(Entry *entries, int capacity, const char *key) {
  uint32_t hash = hashString(key);
  uint32_t index = hash & (capacity - 1); /* Capacity is always a power of 2, thus this works. */
  Entry *tombstone = NULL;

  while (true) {
    Entry *entry = &entries[index];
    if (entry->key == NULL || strcmp(entry->key, key) == 0) {
      return entry;
    }

    index = (index + 1) & (capacity - 1);
  }
}

static void adjustCapacity(Table *table, int capacity) {
  Entry *entries = allocate(sizeof(Entry) * capacity, "Ran out of memory allocating table.");
  for (int i = 0; i < capacity; i++) {
    entries[i].key = NULL;
    entries[i].value = NULL;
    entries[i].enabled = false;
  }

  int count = 0;
  for (int i = 0; i < table->capacity; i++) {
    Entry *entry = &table->entries[i];
    if (entry->key == NULL) continue;

    Entry *dest = findEntry(entries, capacity, entry->key);
    count++;
    dest->key = entry->key;
    dest->value = entry->value;
  }

  free(table->entries);
  table->entries = entries;
  table->count = count;
  table->capacity = capacity;
}
 
static bool tableSet(Table *table, const char *key, void *value) {
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD_FACTOR) {
    int capacity = (table->capacity < 8 ? 8 : (table->capacity * 2));
    adjustCapacity(table, capacity);
  }

  Entry *entry = findEntry(table->entries, table->capacity, key);
  bool isNewKey = entry->key == NULL;
  if (isNewKey) table->count++;

  entry->key = key;
  entry->value = value;
  entry->enabled = true;
  return isNewKey;
}

static bool tableGet(Table *table, const char *key, void **value) {
  if (table->count == 0) return false;

  Entry *entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL) {
    return false;
  }

  if (entry->enabled) *value = entry->value;
  return true;
}
/*
static bool tableDelete(Table *table, char *key) {
  if (table->count == 0) return false;

  Entry *entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL) return false;

  entry->key == NULL;
  entry->tombstone = true;
  return true;
} */

static bool tableEnable(Table *table, const char *key, bool shouldEnable) {
  if (table->count == 0) return false;

  Entry *entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL) return false;

  entry->enabled = shouldEnable;
  return true;
}

static bool tableGetEnable(Table *table, const char *key, bool *enable) {
  if (table->count == 0) return false;

  Entry *entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL) return false;

  *enable = entry->enabled;
  return true;
}

static void printTable(Table *table) {
  printf("{ ");
  int entryCount = 0;
  for (int i = 0; i < table->capacity; i++) {
    Entry *entry = &table->entries[i];
    if (entry->key != NULL) {
      if (entryCount > 0) {
	printf(", ");
      }
      entryCount++;
      printf("%s", entry->key);
      printf(" : ");
      printf("%p", entry->value);
    }
  }
  printf(" }");
}

void printVariables() {
  printTable(&variables);
  printf("\n");
}

char *getVariable(const char *key) {
  void *result = NULL;
  bool success = tableGet(&variables, key, &result);
  if (success) {
    return (char *)result;
  } else {
    return NULL;
  }
}

bool teachVariable(const char *key) {
  return tableSet(&variables, key, NULL);
}

bool setVariable(const char *key, char *value) {
  return tableSet(&variables, key, (void *) value);
}

bool enableVariable(const char *key, bool shouldEnable) {
  return tableEnable(&variables, key, shouldEnable);
}

bool getEnableVariable(const char *key) {
  bool isEnabled = false;
  if (tableGetEnable(&variables, key, &isEnabled)) return isEnabled;
  return false;
}

GtkWidget *getWidget(const char *name) {
  void *result = NULL;
  bool success = tableGet(&namedWidgets, name, &result);
  if (success) {
    return (GtkWidget *)result;
  } else {
    return NULL;
  }
}

bool teachWidget(const char *name) {
  return tableSet(&namedWidgets, name, NULL);
}

bool setWidget(const char *name, GtkWidget *widget) {
  return tableSet(&namedWidgets, name, (void *) widget);
}

bool setSensitiveWidget(const char *name, bool sensitivity) {
  GtkWidget *widget = getWidget(name);
  if (widget == NULL) return false;
  gtk_widget_set_sensitive(widget, sensitivity);
  return true;
}
