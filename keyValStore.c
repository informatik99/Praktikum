#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "keyValStore.h"
#include "stdio.h"
#include <semaphore.h>

int db_init(KeyValueDatabase *db, int isSharedBetweenProcesses) {
    db->inStore = 0;
    sem_init(&db->semPutGetDelPrint, isSharedBetweenProcesses, 1);
    return DB_OK;
}

int db_free(KeyValueDatabase *db) {
    db->inStore = 0;
    return DB_OK;
}

int db_put(KeyValueDatabase *db, const char *key, const char *value) {
    KeyValue new = {"", ""};

    strncpy(new.key, key, MAX_KEY_LENGTH);
    strncpy(new.value, value, MAX_VALUE_LENGTH);

    /* Geht den Store durch, um zu schauen, ob dieser Key schon einen Wert hat */
    for (int i = 0; i < db->inStore; i++) {
        if (strncmp(db->store[i].key, key, MAX_KEY_LENGTH) == 0) {
            strncpy(db->store[i].value, value, MAX_VALUE_LENGTH);
            return DB_OK;
        }
    }

    // der Key ist nicht im Store, jetzt müssen wir noch checken,
    // ob wir genügend Platz haben, sonst bekommen wir vielleicht einen
    // BufferOverflow (IndexOutOfBoundsException)
    if (db->inStore >= KEY_VALUE_MAX_LENGTH) {
        // kein Platz mehr
        return DB_FAIL_NOT_ENOUGH_SPACE;
    }

    // falls nicht, wird das neue KeyValue-Objekt hier in den Store getan
    db->store[db->inStore++] = new;
    return DB_OK;
}

int db_get(KeyValueDatabase *db, const char *key, char *result) {
    for (int i = 0; i < db->inStore; i++) {
        if (strncmp(db->store[i].key, key, MAX_KEY_LENGTH) == 0) {
            strncpy(result, db->store[i].value, MAX_VALUE_LENGTH);
            return DB_OK;
        }
    }
    memset(result, 0, MAX_VALUE_LENGTH);
    return DB_FAIL_KEY_NON_EXISTENT;
}

int db_del(KeyValueDatabase *db, const char *key) {
    // falls es keinen eintrag gibt,
    // gib einen fehler zurück
    if (db->inStore == 0) {
        return DB_FAIL_KEY_NON_EXISTENT;
    }

    // finde die richtige position im store
    int keyIndex = 0;
    for (keyIndex = 0; keyIndex < db->inStore; keyIndex++) {
        if (strcmp(db->store[keyIndex].key, key) == 0) {
            break;
        }
    }

    // falls es den key nicht gibt,
    // gib einen fehler aus
    if (keyIndex >= db->inStore) {
        return DB_FAIL_KEY_NON_EXISTENT;
    }

    // tausche den letzten eintrag mit dem
    // des gesuchten keys
    KeyValue tmp = db->store[keyIndex];
    db->store[keyIndex] = db->store[db->inStore - 1];
    db->store[db->inStore - 1] = tmp;

    // der jetzt letzte eintrag kann
    // gelöscht werden
    db->inStore--;

    // alles in ordnung
    return DB_OK;
}

int db_beg(KeyValueDatabase *db){
    sem_wait(&db->semPutGetDelPrint);
    return DB_OK;
}

int db_end(KeyValueDatabase *db){
    sem_post(&db->semPutGetDelPrint);
    return DB_OK;
}

int db_print(KeyValueDatabase *db) {
    printf("Im Store sind:\n");
    for (int i = 0; i < db->inStore; i++) {
        printf("(Key: \"%s\" Wert: \"%s\")", db->store[i].key, db->store[i].value);
    }
    return DB_OK;
}

int db_test_simple_put_get_del(KeyValueDatabase *db, const char *key, const char *value) {
    char result[MAX_VALUE_LENGTH];
    char delResultStr[MAX_VALUE_LENGTH];

    int putGetPassed = 1;
    int delPassed = 1;

    db_put(db, key, value);
    db_get(db, key, result);
    if (strcmp(value, result) == 0) {
        fprintf(stdout,
                "PASSED put_get_del: PUT %s %s\n",
                key, value);
    } else {
        fprintf(stderr, "FAILED put_get_del: PUT %s %s \n\tres: %s\n",
                key, value, result);
        putGetPassed = 0;
    }

    int delStatus = db_del(db, key);
    if (delStatus < 0) {
        fprintf(stderr, "FAILED del: statusCode %d for DEL %s\n", delStatus, key);
        delPassed = 0;
    }

    db_get(db, key, delResultStr);
    if (strncmp(delResultStr, value, MAX_VALUE_LENGTH) == 0) {
        fprintf(stderr, "FAILED del: DEL %s  res: %s", delResultStr, value);
        delPassed = 0;
    }
    db_del(db, key);
    if(putGetPassed && delPassed) {
        return DB_OK;
    }
    return DB_FAIL_TEST;
}

int db_test_many_put_get_del(KeyValueDatabase *db, int numTests, int seed) {
    char randomKey[MAX_KEY_LENGTH];
    char randomValue[MAX_VALUE_LENGTH];
    srand(seed);
    for (int t = 0; t < numTests; t++) {
        for (int i = 0; i < MAX_VALUE_LENGTH - 1; i++) {
            randomValue[i] = (char) (rand() % (126 - 32) + 32);
        }
        randomValue[MAX_VALUE_LENGTH - 1] = '\0';

        for (int i = 0; i < MAX_KEY_LENGTH - 1; i++) {
            randomKey[i] = (char) (rand() % (126 - 32) + 32);
        }
        randomKey[MAX_KEY_LENGTH - 1] = '\0';

        int testStatus = db_test_simple_put_get_del(db, randomKey, randomValue);
        if (testStatus != DB_OK) {
            return DB_FAIL_TEST;
        }
    }
    return DB_OK;
}

int db_test(KeyValueDatabase *db) {
    return db_test_many_put_get_del(db, KEY_VALUE_MAX_LENGTH, time(NULL));
}







