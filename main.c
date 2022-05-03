#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "keyValStore.h"
#include "userInteraction.h"

#define SERVER_PORT 5678
#define MAX_NUM_OF_CHILDREN 2


int main() {
    KeyValueDatabase *sharedDatabaseHandle;

    // Hole shared memory für unseren Store / Datenbank
    int sharedMemReadWritePrivilege = 0644;   // vergebe lese- und schreibrechte
    int sharedMemId = shmget(IPC_PRIVATE, sizeof(KeyValueDatabase),
                             IPC_CREAT | sharedMemReadWritePrivilege);

    sharedDatabaseHandle = (KeyValueDatabase *) shmat(sharedMemId, 0, 0);
    if (sharedDatabaseHandle == NULL) {
        fprintf(stderr, "couldn't attach shared memory\n");
        return 21;
    }

    // initialisiere die Datenbank und stelle sicher,
    // dass alles richtig funktioniert
    db_init(sharedDatabaseHandle, 1);
    if (!db_test(sharedDatabaseHandle)) {
        fprintf(stderr, "server store not working!\n");
        if (shmdt(sharedDatabaseHandle) < 0) {
            fprintf(stderr, "couldn't detach shared memory\n");
        }
        shmctl(sharedMemId, IPC_RMID, 0);
        return 1;
    };

    fprintf(stdout, "server store working\n");


    // der server braucht ein socket für die kommunikation
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        fprintf(stderr, "failed to open socket\n");
        return 3;
    }
    fprintf(stdout, "socket created\n");

    // nach dem Schließen unseres server programms,
    // wäre der socket noch für längere Zeit gebunden
    // das kann nerven, wenn man schnell den server wieder starten möchte
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,
                   &(int) {1}, sizeof(int)) < 0) {
        fprintf(stderr, "setsockopt(SO_REUSEADDR) failed");
        return 4;
    }
    fprintf(stdout, "socket options set\n");


    // wir wollen eine TCP Verbindung mit unserem eigenen Port
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    // die server port nummer muss in network byte order übergeben werden
    serverAddress.sin_port = htons(SERVER_PORT);

    // jetzt können wir unseren socket mit unseren daten binden
    const int bindingStatus = bind(serverSocket,
                                   (struct sockaddr *) &serverAddress,
                                   sizeof(serverAddress));
    // falls das nicht klappt...
    if (bindingStatus < 0) {
        fprintf(stderr, "couldn't bind socket\n");
        return 4;
    }
    fprintf(stdout, "socket bound for port %d \n", SERVER_PORT);


    // jetzt können wir anfangen auf Anfragen zu hören
    int maxConnections = 5;
    listen(serverSocket, maxConnections);
    fprintf(stdout, "server listening on port %d \n", SERVER_PORT);

    struct sockaddr_in clientAddress;
    unsigned int clientAddressLen = sizeof(clientAddress);

    int pids[MAX_NUM_OF_CHILDREN];
    int pidCount = 0;

    while (1) {
        int clientFileDescriptor = accept(serverSocket,
                                          (struct sockaddr *) &clientAddress,
                                          &clientAddressLen);
        if (clientFileDescriptor < 0) {
            fprintf(stderr, "invalid clientFileDescriptor\n");
            break;
        }
        int childPid = fork();
        if (childPid < 0) {
            fprintf(stderr, "couldn't fork!\n");
            break;
        } else if (childPid == 0) {
            // kind prozess
            // jetzt kann der user mit der Datenbank
            // interagieren
            childPid = getpid();
            if (pidCount >= MAX_NUM_OF_CHILDREN) {
                dprintf(clientFileDescriptor, "Sorry: too many connections\n");
                user_show_unavailable(clientFileDescriptor);
                fprintf(stderr, "child %d refused\n", childPid);
                close(clientFileDescriptor);
                exit(0);
            } else {
                fprintf(stdout, "child %d now working\n", childPid);
                user_interact(clientFileDescriptor, sharedDatabaseHandle);
                if (shmdt(sharedDatabaseHandle) < 0) {
                    fprintf(stderr, "child %d couldn't detach from shared memory\n", childPid);
                } else {
                    fprintf(stdout, "child %d shared memory detached\n", childPid);
                }
                close(clientFileDescriptor);
                fprintf(stdout, "child %d closed\n", childPid);
                exit(0);
            }
        } else {
            // eltern prozess
            // der eltern prozess braucht gerade keine interaktion mehr mit dem client
            // aber speichert vorsichtshalber die kind pid ab
            fprintf(stdout, "child process created, pid: %d\n", childPid);
            if (pidCount < MAX_NUM_OF_CHILDREN) {
                pids[pidCount] = childPid;
                pidCount++;
                close(clientFileDescriptor);
                continue;
            } else {
                close(clientFileDescriptor);
                break;
            }
        }
    }

    // eltern prozess
    // stelle sicher, dass alle clients fertig sind
    printf("waiting for child processes to finish\n");
    for (int i = 0; i < pidCount; i++) {
        printf("wait for child %d\n", pids[i]);
        waitpid(pids[i], NULL, 0);
        printf("child %d closed\n", pids[i]);
    }
    printf("all %d childProcesses closed.\n", pidCount);

    db_print(sharedDatabaseHandle);
    db_free(sharedDatabaseHandle);

    if (shmdt(sharedDatabaseHandle) < 0) {
        fprintf(stderr, "coudn't detach shared memory\n");
    } else {
        fprintf(stdout, "server parent shared memory detached\n");
    }

    if (shmctl(sharedMemId, IPC_RMID, 0) < 0) {
        fprintf(stderr, "couldn't destroy shared memory\n");
    } else {
        fprintf(stdout, "shared memory destroyed\n");
    }

    printf("exit server\n");
    return 0;
}
