#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "shared.h"
#include "interaction.h"

int socket_tcp_ipv4_create_and_bind(int port)
{

    // der server braucht ein socket für die kommunikation.
    // den soll das Betriebssystem für uns bereithalten
    // und wir wollen mithilfe eines schlüssels/keys
    // aktionen mit diesem socket durchführen
    int serverSocketKey = socket(
            AF_INET,   // ipv4 adressen
            SOCK_STREAM, // verbindungsorientiert (tcp)
            0         // standard protokoll
    );

    if (serverSocketKey < 0)
    {
        fprintf(stderr, "failed to open socket\n");
        exit(3);
    }

    fprintf(stdout, "socket created\n");

    // nach dem Schließen unseres server programms,
    // wäre der socket noch für längere Zeit gebunden
    // das kann nerven, wenn man schnell den server wieder starten möchte
    if (setsockopt(
            serverSocketKey,     // <-- socket für den wir die option setzen wollen
            SOL_SOCKET,        // <-- level auf dem die option gesetzt wird
            SO_REUSEADDR,   //  wir wollen den port und so wiederverwenden
            &(int) {1},       //  also setzen wir die option auf 1
            sizeof(int)       //  wir müssen noch die größe angeben, die die option hat, die wir setzen
    ) < 0)
    {
        fprintf(stderr, "setsockopt(SO_REUSEADDR) failed");
        exit(4);
    }

    fprintf(stdout, "socket options set\n");

    // wir wollen eine TCP Verbindung mit unserem eigenen Port
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;            // wir wollen ipv4
    serverAddress.sin_addr.s_addr = INADDR_ANY;    // wir erlauben jede beliebige ip adresse, die unser computer hat
    serverAddress.sin_port = htons(port);  // die server port nummer wird in network byte order übergeben

    // jetzt können wir unseren socket mit unseren daten binden
    const int bindingStatus = bind(
            serverSocketKey,
            (struct sockaddr *) &serverAddress,
            sizeof(serverAddress));
    if (bindingStatus < 0)
    {
        fprintf(stderr, "couldn't bind socket\n");
        exit(5);
    }

    fprintf(stdout, "socket bound for port %d \n", port);

    return serverSocketKey;
}


int main()
{
    int serverSocketId = socket_tcp_ipv4_create_and_bind(SERVER_PORT);

    // bald wird dieser prozess
    // einige male geforkt / kopiert und wir wollen
    // bei all diesen prozessen speicher haben,
    // der zwischen allen prozessen geteilt wird
    SharedData sharedData;
    sh_init(&sharedData);

    // jetzt können wir anfangen
    // auf Anfragen zu hören
    int maxConnections = 5;
    listen(serverSocketId, maxConnections);

    fprintf(stdout, "server listening on port %d \n", SERVER_PORT);

    // die address-struktur für den client, wird ausgefüllt (überschrieben)
    // sobald wir einen client akzeptieren
    struct sockaddr_in clientAddress;
    unsigned int clientAddressLen = sizeof(clientAddress);

    // theoretisch könnte unser server
    // für immer laufen...
    // aber wir sagen jetzt einfach mal,
    // dass wir den server beenden,
    // wenn sich so und so viele clients,
    // über die Zeit hinweg,
    // verbunden haben
    int clientConnectionCount = 0;
    while (clientConnectionCount < MAX_NUM_OF_CLIENT_CONNECTIONS)
    {

        // der server wartet auf einen neuen client
        // der mit ihm kommunizieren will,
        // wir bekommen einen fileDescriptor zurück,
        // also eine zahl, die auf eine "datei" verweist,
        // von der man lesen, und auf die man schreiben kann.
        // nur ist es in dem Fall eine "datei" mit der man
        // mit dem client kommunizieren kann
        int clientFileDescriptor = accept(
                serverSocketId,
                (struct sockaddr *) &clientAddress,
                &clientAddressLen
        );
        if (clientFileDescriptor < 0)
        {
            fprintf(stderr, "invalid clientFileDescriptor\n");
            break;
        }

        clientConnectionCount++;

        // ein client hat sich mit dem server verbunden
        // aber ein Kind prozess soll sich um ihn kümmern,
        // während dieser prozess (der, der gerade ausgeführt wird),
        // nur die Aufgabe hat, weitere clients zu akzeptieren
        int childPid = fork();
        if (childPid == 0)
        {
            interaction_process_and_exit(&sharedData, clientFileDescriptor);
        }
        else if (childPid < 0)
        {
            fprintf(stderr, "couldn't fork!\n");
            break;
        }

        // der eltern prozess braucht keine interaktion
        // mit dem client und sollte deswegen den fileDescriptor schließen.
        // Wenn nicht, dann bleibt die Verbindung noch bestehen,
        // auch wenn der Kind-prozess beendet wurde, und der client eigentlich
        // fertig wäre mit dem kommunizieren. Der client würde dann nur so da hängen,
        // und frustriert sein, dass er nicht raus kann, obwohl er den
        // quit befehlt eingegeben hat.
        close(clientFileDescriptor);

        fprintf(stdout, "child process created, processId: %d\n", childPid);
    }

    // der eltern prozess kann jetzt beendet werden
    // stelle sicher, dass alle clients fertig sind
    printf("waiting for all child processes to finish\n");
    while (wait(NULL) > 0);
    printf("all processes finished\n");

    // zum debuggen geben wir einfach mal aus,
    // was in der datenbank ist
    sh_race_print(&sharedData);

    // zu guter letzt können wir
    // alle angeforderten
    // ressourcen freigeben
    sh_free(&sharedData);

    printf("exit server\n");
    return 0;
}
