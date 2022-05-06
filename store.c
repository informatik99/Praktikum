#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "store.h"
#include "constants.h"

int store_init(KeyValueStore *keyValueStore)
{
    // am Anfang haben wir keine
    // Wert-Schlüsselpaare gespeichert
    keyValueStore->inStore = 0;
    return STATUS_OK;
}

int store_free(KeyValueStore *keyValueStore)
{
    // wir speichern keine
    // Wert-Schlüsselpaare mehr
    keyValueStore->inStore = 0;
    return STATUS_OK;
}

int store_put(KeyValueStore *keyValueStore, const char *key, const char *value)
{
    // dieser KeyValue soll die
    // Eingabewerte zwischenspeichern
    KeyValue new = {"", ""};
    strncpy(new.key, key, MAX_KEY_LENGTH);
    strncpy(new.value, value, MAX_VALUE_LENGTH);

    // gehe den Store durch, um zu schauen,
    // ob dieser Key schon einen Wert hat
    for (int i = 0; i < keyValueStore->inStore; i++)
    {
        if (strncmp(keyValueStore->keyValues[i].key, key, MAX_KEY_LENGTH) == 0)
        {
            strncpy(keyValueStore->keyValues[i].value, value, MAX_VALUE_LENGTH);
            return STATUS_OK;
        }
    }

    // der Key ist nicht im Store.
    // jetzt müssen wir noch checken,
    // ob wir genügend Platz haben,
    // sonst bekommen wir vielleicht einen
    // BufferOverflow (IndexOutOfBoundsException)
    if (keyValueStore->inStore >= MAX_STORE_KEY_VALUE_ENTRIES)
    {
        // kein Platz mehr
        return STATUS_FAIL_NOT_ENOUGH_SPACE;
    }

    // falls nicht, wird das neue KeyValue-Objekt
    // hier in den Store getan
    keyValueStore->keyValues[keyValueStore->inStore++] = new;
    return STATUS_OK;
}

int store_get(KeyValueStore *db, const char *key, char *result)
{
    // gehe durch die daten durch und schaue
    // ob der Key übereinstimmt
    for (int i = 0; i < db->inStore; i++)
    {
        if (strncmp(db->keyValues[i].key, key, MAX_KEY_LENGTH) == 0)
        {

            // der gesuchte key wurde gefunden...
            // jetzt können wir den entsprechenden value zurückschreiben
            strncpy(result, db->keyValues[i].value, MAX_VALUE_LENGTH);
            return STATUS_OK;
        }
    }
    // der key wurde nicht gefunden,
    // deswegen geben wir am besten
    // einen leeren string zurück
    memset(result, 0, MAX_VALUE_LENGTH);
    return STATUS_KEY_NOT_EXISTENT;
}

int store_del(KeyValueStore *db, const char *key)
{
    // falls es keinen eintrag gibt,
    // gib einen fehler zurück
    if (db->inStore == 0)
    {
        return STATUS_FAIL_EMPTY;
    }

    // finde die richtige
    // position im keyValues
    int keyIndex;
    for (keyIndex = 0; keyIndex < db->inStore; keyIndex++)
    {
        if (strcmp(db->keyValues[keyIndex].key, key) == 0)
        {
            break;
        }
    }

    // falls es den key nicht gibt,
    // gib einen fehler zurück
    if (keyIndex >= db->inStore)
    {
        return STATUS_KEY_NOT_EXISTENT;
    }

    // fülle den zu löschenden Platz,
    // mit dem des letzten eintrages
    KeyValue tmp = db->keyValues[keyIndex];
    db->keyValues[keyIndex] = db->keyValues[db->inStore - 1];
    db->keyValues[db->inStore - 1] = tmp;

    // der letzte eintrag
    // wird herausgenommen
    db->inStore--;

    // alles in ordnung
    return STATUS_OK;
}

void store_print(KeyValueStore *db)
{
    // gehe durch die daten
    // und gebe sie einfach aus
    printf("Store:\n");
    for (int i = 0; i < db->inStore; i++)
    {
        printf(
                "(Key: \"%s\" Value: \"%s\")\n",
                db->keyValues[i].key, db->keyValues[i].value
        );
    }
}

int db_test_put_get_del_get(KeyValueStore *db, const char *key, const char *value)
{
    char result[MAX_VALUE_LENGTH];
    int statusCode;

    // teste, ob einfügen funktioniert
    statusCode = store_put(db, key, value);

    if (statusCode < 0)
    {
        fprintf(
                stderr,
                "FAILED PUT: PUT %s %s  status code %d\n",
                key, value, statusCode
        );
        return STATUS_TEST_FAIL;
    }

    // einfügen funktioniert
    fprintf(
            stdout,
            "PASSED PUT: PUT %s %s\n",
            key, value
    );

    // teste, ob wir das zurückbekommen,
    // was wir reingetan haben
    statusCode = store_get(db, key, result);

    if (statusCode < 0)
    {
        fprintf(
                stderr,
                "FAILED GET after PUT: PUT_GET %s %s\tstatusCode: %d",
                key, value, statusCode
        );
        return STATUS_TEST_FAIL;
    }

    if (strcmp(value, result) != 0)
    {
        fprintf(
                stderr,
                "FAILED GET after PUT: PUT_GET %s %s \n\tGET res: %s\n",
                key, value, result
        );
        return STATUS_TEST_FAIL;
    }

    // zurückbekommen hat funktioniert
    fprintf(
            stdout,
            "PASSED GET after PUT: GET %s: %s = %s \n",
            key, value, result
    );

    // teste, ob löschen funktioniert
    statusCode = store_del(db, key);

    if (statusCode < 0)
    {
        fprintf(
                stderr,
                "FAILED DEL after PUT and GET: statusCode %d for DEL %s\n",
                statusCode, key
        );
        return STATUS_TEST_FAIL;
    }

    // wenn wir zurückbekommen wollen,
    // was wir bereits gelöscht haben,
    // soll das fehlschlagen.
    // der statuscode sollte einen fehler anzeigen und
    // der wiedergegebene string sollte leer sein.
    statusCode = store_get(db, key, result);

    if (statusCode >= 0)
    {
        fprintf(
                stderr,
                "FAILED DEL after PUT and GET: statusCode %d\n",
                statusCode
        );
        return STATUS_TEST_FAIL;
    }

    if (strncmp(result, value, MAX_VALUE_LENGTH) == 0)
    {
        fprintf(
                stderr,
                "FAILED DEL after PUT and GET: GET after DEL %s res: %s",
                value, result
        );
    }

    // löschen funktioniert richtig
    fprintf(stdout, "PASSED DELETE: DEL %s \n", key);

    return STATUS_OK;
}


void string_randomize(char *input, int size)
{
    // schreibe zufällige character in den
    // übergebenen parameter string
    // die zahlen 32 bis 126 sind ganz normale ascii zeichen
    for (int i = 0; i < MAX_VALUE_LENGTH - 1; i++)
    {
        input[i] = (char) (rand() % (126 - 32) + 32);
    }
    input[size - 1] = '\0';
}


int store_test_many_put_get_del(KeyValueStore *db, int numTests)
{
    // teste mit zufälligen strings
    char randomKey[MAX_KEY_LENGTH];
    char randomValue[MAX_VALUE_LENGTH];

    // den pseudozufallsgenerator müssen wir noch
    // vorher mit einer zahl initialisieren
    srand((int) time(NULL));

    for (int t = 0; t < numTests; t++)
    {

        // erstelle zufällige strings
        string_randomize(randomKey, MAX_KEY_LENGTH);
        string_randomize(randomValue, MAX_VALUE_LENGTH);

        // teste damit
        int testStatus = db_test_put_get_del_get(db, randomKey, randomValue);
        if (testStatus < 0)
        { return STATUS_TEST_FAIL; }

    }

    // wenn alle tests funktioniert haben,
    // dann hat auch der hier funktioniert.
    return STATUS_OK;
}
