#include <stdio.h>
#include <netinet/in.h>
#include "keyValStore.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>

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


#define SERVER_PORT 5678

int main() {
    testStore();

    // der server braucht ein socket für die kommunikation
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocket < 0){
        fprintf(stderr,"failed to open socket\n");
        return 3;
    }

    // nach dem Schließen unseres server programms,
    // wäre der socket noch für längere Zeit gebunden
    // das kann nerven, wenn man schnell den server wieder starten möchte
    if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0){
        fprintf(stderr,"setsockopt(SO_REUSEADDR) failed");
        return 4;
    }

    // wir wollen eine TCP Verbindung mit unserem eigenen Port
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    // die server port nummer muss in network byte order übergeben werden
    serverAddress.sin_port = htons(SERVER_PORT);

    // jetzt können wir unseren socket mit unseren daten binden
    const int bindingStatus = bind(serverSocket, (struct sockaddr*) &serverAddress,
                                   sizeof (serverAddress));
    // falls das nicht klappt...
    if(bindingStatus < 0){
        fprintf(stderr, "couldn't bind socket\n");
        return 4;
    }


    // jetzt können wir anfangen auf Anfragen zu hören
    int maxConnections = 5;
    listen(serverSocket, maxConnections);

    // gerade akzeptieren wir nur ein client
    struct sockaddr_in clientAddress;
    unsigned int clientAddressLen = sizeof(clientAddress);
    int clientFileDescriptor = accept(serverSocket,
                                      (struct sockaddr*) &clientAddress,
                                      &clientAddressLen);

    // jetzt kann der user damit interagieren
    userInteraction(clientFileDescriptor);

    close(clientFileDescriptor);
    return 0;
}
