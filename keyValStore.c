#include <string.h>
#include "keyValStore.h"
#include "stdio.h"

#define KEY_VALUE_MAX_LENGTH 50
int inStore =0;                       //Speichert ab, wie viele KeyValues bereits im Store sind!
KeyValue store[KEY_VALUE_MAX_LENGTH]; //Die Struktur muss nicht hier definiert werden, weil sue schon in keyValStore.h ist

int get(char *key, char *res){

    for(int i=0;i< inStore;i++){
        if(strncmp(store[i].key,key,MAX_KEY_LENGTH) == 0){
            strncpy(res,store[i].value, MAX_VALUE_LENGTH);
            return 0;
        }
    }
    return -1;
}

int put(char *key, char *value){

    KeyValue new = {"",""};

    strncpy(new.key,key, MAX_KEY_LENGTH);
    strncpy(new.value,value, MAX_VALUE_LENGTH);

    printf("\nKEY: %s",new.key);
    printf("\nVAL: %s",new.value);
    printf("\nÜbergabe nach PUT hat geklappt");

    /*Geht den Store durch, um zu schauen, ob dieser Key schon einen Wert hat*/
    for(int i = 0;i< inStore;i++){
        if(strncmp(store[i].key,key,MAX_KEY_LENGTH) == 0){
            strncpy(store[i].value, value, MAX_VALUE_LENGTH);
            return 1;
        }
    }

    // der Key ist nicht im Store, jetzt müssen wir noch checken,
    // ob wir genügend Platz haben, sonst bekommen wir vielleicht einen
    // BufferOverflow (IndexOutOfBoundsException)
    if(inStore >= KEY_VALUE_MAX_LENGTH){
        // kein Platz mehr
        return -1;
    }

    //Falls nicht -> wird das neue KeyValue-Objekt hier in den Store getan
    store[inStore++] = new;
    printf("\nNeues Paar in Store gespeichert.");
    printf("\nObjekte im Store: %i",inStore);

    return 0;
}

int del(char *key){
    // falls es keinen eintrag gibt,
    // gib einen fehler zurück
    if(inStore == 0){
        return -1;
    }

    // finde die richtige position im store
    int keyIndex = 0;
    for(keyIndex=0; keyIndex<inStore; keyIndex++){
        if(strcmp(store[keyIndex].key, key) == 0){
            break;
        }
    }

    // falls es den key nicht gibt,
    // gib einen fehler aus
    if(keyIndex >= inStore){
        return -2;
    }

    // tausche den letzten eintrag mit dem
    // des gesuchten keys
    KeyValue tmp = store[keyIndex];
    store[keyIndex] = store[inStore-1];
    store[inStore-1] = tmp;

    // der jetzt letzte eintrag kann
    // gelöscht werden
    inStore--;

    // alles in ordnung
    return 1;
}

void printStore(){
    printf("Im Store sind:");
    for(int i=0; i<inStore;i++){
        printf("\n Key: %s und Wert: %s",store[i].key,store[i].value);
    }
}




