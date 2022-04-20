#include <stdio.h>
#include <netinet/in.h>
#include "keyValStore.h"
#include "string.h"

int main() {
char key[50] = "EINS";
char value[50] = "ICH";
    char key2[50] = "ZWEI";
    char value2[50] = "DU";

    put(key2,value2);
    char back[50];
    get(key2,back);

    printf("\n Zurückgekommen: %s",back);

    //falschen Löschen
    if(del("EINS") == 0){
        printf("\n falscher Gelöscht");
    }
    if(del("ZWEI") == 1){
        printf("\n richtiger Gelöscht");
    }

    printStore();
    return 0;
}
