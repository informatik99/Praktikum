#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
//#include <semaphore.h>
#include <sys/sem.h>

#include "keyValStore.h"

int db_init(KeyValueDatabase *db) {

    // am Anfang haben wir keine
    // Wert-Schlüsselpaare gespeichert
    db->inStore = 0;

    // man muss einen exklusiven Zugriff
    // auf die Datenbank sicherstellen können,
    // dafür benötigen wir einen semaphore
    int numOfSemaphores = 1;
    int sharedMemReadWritePrivilege = 0644;
    short initialSemaphoreValue = 1;
    db->lockSemaphoreId = semget(IPC_PRIVATE,
                                 numOfSemaphores,
                                 IPC_CREAT | sharedMemReadWritePrivilege);
    if(db->lockSemaphoreId < 0){
        fprintf(stderr, "The database semaphore couldn't be created\n");
        return DB_FAIL_NO_SEMAPHORE;
    }
    semctl(db->lockSemaphoreId,1,SETALL, &initialSemaphoreValue);


    return DB_OK;
}

int db_free(KeyValueDatabase *db) {

    // wir speichern keine
    // Wert-Schlüsselpaare mehr
    db->inStore = 0;

    // der initialisierte semaphore,
    // sollte wieder zerstört werden
    semctl(db->lockSemaphoreId, 0, IPC_RMID);

    return DB_OK;
}

int db_put(KeyValueDatabase *db, const char *key, const char *value) {

    // dieser KeyValue soll die
    // Eingabewerte zwischenspeichern
    KeyValue new = {"", ""};
    strncpy(new.key, key, MAX_KEY_LENGTH);
    strncpy(new.value, value, MAX_VALUE_LENGTH);

    // gehe den Store durch, um zu schauen,
    // ob dieser Key schon einen Wert hat
    for (int i = 0; i < db->inStore; i++) {
        if (strncmp(db->store[i].key, key, MAX_KEY_LENGTH) == 0) {
            strncpy(db->store[i].value, value, MAX_VALUE_LENGTH);
            return DB_OK;
        }
    }

    // der Key ist nicht im Store.
    // jetzt müssen wir noch checken,
    // ob wir genügend Platz haben,
    // sonst bekommen wir vielleicht einen
    // BufferOverflow (IndexOutOfBoundsException)
    if (db->inStore >= MAX_DB_ENTRIES) {
        // kein Platz mehr
        return DB_FAIL_NOT_ENOUGH_SPACE;
    }

    // falls nicht, wird das neue KeyValue-Objekt
    // hier in den Store getan
    db->store[db->inStore++] = new;
    return DB_OK;
}

int db_get(KeyValueDatabase *db, const char *key, char *result) {

    // gehe durch die daten durch und schaue
    // ob der Key übereinstimmt
    for (int i = 0; i < db->inStore; i++) {

        if (strncmp(db->store[i].key, key, MAX_KEY_LENGTH) == 0) {

            // der gesuchte key wurde gefunden...
            // jetzt können wir den entsprechenden value zurückschreiben
            strncpy(result, db->store[i].value, MAX_VALUE_LENGTH);
            return DB_OK;
        }

    }

    // der key wurde nicht gefunden,
    // deswegen geben wir am besten
    // einen leeren string zurück
    memset(result, 0, MAX_VALUE_LENGTH);

    return DB_FAIL_KEY_NON_EXISTENT;
}

int db_del(KeyValueDatabase *db, const char *key) {

    // falls es keinen eintrag gibt,
    // gib einen fehler zurück
    if (db->inStore == 0) {
        return DB_FAIL_KEY_NON_EXISTENT;
    }

    // finde die richtige
    // position im store
    int keyIndex;
    for (keyIndex = 0; keyIndex < db->inStore; keyIndex++) {
        if (strcmp(db->store[keyIndex].key, key) == 0) {
            break;
        }
    }

    // falls es den key nicht gibt,
    // gib einen fehler zurück
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

int db_lock(KeyValueDatabase *db){

    // sem_wait erniedrigt den jeweiligen semaphore um eins.
    // sobald der semaphore 0 ist, muss gewartet werden.
    // ein semaphore der mit 1 initialisiert wurde, sorgt
    // also für einen exklusiven Zugriff
    struct sembuf semDownBuf;
    semDownBuf.sem_op = -1;
    semDownBuf.sem_num = 0;
    semDownBuf.sem_flg = SEM_UNDO;
    semop(db->lockSemaphoreId, &semDownBuf,1);

    return DB_OK;
}

int db_unlock(KeyValueDatabase *db){

    // sem_post erhöht den semaphore um 1 und hebt
    // damit den exklusiven Zugriff wieder auf
    //sem_post(&db->lockSemaphore);
    struct sembuf semUpBuf;
    semUpBuf.sem_op = 1;
    semUpBuf.sem_num = 0;
    semUpBuf.sem_flg = SEM_UNDO;
    semop(db->lockSemaphoreId,&semUpBuf,1);

    return DB_OK;
}

int db_print(KeyValueDatabase *db) {

    // gehe durch die daten
    // und gebe sie einfach aus
    printf("Store:\n");
    for (int i = 0; i < db->inStore; i++) {
        printf("(Key: \"%s\" Value: \"%s\")",
               db->store[i].key, db->store[i].value);
    }
    return DB_OK;
}

int db_test_put_get_del_get(KeyValueDatabase *db, const char *key, const char *value) {

    char result[MAX_VALUE_LENGTH];
    int statusCode;

    // teste, ob einfügen funktioniert
    statusCode = db_put(db, key, value);

    if(statusCode != DB_OK){
        fprintf(stderr,
                "FAILED PUT: PUT %s %s  status code %d\n",
                key, value, statusCode);
        return DB_FAIL_TEST;
    }

    // einfügen funktioniert
    fprintf(stdout,
            "PASSED PUT: PUT %s %s\n",
            key, value);

    // teste, ob wir das zurückbekommen,
    // was wir reingetan haben
    statusCode = db_get(db, key, result);

    if(statusCode != DB_OK){
        fprintf(stderr,
                "FAILED GET after PUT: PUT_GET %s %s\tstatusCode: %d",
                    key, value, statusCode);
        return DB_FAIL_TEST;
    }

    if(strcmp(value, result) != 0) {
        fprintf(stderr,
                "FAILED GET after PUT: PUT_GET %s %s \n\tGET res: %s\n",
                key, value, result);
        return DB_FAIL_TEST;
    }

    // zurückbekommen hat funktioniert
    fprintf(stdout,
            "PASSED GET after PUT: GET %s: %s = %s \n",
            key, value, result);

    // teste, ob löschen funktioniert
    statusCode = db_del(db, key);

    if(statusCode != DB_OK) {
        fprintf(stderr,
                "FAILED DEL after PUT and GET: statusCode %d for DEL %s\n",
                statusCode, key);
        return DB_FAIL_TEST;
    }

    // wenn wir zurückbekommen wollen,
    // was wir bereits gelöscht haben,
    // soll das fehlschlagen.
    // der statuscode sollte einen fehler anzeigen und
    // der wiedergegebene string sollte leer sein.
    statusCode = db_get(db, key, result);


    if(statusCode == DB_OK){
        fprintf(stderr,
                "FAILED DEL after PUT and GET: statusCode %d\n",
                statusCode);
        return DB_FAIL_TEST;
    }

    if (strncmp(result, value, MAX_VALUE_LENGTH) == 0) {
        fprintf(stderr,
                "FAILED DEL after PUT and GET: GET after DEL %s res: %s",
                value, result);
    }

    // löschen funktioniert richtig
    fprintf(stdout,"PASSED DELETE: DEL %s \n",key);

    return DB_OK;
}


void string_randomize(char *input, int size, int seed){
    srand(seed);
    for (int i = 0; i < MAX_VALUE_LENGTH - 1; i++) {
        input[i] = (char) (rand() % (126 - 32) + 32);
    }
    input[size-1] = '\0';
}


int db_test_many_put_get_del(KeyValueDatabase *db, int numTests) {

    // teste mit zufälligen strings
    char randomKey[MAX_KEY_LENGTH];
    char randomValue[MAX_VALUE_LENGTH];

    for (int t = 0; t < numTests; t++) {

        // erstelle zufällige strings
        string_randomize(randomKey,MAX_KEY_LENGTH,(int) time(NULL));
        string_randomize(randomValue, MAX_VALUE_LENGTH,(int) (time(NULL)));

        // teste damit
        int testStatus = db_test_put_get_del_get(db, randomKey, randomValue);
        if (testStatus != DB_OK) {
            return DB_FAIL_TEST;
        }

    }

    // wenn alle tests funktioniert haben,
    // dann hat auch der hier funktioniert.
    return DB_OK;
}


int db_test(KeyValueDatabase *db) {
    int numTests = 500;
    return db_test_many_put_get_del(db, numTests);
}







