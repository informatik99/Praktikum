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
            } else {
                dprintf(fileDescriptor,"PUT %s:%s\n",keyBuff,valBuff);
            }
        } else if(sscanf(inputBuff, "DEL %s",keyBuff) == 1){
            status = del(keyBuff);
            if(status < 0){
                dprintf(fileDescriptor, "DEL failed\n");
            } else {
                dprintf(fileDescriptor,"DEL %s\n", keyBuff);
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
    fprintf(stdout,"server store working\n");

    // der server braucht ein socket für die kommunikation
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocket < 0){
        fprintf(stderr,"failed to open socket\n");
        return 3;
    }
    fprintf(stdout,"socket created\n");

    // nach dem Schließen unseres server programms,
    // wäre der socket noch für längere Zeit gebunden
    // das kann nerven, wenn man schnell den server wieder starten möchte
    if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0){
        fprintf(stderr,"setsockopt(SO_REUSEADDR) failed");
        return 4;
    }
    fprintf(stdout,"socket options set\n");


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
    fprintf(stdout,"socket bound for port %d \n", SERVER_PORT);


    // jetzt können wir anfangen auf Anfragen zu hören
    int maxConnections = 5;
    listen(serverSocket, maxConnections);
    fprintf(stdout,"server listening on port %d \n", SERVER_PORT);

    struct sockaddr_in clientAddress;
    unsigned int clientAddressLen = sizeof(clientAddress);

    while(1){
        int clientFileDescriptor = accept(serverSocket,
                                          (struct sockaddr*) &clientAddress,
                                          &clientAddressLen);

        if(clientFileDescriptor < 0){
            fprintf(stderr, "invalid clientFileDescriptor\n");
            break;
        }

        int childPid = fork();
        if(childPid < 0){
            fprintf(stderr, "couldn't fork!\n");
            break;
        }

        if(childPid == 0){
            // child process
            // jetzt kann der user mit dem clientFileDescriptor interagieren
            userInteraction(clientFileDescriptor);
            close(clientFileDescriptor);
            break;
        }

        // parent process
        fprintf(stdout,"child process created, pid: %d\n", childPid);
        close(clientFileDescriptor);
    }




    return 0;
}
