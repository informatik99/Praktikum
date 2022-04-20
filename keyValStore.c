#include <string.h>
#include "keyValStore.h"
#include "stdio.h"

#define KEY_VALUE_MAX_LENGTH 50
int inStore =0;                       //Speichert ab, wie viele KeyValues bereits im Store sind!
KeyValue store[KEY_VALUE_MAX_LENGTH]; //Die Struktur muss nicht hier definiert werden, weil sue schon in keyValStore.h ist

int get(char *key, char *res){

    for(int i=0;i< inStore;i++){
        if(strcmp(store[i].key,key) == 0){
            strcpy(res,store[i].value);
            return 0;
        }
    }
    return -1;
}

int put(char *key, char *value){

    KeyValue new = {"",""};

    strcpy(new.key,key);
    strcpy(new.value,value);

    printf("\nKEY: %s",new.key);
    printf("\nVAL: %s",new.value);
    printf("\nÜbergabe nach PUT hat geklappt");

    /*Geht den Store durch, um zu schauen, ob dieser Key schon einen Wert hat*/
    for(int i = 0;i< inStore;i++){
        if(strncmp(store[i].key,key,MAX_KEY_LENGTH) == 0){
            strcpy(store[i].value, value);
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

    //Gibt an, ob des gesuchte Key gefunden wurde und dass man dann die Werte zurückkopiert.
    int found = 0;

    for(int i=0;i<inStore; i++){


        //Gefunden
        if(strcmp(store[i].key,key) == 0 || found == 1) {

            //wenn i+1=inStore ist, gibt es keine weiteren Einträge.
            //Da wir im else kopieren, muss der letzte Storeeintrag auch überschrieben werden, weil es
            //sonst einen KeyValue 2 mal gibt.
            if (i + 1 == inStore) {
                strcpy(store[i].key, "\0");
                strcpy(store[i].value, "\0");

                inStore--;
                printf("\nObjekte im Store: %i",inStore);
                return 1;

            } else
                store[i] = store[i + 1];
                found = 1;
            }

        }
    printf("\nObjekte im Store: %i",inStore);

    return 0;
    }

    void testarray(){
        printf("Im Store sind:");
    for(int i=0; i<inStore;i++){
        printf("\n Key: %s und Wert: %s",store[i].key,store[i].value);
    }
}




