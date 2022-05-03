//
// Created by HP on 09.04.2022.
//

#ifndef PRAKTIKUM_KEY_VALUE_STORE_H
#define PRAKTIKUM_KEY_VALUE_STORE_H

//#include <semaphore.h>
#include <sys/sem.h>

#define MAX_KEY_LENGTH 50
#define MAX_VALUE_LENGTH 100

typedef struct KeyValue {
    char key[MAX_KEY_LENGTH];
    char value[MAX_VALUE_LENGTH];
} KeyValue;

#define MAX_DB_ENTRIES 50
typedef struct KeyValueDatabase {
    KeyValue store[MAX_DB_ENTRIES];
    int inStore;
    int lockSemaphoreId;
} KeyValueDatabase;

typedef enum DbCommandStatus {
    DB_FAIL_NOT_ENOUGH_SPACE = -20,
    DB_FAIL_KEY_NON_EXISTENT,
    DB_FAIL_TEST,
    DB_FAIL_NO_SEMAPHORE,
    DB_OK = 1,
} DbCommandStatus;

int db_init(KeyValueDatabase *db);

int db_free(KeyValueDatabase *db);

int db_put(KeyValueDatabase *db, const char *key, const char *value);

int db_get(KeyValueDatabase *db, const char *key, char *result);

int db_del(KeyValueDatabase *db, const char *key);

int db_print(KeyValueDatabase *db);

int db_test(KeyValueDatabase *db);

int db_lock(KeyValueDatabase *db);

int db_unlock(KeyValueDatabase *db);

#endif //PRAKTIKUM_KEY_VALUE_STORE_H
