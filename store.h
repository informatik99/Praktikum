#ifndef PRAKTIKUM_STORE_H
#define PRAKTIKUM_STORE_H

#include "constants.h"

typedef struct KeyValue
{
    char key[MAX_KEY_LENGTH];
    char value[MAX_VALUE_LENGTH];
} KeyValue;

typedef struct KeyValueStore
{
    KeyValue keyValues[MAX_STORE_KEY_VALUE_ENTRIES];
    int inStore;
} KeyValueStore;


//////// STORE FUNCTIONS //////////////////////////////////

int store_init(KeyValueStore *keyValueStore);

int store_free(KeyValueStore *keyValueStore);

int store_put(KeyValueStore *keyValueStore, const char *key, const char *value);

int store_get(KeyValueStore *keyValueStore, const char *key, char *result);

int store_del(KeyValueStore *keyValueStore, const char *key);

int store_test_many_put_get_del(KeyValueStore *keyValueStore, int numTests);

void store_print(KeyValueStore *keyValueStore);

#endif //PRAKTIKUM_STORE_H
