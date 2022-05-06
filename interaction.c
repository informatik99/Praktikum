#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <assert.h>
#include "store.h"
#include "interaction.h"

int client_read_command_loop(
        int fileDescriptor,
        SharedData *db,
        int interestedProcessId,
        int isInsideTransaction);

int client_print_message_loop(
        int fileDescriptor,
        SharedData *sharedServerData,
        int communicationType);

int interaction_process_and_exit(SharedData *sharedServerData, int clientFileDescriptor)
{
    // jede kommunikation und interaktion mit dem client findet in diesem prozess statt.
    // wir wollen wir aber, dass der client sich für keys interessieren kann (subscriben).
    // Hierzu muss er nachrichten an den client weiterreichen können, die auch von anderen prozessen
    // angestoßen werden (die mit anderen clients interagieren)

    // Die Lösung:
    // wir teilen diesen prozess nochmal in 2 Prozesse
    // ---> der eine Prozess liest die kommandos,
    //      die der client eingibt und schreibt die Antwort in eine Warteschlange
    //      mit einer kommunikations-id
    // ---> der andere Prozess liest alles,
    //      was in der Warteschlange ist,
    //      was die gleichen kommunikations-id hat,
    //      und schreibt das zum client.
    // resultat: andere Prozesse können auch in die warteschlange reinschreiben mit der entsprechenden
    //           kommunikations-id und der entsprechende client bekommt die nachricht


    // die zwei prozesse sollen die gleiche kommunkationsId
    // zum Verständigen benutzen
    int interestedProcessId = getpid();

    fprintf(stdout, "client process %d now working\n", interestedProcessId);

    int childProcessId = fork();
    if (childProcessId == 0)
    {
        // dieser prozess soll nur lesen, welche nachrichten
        // mit einer bestimmten kommunkationsId
        // in der nachrichten warteschlange ankommen,
        // und sie zum client schreiben.
        fprintf(
                stdout,
                "client printer processId:%d working with interestedProcessId: %d\n",
                getpid(),
                interestedProcessId
        );

        client_print_message_loop(
                clientFileDescriptor,
                sharedServerData,
                interestedProcessId
        );
    }
    else if (childProcessId > 0)
    {
        // dieser prozess soll lesen, was der client geschrieben hat,
        // das kommando ausführen, und gegebenfalls eine Nachricht,
        // mit der entsprechenden kommunikationsId,
        // in die nachrichtenWarteschlange
        // reintun, sodass der client die Nachricht bekommt.
        fprintf(
                stdout,
                "client interaction worker processId:%d with interestedProcessId: %d\n",
                getpid(), interestedProcessId
        );

        client_read_command_loop(
                clientFileDescriptor,
                sharedServerData,
                interestedProcessId, 0
        );
    }
    else
    {
        fprintf(stderr, "Couldn't fork client worker\n");
    }

    // jeder der beiden prozesse soll warten,
    // bis alle kind prozesse fertig sind
    while (wait(NULL) > 0);

    // wir sagen dem betriebssystem noch,
    // dass wir nicht mehr auf den geteilten
    // speicher zugreifen
    sh_detach(sharedServerData);

    // wir kommunizieren nicht mehr mit dem client,
    // und schließen deswegen den fileDescriptor
    close(clientFileDescriptor);

    fprintf(stdout, "client process %d closed\n", interestedProcessId);
    exit(0);
}


int client_print_message_loop(int fileDescriptor, SharedData *sharedServerData, int communicationType)
{
    // mache nichts anderes, als nachrichten
    // aus der queue zu bekommen
    // und sie dem client zurückzuliefern / zu schreiben.
    // beende bei einer speziellen nachricht
    char message[MAX_MESSAGE_LENGTH];
    while (1)
    {
        sh_receive_as(sharedServerData, communicationType, message, MAX_MESSAGE_LENGTH);
        write(fileDescriptor, message, MAX_MESSAGE_LENGTH);
        if (strncmp(message, "QUIT", 4) == 0)
        {
            break;
        }
    }
    return 1;
}

typedef struct Command
{
    enum
    {
        COMMAND_UNDEFINED = 1,
        COMMAND_QUIT,
        COMMAND_GET,
        COMMAND_PUT,
        COMMAND_DEL,
        COMMAND_BEG,
        COMMAND_END,
        COMMAND_SUB,
        COMMAND_UNSUB
    } type;
    char key[MAX_KEY_LENGTH];
    char value[MAX_VALUE_LENGTH];
} Command;

int command_from_string(Command *command, const char *string)
{
    if (sscanf(string, "PUT %s %s", command->key, command->value) == 2)
    {
        command->type = COMMAND_PUT;
        return COMMAND_PUT;
    }
    if (sscanf(string, "GET %s", command->key) == 1)
    {
        command->type = COMMAND_GET;
        return COMMAND_GET;
    }
    if (sscanf(string, "DEL %s", command->key) == 1)
    {
        command->type = COMMAND_DEL;
        return COMMAND_DEL;
    }
    if (sscanf(string, "SUB %s", command->key) == 1)
    {
        command->type = COMMAND_SUB;
        return COMMAND_SUB;
    }
    if (sscanf(string, "UNSUB %s", command->key) == 1)
    {
        command->type = COMMAND_UNSUB;
        return COMMAND_UNSUB;
    }
    if (strncmp(string, "QUIT", 4) == 0)
    {
        command->type = COMMAND_QUIT;
        return COMMAND_QUIT;
    }
    if (strncmp(string, "BEG", 3) == 0)
    {
        command->type = COMMAND_BEG;
        return COMMAND_BEG;
    }
    if (strncmp(string, "END\n", 3) == 0)
    {
        command->type = COMMAND_END;
        return COMMAND_END;
    }
    memset(command->key, 0, MAX_KEY_LENGTH);
    memset(command->value, 0, MAX_VALUE_LENGTH);
    command->type = COMMAND_UNDEFINED;
    return COMMAND_UNDEFINED;
}


// innerhalb einer transaktion
// darf kein exklusiver zugriff
// auf den store angefordert werden,
// weil wir dadurch einen DEADLOCK bekämen.
int command_execute_inside_transaction(Command *command, SharedData *sharedServerData, int communicationType)
{
    char message[MAX_MESSAGE_LENGTH];
    memset(message, 0, MAX_MESSAGE_LENGTH);
    int status;
    switch (command->type)
    {
    case COMMAND_UNDEFINED:
        snprintf(
                message,
                MAX_MESSAGE_LENGTH,
                "COMMAND UNKNOWN!\r\n"
        );
        sh_send_to(sharedServerData, message, communicationType);
        break;
    case COMMAND_QUIT:
        snprintf(
                message,
                MAX_MESSAGE_LENGTH,
                "You are inside a transaction. Please say END first\r\n"
        );
        sh_send_to(sharedServerData, message, communicationType);
        break;
    case COMMAND_BEG:
        snprintf(
                message,
                MAX_MESSAGE_LENGTH,
                "You are already inside the transaction\r\n"
        );
        sh_send_to(sharedServerData, message, communicationType);
        break;
    case COMMAND_END:
        snprintf(
                message,
                MAX_MESSAGE_LENGTH,
                "TRANSACTION END\r\n"
        );
        sh_send_to(sharedServerData, message, communicationType);
        return 0;
    case COMMAND_GET:
        status = sh_store_race_get(sharedServerData, command->key, command->value);
        if (status < 0)
        {
            snprintf(
                    message, MAX_MESSAGE_LENGTH,
                    "GET:%s:key_non_existent\r\n",
                    command->key
            );
            sh_send_to(sharedServerData, message, communicationType);
        }
        else
        {
            snprintf(
                    message, MAX_MESSAGE_LENGTH,
                    "GET:%s:%s\r\n",
                    command->key, command->value
            );
            sh_send_to(sharedServerData, message, communicationType);
        }
        break;
    case COMMAND_PUT:
        status = sh_store_race_put(sharedServerData, command->key, command->value);
        if (status < 0)
        {
            snprintf(
                    message, MAX_MESSAGE_LENGTH,
                    "PUT FAILED\r\n"
            );
            sh_send_to(sharedServerData, message, communicationType);
        }
        else
        {
            snprintf(
                    message, MAX_MESSAGE_LENGTH,
                    "PUT %s:%s\r\n",
                    command->key, command->value
            );

            sh_send_to(sharedServerData, message, communicationType);
            sh_subs_sync_notify(sharedServerData, command->key, message);
        }
        break;
    case COMMAND_DEL:
        status = sh_store_race_del(sharedServerData, command->key);
        if (status < 0)
        {
            snprintf(
                    message, MAX_MESSAGE_LENGTH,
                    "DEL failed\r\n"
            );
            sh_send_to(sharedServerData, message, communicationType);
        }
        else
        {
            snprintf(
                    message, MAX_MESSAGE_LENGTH,
                    "DEL %s\r\n",
                    command->key
            );
            sh_send_to(sharedServerData, message, communicationType);
            sh_subs_sync_notify(sharedServerData, command->key, message);
        }
        break;
    case COMMAND_SUB:
        status = sh_subs_sync_add(sharedServerData, command->key, communicationType);
        if (status < 0)
        {
            snprintf(
                    message, MAX_MESSAGE_LENGTH,
                    "SUB %s failed\r\n",
                    command->key
            );
            sh_send_to(sharedServerData, message, communicationType);
        }
        else
        {
            snprintf(
                    message, MAX_MESSAGE_LENGTH,
                    "SUB %s OK\r\n",
                    command->key
            );
            sh_send_to(sharedServerData, message, communicationType);
        }
        break;
    case COMMAND_UNSUB:
        status = sh_subs_sync_del(sharedServerData, command->key, communicationType);
        if (status < 0)
        {
            snprintf(
                    message, MAX_MESSAGE_LENGTH,
                    "UNSUB %s FAILED\r\n",
                    command->key
            );
            sh_send_to(sharedServerData, message, communicationType);
        }
        else
        {
            snprintf(
                    message, MAX_MESSAGE_LENGTH,
                    "UNSUB %s OK\r\n",
                    command->key
            );
            sh_send_to(sharedServerData, message, communicationType);
        }
        break;
    }
    return 1;
}

// außerhalb einer transaktion
// müssen wir bei einzelnen store-Befehlen
// exklusiven zugriff anfordern,
// weil wir sonst eine RACE-CONDITION hätten
int command_execute_normal(Command *command,
                           SharedData *sharedServerData,
                           int interestedProcessId,
                           int clientFileDescriptor)
{

    char message[MAX_MESSAGE_LENGTH];
    memset(message, 0, MAX_MESSAGE_LENGTH);
    int status;
    switch (command->type)
    {
    case COMMAND_UNDEFINED:
        snprintf(
                message, MAX_MESSAGE_LENGTH,
                "COMMAND UNKNOWN. what did you mean?\r\n"
        );
        sh_send_to(sharedServerData, message, interestedProcessId);
        break;
    case COMMAND_QUIT:
        snprintf(
                message, MAX_MESSAGE_LENGTH,
                "Thank you for using our service!\r\n"
        );
        sh_subs_sync_del_all_for(sharedServerData, interestedProcessId);
        sh_send_to(sharedServerData, message, interestedProcessId);
        sh_send_to(sharedServerData, "QUIT", interestedProcessId);
        return 0;
    case COMMAND_GET:
        sh_store_lock(sharedServerData);
        status = sh_store_race_get(sharedServerData, command->key, command->value);
        sh_store_unlock(sharedServerData);
        if (status < 0)
        {
            snprintf(
                    message, MAX_MESSAGE_LENGTH,
                    "GET failed\r\n"
            );
            sh_send_to(sharedServerData, message, interestedProcessId);
        }
        else
        {
            snprintf(
                    message, MAX_MESSAGE_LENGTH,
                    "GET:%s:%s\r\n",
                    command->key, command->value
            );
            sh_send_to(sharedServerData, message, interestedProcessId);
        }
        break;
    case COMMAND_PUT:
        sh_store_lock(sharedServerData);
        status = sh_store_race_put(sharedServerData, command->key, command->value);
        sh_store_unlock(sharedServerData);
        if (status < 0)
        {
            snprintf(
                    message, MAX_MESSAGE_LENGTH,
                    "PUT FAILED\r\n"
            );
            sh_send_to(sharedServerData, message, interestedProcessId);
        }
        else
        {
            snprintf(
                    message, MAX_MESSAGE_LENGTH, "PUT %s:%s\r\n",
                    command->key, command->value
            );
            sh_subs_sync_notify(sharedServerData, command->key, message);
            sh_send_to(sharedServerData, message, interestedProcessId);
        }
        break;
    case COMMAND_DEL:
        sh_store_lock(sharedServerData);
        status = sh_store_race_del(sharedServerData, command->key);
        sh_store_unlock(sharedServerData);
        if (status < 0)
        {
            snprintf(
                    message, MAX_MESSAGE_LENGTH,
                    "DEL failed\r\n"
            );
            sh_send_to(sharedServerData, message, interestedProcessId);
        }
        else
        {
            snprintf(
                    message, MAX_MESSAGE_LENGTH,
                    "DEL %s\r\n",
                    command->key
            );
            sh_subs_sync_notify(sharedServerData, command->key, message);
            sh_send_to(sharedServerData, message, interestedProcessId);
        }
        break;
    case COMMAND_BEG:
        sh_store_lock(sharedServerData);
        snprintf(
                message, MAX_MESSAGE_LENGTH,
                "TRANSACTION START\r\n"
        );
        sh_send_to(sharedServerData, message, interestedProcessId);
        client_read_command_loop(
                clientFileDescriptor, sharedServerData,
                interestedProcessId, 1
        );
        sh_store_unlock(sharedServerData);
        break;
    case COMMAND_END:
        snprintf(
                message, MAX_MESSAGE_LENGTH,
                "You are NOT inside a transaction\r\n"
        );
        sh_send_to(sharedServerData, message, interestedProcessId);
        break;
    case COMMAND_SUB:
        status = sh_subs_sync_add(sharedServerData, command->key, interestedProcessId);
        if (status < 0)
        {
            snprintf(
                    message, MAX_MESSAGE_LENGTH,
                    "SUB failed\r\n"
            );
            sh_send_to(sharedServerData, message, interestedProcessId);
        }
        else
        {
            snprintf(
                    message, MAX_MESSAGE_LENGTH,
                    "SUB %s \r\n",
                    command->key
            );
            sh_send_to(sharedServerData, message, interestedProcessId);
        }
        break;
    case COMMAND_UNSUB:
        status = sh_subs_sync_del(sharedServerData, command->key, interestedProcessId);
        if (status < 0)
        {
            snprintf(
                    message, MAX_MESSAGE_LENGTH,
                    "UNSUB %s FAILED\r\n",
                    command->key
            );
            sh_send_to(sharedServerData, message, interestedProcessId);
        }
        else
        {
            snprintf(
                    message, MAX_MESSAGE_LENGTH,
                    "UNSUB %s OK\r\n",
                    command->key
            );
            sh_send_to(sharedServerData, message, interestedProcessId);
        }
        break;
    }
    return 1;
}

// Diese Funktion ist ein wrapper für read,
// weil read nicht sicherstellt, dass man auch eine zeile bekommt.
// Häufig ist das so unter linux telnet,
// der windows telnet client scheint allerdings nicht
// auf eine neue Zeile zu warten bis er sendet...
unsigned int readline(int fileDescriptor, char *outputBuff, unsigned int maxLength)
{
    assert(maxLength >= 3);

    char readBuffer[maxLength];
    unsigned int readCountTotal = 0;
    unsigned int readCount;

    while ((readCount = read(fileDescriptor, readBuffer, maxLength)) > 0)
    {
        if (readCountTotal + readCount < maxLength - 3)
        {
            for (int i = 0; i < readCount; i++)
            {
                outputBuff[readCountTotal + i] = readBuffer[i];
                if (outputBuff[readCountTotal + i] == '\n')
                {
                    readCountTotal += i;
                    outputBuff[readCountTotal] = '\r';
                    outputBuff[readCountTotal + 1] = '\n';
                    outputBuff[readCountTotal + 2] = '\0';
                    return readCountTotal + 3;
                }
            }
            readCountTotal += readCount;
        }
        else
        {
            unsigned int numToWrite = maxLength - readCountTotal;
            for (int i = 0; i < numToWrite; i++)
            {
                outputBuff[readCountTotal + i] = readBuffer[i];
            }
            outputBuff[maxLength - 3] = '\r';
            outputBuff[maxLength - 2] = '\n';
            outputBuff[maxLength - 1] = '\0';
            readCountTotal += numToWrite;
            return readCountTotal;
        }
    }
    return 0;
}


int client_read_command_loop(int fileDescriptor, SharedData *sharedServerData, int interestedProcessId,
                             int isInsideTransaction)
{
    char clientInput[MAX_MESSAGE_LENGTH];

    sh_send_to(
            sharedServerData,
            HGRN "Welcome my dear friend\r\n" COLOR_RESET,
            interestedProcessId
    );

    Command command;
    int canGoOn;
    if (isInsideTransaction)
    {
        while (1)
        {
            // lese eine zeile vom client
            canGoOn = readline(fileDescriptor, clientInput, MAX_MESSAGE_LENGTH) > 0;
            if (!canGoOn)
            { break; }

            // parse die zeile und mach ein kommando daraus
            command_from_string(&command, clientInput);

            // interpretiere das kommando innerhalb einer transaktion
            canGoOn = command_execute_inside_transaction(&command, sharedServerData, interestedProcessId);
            if (!canGoOn)
            { break; }
        }
    }
    else
    {
        while (1)
        {
            // lese eine zeile vom client
            canGoOn = readline(fileDescriptor, clientInput, MAX_MESSAGE_LENGTH) > 0;
            if (!canGoOn)
            { break; }

            // parse die zeile und mach ein kommando daraus
            command_from_string(&command, clientInput);

            // interpretiere das kommando ganz normal
            canGoOn = command_execute_normal(
                    &command,
                    sharedServerData,
                    interestedProcessId,
                    fileDescriptor
            );
            if (!canGoOn)
            { break; }
        }
    }
    return 1;
}
