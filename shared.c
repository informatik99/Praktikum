#include <sys/shm.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <string.h>
#include <assert.h>
#include "shared.h"


///////////// private section ///////////////
int sem_down(int semaphoreId)
{
    struct sembuf semBuf;
    semBuf.sem_flg = SEM_UNDO;
    semBuf.sem_num = 0;
    semBuf.sem_op = -1;
    semop(semaphoreId, &semBuf, 1);
    return 1;
}

int sem_up(int semaphoreId)
{
    struct sembuf semBuf;
    semBuf.sem_flg = SEM_UNDO;
    semBuf.sem_num = 0;
    semBuf.sem_op = 1;
    semop(semaphoreId, &semBuf, 1);
    return 1;
}

struct Message
{
    long type;
    char message[MAX_MESSAGE_LENGTH];
};

int message_queue_send(int queueId, int pid, const char *input)
{
    struct Message message;
    message.type = pid;
    strncpy(message.message, input, MAX_MESSAGE_LENGTH);
    message.message[MAX_MESSAGE_LENGTH - 1] = 0;
    return msgsnd(queueId, &message, MAX_MESSAGE_LENGTH, 0);
}

long message_queue_receive(int queueId, int messageType, char *output, int maxLength)
{
    assert(output != NULL);
    assert(maxLength > 0);
    long status;
    struct Message message;
    status = msgrcv(queueId, &message, MAX_MESSAGE_LENGTH, messageType, 0);
    maxLength = maxLength < MAX_MESSAGE_LENGTH ? maxLength : MAX_MESSAGE_LENGTH;
    strncpy(output, message.message, maxLength);
    output[maxLength - 1] = '\0';
    return status;
}


///////// public section /////////////////////////

int sh_init(SharedData *sharedServerData)
{

    // erstelle shared memory für unseren store
    // und auch für eine subscription liste
    int sharedMemReadWritePrivilege = 0644;
    int sharedMemStoreId = shmget(IPC_PRIVATE,
                                  sizeof(KeyValueStore),
                                  IPC_CREAT | sharedMemReadWritePrivilege
    );
    int sharedMemSubsId = shmget(IPC_PRIVATE,
                                 sizeof(SubscriptionList),
                                 IPC_CREAT | sharedMemReadWritePrivilege
    );

    // die keys müssen wir abspeichern,
    // damit wir später die shared memory bereiche ganz zum schluss
    // wieder löschen können
    sharedServerData->sharedMemStoreKey = sharedMemStoreId;
    sharedServerData->sharedMemSubKey = sharedMemSubsId;


    // jetzt wollen wir die pointer auf
    // den shared memory bereich bekommen
    sharedServerData->store = (KeyValueStore *) shmat(sharedMemStoreId, 0, 0);
    sharedServerData->subscriptions = (SubscriptionList *) shmat(sharedMemSubsId, 0, 0);

    if (sharedServerData->store == NULL)
    {
        fprintf(stderr, "couldn't attach shared memory for store\n");
    }
    if (sharedServerData->subscriptions == NULL)
    {
        fprintf(stderr, "couldn't attach shared memory for subscriptions\n");
    }

    // wir brauchen auch die möglichkeit exklusiven zugriff
    // auf die jeweiligen shared memory bereiche zu bekommen,
    // deswegen verwalten wir dafür binäre semaphoren
    int initialSemaphoreValue = 1;
    sharedServerData->subsSemaphoreId = semget(IPC_PRIVATE, 1, IPC_CREAT | sharedMemReadWritePrivilege);
    sharedServerData->storeSemaphoreId = semget(IPC_PRIVATE, 1, IPC_CREAT | sharedMemReadWritePrivilege);

    if (sharedServerData->subsSemaphoreId < 0)
    {
        fprintf(stderr, "The subscription semaphore couldn't be created\n");
    }
    if (sharedServerData->storeSemaphoreId < 0)
    {
        fprintf(stderr, "The store semaphore couldn't be created\n");
    }

    // binäre semaphoren sollten auf eins gesetzt werden
    semctl(sharedServerData->subsSemaphoreId, 1, SETALL, &initialSemaphoreValue);
    semctl(sharedServerData->storeSemaphoreId, 1, SETALL, &initialSemaphoreValue);


    // wir wollen auch eine message queue haben.
    // sie ist da, damit wir interessierten prozessen eine nachricht zukommen lassen können,
    // wenn wir für die subscriptionListe notify aufrufen
    sharedServerData->queueId = msgget(IPC_PRIVATE, IPC_CREAT | sharedMemReadWritePrivilege);


    // initialisiere jetzt die speicherbereiche
    store_init(sharedServerData->store);
    sub_init(sharedServerData->subscriptions);

    // mache noch ein paar tests, und schau, ob alles klappt
    store_test_many_put_get_del(sharedServerData->store, 200);
    return STATUS_OK;
}


int sh_detach(SharedData *sharedServerData)
{

    if (shmdt(sharedServerData->store) < 0)
    {
        fprintf(stderr, "couldn't detach shared memory for store\n");
    }
    else
    {
        fprintf(stdout, "server parent shared memory for store detached\n");
    }

    if (shmdt(sharedServerData->subscriptions) < 0)
    {
        fprintf(stderr, "couldn't detach shared memory for subscriptions\n");
    }
    else
    {
        fprintf(stdout, "server parent shared memory for subscriptions detached\n");
    }

    return STATUS_OK;
}

int sh_free(SharedData *sharedServerData)
{

    sub_free(sharedServerData->subscriptions);
    store_free(sharedServerData->store);

    printf("data uninitialized\n");

    sh_detach(sharedServerData);

    printf("data detached\n");

    // die semaphoren brauchen wir nicht mehr
    semctl(sharedServerData->subsSemaphoreId, 0, IPC_RMID);
    semctl(sharedServerData->storeSemaphoreId, 0, IPC_RMID);

    printf("semaphores destroyed\n");

    // die queue brauchen wir nicht mehr
    msgctl(sharedServerData->queueId, IPC_RMID, 0);

    printf("queue destroyed\n");


    // und das shared memory kann gelöscht werden
    if (shmctl(sharedServerData->sharedMemStoreKey, IPC_RMID, 0) < 0)
    {
        fprintf(stderr, "couldn't destroy shared memory for store\n");
    }
    else
    {
        fprintf(stdout, "shared memory destroyed for store\n");
    }

    if (shmctl(sharedServerData->sharedMemSubKey, IPC_RMID, 0) < 0)
    {
        fprintf(stderr, "couldn't destroy shared memory for subscriptions\n");
    }
    else
    {
        fprintf(stdout, "shared memory destroyed for subscriptions\n");
    }

    return STATUS_OK;
}


void sh_race_print(SharedData *sharedServerData)
{
    printf("Shared Subscriptions:\n");
    sem_down(sharedServerData->subsSemaphoreId);
    sub_print(sharedServerData->subscriptions);
    sem_up(sharedServerData->subsSemaphoreId);

    printf("Shared Store: \n");
    store_print(sharedServerData->store);
    printf("\n");
}

int sh_send_to(SharedData *sharedServerData, const char *message, int pid)
{
    int status = message_queue_send(sharedServerData->queueId, pid, message);
    if (status < 0)
    {
        printf("send failed\n");
    }
    return status;
}

int sh_receive_as(SharedData *sharedServerData, int pid, char *message, int maxLen)
{
    long status = message_queue_receive(sharedServerData->queueId, pid, message, maxLen);
    if (status < 0)
    {
        printf("receive for failed\n");
        return STATUS_FAILED;
    }
    return STATUS_OK;
}


int sh_store_race_put(SharedData *sharedServerData, const char *key, const char *value)
{
    int status = store_put(sharedServerData->store, key, value);
    if (status < 0)
    {
        printf("put failed\n");
    }
    return status;
}

int sh_store_race_get(SharedData *sharedServerData, const char *key, char *value)
{
    int status = store_get(sharedServerData->store, key, value);
    if (status < 0)
    {
        printf("get failed\n");
    }
    return status;
}

int sh_store_race_del(SharedData *sharedServerData, const char *key)
{
    int status = store_del(sharedServerData->store, key);
    if (status < 0)
    {
        printf("del failed\n");
    }
    return status;
}

int sh_subs_sync_notify(SharedData *sharedServerData, const char *key, const char *message)
{

    int status = STATUS_OK;
    sem_down(sharedServerData->subsSemaphoreId);

    // sende die message an alle prozesse,
    // die an dem (als parameter angegebenen) key interessiert sind
    for (int i = 0; i < sharedServerData->subscriptions->subLen; i++)
    {
        if (strncmp(sharedServerData->subscriptions->subs[i].key, key, MAX_KEY_LENGTH) == 0)
        {
            status = message_queue_send(
                    sharedServerData->queueId,
                    sharedServerData->subscriptions->subs[i].processId,
                    message
            );
        }
    }
    sem_up(sharedServerData->subsSemaphoreId);
    if (status < 0)
    {
        perror("notify failed\n");
        return STATUS_FAILED;
    }
    return status;
}

int sh_subs_sync_add(SharedData *sharedServerData, const char *key, int communicationId)
{
    sem_down(sharedServerData->subsSemaphoreId);
    int status = sub_add(sharedServerData->subscriptions, communicationId, key);
    if (status < 0)
    {
        printf("sub add failed\n");
    }
    sem_up(sharedServerData->subsSemaphoreId);
    return status;
}

int sh_subs_sync_del(SharedData *sharedServerData, const char *key, int pid)
{

    sem_down(sharedServerData->subsSemaphoreId);

    int status = sub_del(sharedServerData->subscriptions, pid, key);
    if (status < 0)
    {
        printf(" unsub failed\n");
    }

    sem_up(sharedServerData->subsSemaphoreId);
    return status;
}

int sh_subs_sync_del_all_for(SharedData *sharedServerData, int communicationId)
{
    sem_down(sharedServerData->subsSemaphoreId);
    int status = sub_del_all(sharedServerData->subscriptions, communicationId);
    if (status < 0)
    {
        printf("unsub all failed\n");
    }
    sem_up(sharedServerData->subsSemaphoreId);
    return status;
}

int sh_store_lock(SharedData *sharedServerData)
{
    return sem_down(sharedServerData->storeSemaphoreId);
}

int sh_store_unlock(SharedData *sharedServerData)
{
    return sem_up(sharedServerData->storeSemaphoreId);
}

