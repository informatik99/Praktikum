//
// Created by HP on 09.04.2022.
//

#ifndef PRAKTIKUM_KEYVALSTORE_H
#define PRAKTIKUM_KEYVALSTORE_H

#define MAX_KEY_LENGTH 50
#define MAX_VALUE_LENGTH 50

typedef struct KeyValue_{
    char key[MAX_KEY_LENGTH];
    char value[MAX_VALUE_LENGTH];
}KeyValue;

#define KEY_VALUE_MAX_LENGTH 50
typedef struct KeyValueDatabase {
    int inStore;
    KeyValue store[KEY_VALUE_MAX_LENGTH];
} KeyValueDatabase;


int db_init(KeyValueDatabase *db);
int db_free(KeyValueDatabase *db);

int db_put(KeyValueDatabase *db, const char *key, const char *value);
int db_get(KeyValueDatabase *db, const char *key, char *result);
int db_del(KeyValueDatabase *db, const char *key);
int db_print(KeyValueDatabase *db);


int put(char *key, char *value);
int get(char *key, char *res);
int del(char *key);
int printStore();

#endif //PRAKTIKUM_KEYVALSTORE_H
