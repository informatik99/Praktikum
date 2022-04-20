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

int put(char *key, char *value);
int get(char *key, char *res);
int del(char *key);
void printStore();


#endif //PRAKTIKUM_KEYVALSTORE_H
