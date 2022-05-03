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
#define MAX_NUM_OF_CHILDREN 3

int main() {

    // unsere Datenbank wird im shared memory angelegt
    // damit mehrere Prozesse auf die gleiche Datenbank
    // zugreifen können
    KeyValueDatabase *sharedDatabaseHandle;

    // erstelle shared memory für unsere Datenbank
    int sharedMemReadWritePrivilege = 0644;
    int sharedMemId = shmget(IPC_PRIVATE,
                             sizeof(KeyValueDatabase),
                             IPC_CREAT | sharedMemReadWritePrivilege);

    // bekomme die adresse für shared memory
    sharedDatabaseHandle = (KeyValueDatabase *) shmat(
            sharedMemId, 0, 0);

    if (sharedDatabaseHandle == NULL) {
        fprintf(stderr,
                "couldn't attach shared memory\n");
        exit(1);
    }

    // initialisiere die Datenbank und stelle sicher,
    // dass alles richtig funktioniert
    db_init(sharedDatabaseHandle, 1);

    if (db_test(sharedDatabaseHandle) == DB_FAIL_TEST) {

        // falls die Datenbank nicht funktioniert,
        // kann das shared memory auch gleich wieder freigegeben werden,
        // weil der server sowieso schließen soll
        fprintf(stderr, "server store not working!\n");

        if (shmdt(sharedDatabaseHandle) < 0) {
            fprintf(stderr,
                    "couldn't detach shared memory\n");
        }

        // lösche das shared memory
        shmctl(sharedMemId, IPC_RMID, 0);
        exit(2);
    }

    fprintf(stdout, "server store working\n");

    // der server braucht ein socket für die kommunikation
    // ipv4 und tcp wollen wir haben
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
    // die server port nummer muss in network byte order übergeben werden
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(SERVER_PORT);

    // jetzt können wir unseren socket mit unseren daten binden
    const int bindingStatus = bind(serverSocket,
                                   (struct sockaddr *) &serverAddress,
                                   sizeof(serverAddress));
    if (bindingStatus < 0) {
        fprintf(stderr, "couldn't bind socket\n");
        return 4;
    }

    fprintf(stdout, "socket bound for port %d \n", SERVER_PORT);

    // jetzt können wir anfangen auf Anfragen zu hören
    int maxConnections = 5;
    listen(serverSocket, maxConnections);

    fprintf(stdout, "server listening on port %d \n", SERVER_PORT);

    // unser server akzeptiert clients
    // und forkt für jeden client einen neuen prozess
    // in pids speichert der server alle prozess ids für die clients ab
    int pids[MAX_NUM_OF_CHILDREN];
    int pidCount = 0;

    struct sockaddr_in clientAddress;
    unsigned int clientAddressLen = sizeof(clientAddress);

    // theoretisch könnte unser server
    // für immer laufen...
    while (1) {

        // der server wartet auf einen neuen client
        int clientFileDescriptor = accept(serverSocket,
                                          (struct sockaddr *) &clientAddress,
                                          &clientAddressLen);
        if (clientFileDescriptor < 0) {
            fprintf(stderr, "invalid clientFileDescriptor\n");
            break;
        }

        // ein client hat sich verbunden
        // aber ein Kind prozess soll sich um ihn kümmern,
        // währen dieser prozess hier, dabei ist weitere clients
        // zu akzeptieren
        int childPid = fork();

        if (childPid == 0) {

            // hier beginnt der kind prozess
            // jetzt kann der user mit der Datenbank
            // interagieren
            childPid = getpid();

            if (pidCount >= MAX_NUM_OF_CHILDREN) {

                // falls wir zu viele prozesse hatten,
                // dann soll für den client nur eine nachricht zurückgegeben
                // werden, aber keine Interaktion gestartet werden
                dprintf(clientFileDescriptor, "Sorry: too many connections\n");
                user_show_unavailable(clientFileDescriptor);
                fprintf(stderr, "child %d refused\n", childPid);

            } else {

                // der client kann jetzt mit
                // unserer datenbank interagieren
                fprintf(stdout, "child %d now working\n", childPid);
                user_interact(clientFileDescriptor, sharedDatabaseHandle);
                fprintf(stdout, "child %d closed\n", childPid);

            }

            // der kind prozess kann vor dem Schließen
            // seinen zeiger zur datenbank ausklinken
            if (shmdt(sharedDatabaseHandle) < 0) {
                fprintf(stderr,
                        "child %d couldn't detach from shared memory\n",
                        childPid);
            } else {
                fprintf(stdout,
                        "child %d shared memory detached\n",
                        childPid);
            }

            // beende den kind prozess
            close(clientFileDescriptor);
            exit(0);

        } else if(childPid > 0) {

            // eltern prozess
            // der eltern prozess braucht keine interaktion mehr mit dem client
            // aber speichert vorsichtshalber die kind pids ab,
            // denn bevor er schließt muss er noch auf alle kind prozesse warten
            fprintf(stdout, "child process created, pid: %d\n", childPid);

            if (pidCount < MAX_NUM_OF_CHILDREN) {

                // alles in ordnung,
                // der eltern prozess des servers
                // kann weiter clients akzeptieren
                pids[pidCount] = childPid;
                pidCount++;
                close(clientFileDescriptor);
                continue;

            } else {

                // zu viele clients haben sich über die
                // Zeit hinweg verbunden....
                // deswegen kann der server aufhören
                // weitere clients zu akzeptieren
                close(clientFileDescriptor);
                break;
            }

        } else if (childPid < 0) {
            // dieser fall, dürfte eigentlich
            // nicht auftreten....
            fprintf(stderr, "couldn't fork!\n");
            break;
        }
    }

    // der eltern prozess kann jetzt beendet werden
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

    // zu guter letzt kann unser Zeiger vom
    // shared memory wieder ausgehangen werden,
    if (shmdt(sharedDatabaseHandle) < 0) {
        fprintf(stderr,
                "coudn't detach shared memory\n");
    } else {
        fprintf(stdout,
                "server parent shared memory detached\n");
    }

    // und das shared memory kann gelöscht werden
    if (shmctl(sharedMemId, IPC_RMID, 0) < 0) {
        fprintf(stderr,
                "couldn't destroy shared memory\n");
    } else {
        fprintf(stdout,
                "shared memory destroyed\n");
    }

    printf("exit server\n");
    return 0;
}
