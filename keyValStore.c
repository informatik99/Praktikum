#include <string.h>
#include "keyValStore.h"
#include "stdio.h"

KeyValueDatabase globalDb = {0, {0}};

int db_init(KeyValueDatabase *db){
    db->inStore = 0;
    return 1;
}

int db_free(KeyValueDatabase *db){
    db->inStore = 0;
    return 1;
}

int db_put(KeyValueDatabase *db, const char *key, const char *value){
    KeyValue new = {"",""};

    strncpy(new.key,key, MAX_KEY_LENGTH);
    strncpy(new.value,value, MAX_VALUE_LENGTH);

    /* Geht den Store durch, um zu schauen, ob dieser Key schon einen Wert hat */
    for(int i = 0;i< db->inStore;i++){
        if(strncmp(db->store[i].key,key,MAX_KEY_LENGTH) == 0){
            strncpy(db->store[i].value, value, MAX_VALUE_LENGTH);
            return 1;
        }
    }

    // der Key ist nicht im Store, jetzt müssen wir noch checken,
    // ob wir genügend Platz haben, sonst bekommen wir vielleicht einen
    // BufferOverflow (IndexOutOfBoundsException)
    if(db->inStore >= KEY_VALUE_MAX_LENGTH){
        // kein Platz mehr
        return -1;
    }

    // falls nicht, wird das neue KeyValue-Objekt hier in den Store getan
    db->store[db->inStore++] = new;
    return 0;
}

int db_get(KeyValueDatabase *db, const char *key, char *result){
    for(int i=0;i< db->inStore;i++){
        if(strncmp(db->store[i].key,key,MAX_KEY_LENGTH) == 0){
            strncpy(result,db->store[i].value, MAX_VALUE_LENGTH);
            return 0;
        }
    }
    return -1;
}

int db_del(KeyValueDatabase *db, const char *key){
    // falls es keinen eintrag gibt,
    // gib einen fehler zurück
    if(db->inStore == 0){
        return -1;
    }

    // finde die richtige position im store
    int keyIndex = 0;
    for(keyIndex=0; keyIndex<db->inStore; keyIndex++){
        if(strcmp(db->store[keyIndex].key, key) == 0){
            break;
        }
    }

    // falls es den key nicht gibt,
    // gib einen fehler aus
    if(keyIndex >= db->inStore){
        return -2;
    }

    // tausche den letzten eintrag mit dem
    // des gesuchten keys
    KeyValue tmp = db->store[keyIndex];
    db->store[keyIndex] = db->store[db->inStore-1];
    db->store[db->inStore-1] = tmp;

    // der jetzt letzte eintrag kann
    // gelöscht werden
    db->inStore--;

    // alles in ordnung
    return 1;
}



int db_print(KeyValueDatabase *db){
    printf("Im Store sind:");
    for(int i=0; i<db->inStore;i++){
        printf("\n Key: %s und Wert: %s",db->store[i].key, db->store[i].value);
    }
    return 1;
}

int get(char *key, char *res){
    return db_get(&globalDb,key,res);
}

int put(char *key, char *value){
    return db_put(&globalDb,key,value);
}

int del(char *key){
    return db_del(&globalDb, key);
}

int printStore(){
    return db_print(&globalDb);
}




