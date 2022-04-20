#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include "keyValStore.h"
#include "string.h"
#include "assert.h"


void testStoreDel(){
    del("Blub");
    del("Eins");
    del("Noch irgendwas");

    put("Eins", "hallo");
    put("Noch irgendwas", "bla");

    assert(del("Eins") == 1);
    assert(del("Noch irgendwas") == 1);
    assert(del("Blub") < 0); // das sollte schief gehen
}

void testStoreSimplePutGet(){
    char key[MAX_KEY_LENGTH] = "name";
    char val[MAX_VALUE_LENGTH] = "robert";

    put(key,val);

    char res[MAX_VALUE_LENGTH];
    get(key, res);
    assert(strcmp(val, res) == 0);
}

void testStore(){
    testStoreDel();
    testStoreSimplePutGet();
}

void userInteraction(int fileDescriptor){
    int inputBuffSize = MAX_KEY_LENGTH + MAX_VALUE_LENGTH;
    char inputBuff[inputBuffSize];

    char keyBuff[MAX_KEY_LENGTH] = "";
    char valBuff[MAX_VALUE_LENGTH] = "";
    int status = 0;

    while(read(fileDescriptor,inputBuff,inputBuffSize) > 0){
        if(strncmp(inputBuff,"QUIT",4) == 0){
            break;
        } else if(sscanf(inputBuff,"PUT %s %s",keyBuff,valBuff) == 2){
            status = put(keyBuff,valBuff);
            if(status < 0){
                dprintf(fileDescriptor,"PUT FAILED\n");
            }
        } else if(sscanf(inputBuff, "DEL %s",keyBuff) == 1){
            status = del(keyBuff);
            if(status < 0){
                dprintf(fileDescriptor, "DEL failed\n");
            }
        } else if(sscanf(inputBuff, "GET %s", keyBuff)){
            status = get(keyBuff, valBuff);
            if(status < 0 ){
                dprintf(fileDescriptor, "GET failed\n");
            } else {
                dprintf(fileDescriptor,"GET:%s:%s\n" ,keyBuff,valBuff);
            }
        }
    }
}

int main() {
    testStore();
    userInteraction(STDOUT_FILENO);
    return 0;
}
