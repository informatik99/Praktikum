#ifndef PRAKTIKUM_SHARED_H
#define PRAKTIKUM_SHARED_H

#include "sub.h"
#include "store.h"

/*
 * Diese Daten werden zwischen Prozessen
 * geteilt und sind somit anfällig für race-conditions.
 * Zugriff sollte nur über die angegebenen funktionen erfolgen.
 */
typedef struct SharedData
{
    KeyValueStore *store;
    SubscriptionList *subscriptions;
    int sharedMemStoreKey;
    int sharedMemSubKey;
    int storeSemaphoreId;
    int subsSemaphoreId;
    int queueId;
} SharedData;

// fordert shared memory an, initialisiert
// alle sachen, die (unter anderem)
// zur synchronisation wichtig sind,
// und initialisiert dann auch die einzelnen Elemente
// im shared memory
int sh_init(SharedData *sharedServerData);

// entkoppelt shared memory von dem prozess,
// der gerade läuft. jeder Kind-Prozess sollte das machen bevor er schließt.
// der Eltern prozess ruft stattdessen free zum schluss auf
int sh_detach(SharedData *sharedServerData);

// zerstört das angeforderte shared memory
// (soll nur aufgerufen werden, wenn alle kind-prozesse fertig sind)
int sh_free(SharedData *sharedServerData);

// sendet eine nachricht an den prozess, der es bekommen soll
int sh_send_to(SharedData *sharedServerData, const char *message, int pid);

// empfängt eine nachricht und schreibt sie in den parameter message rein
int sh_receive_as(SharedData *sharedServerData, int pid, char *message, int maxLen);

// gibt alle daten auf die konsole aus
void sh_race_print(SharedData *sharedServerData);


// platziert ein key value paar im store - NICHT SYNCHRONISIERT!
int sh_store_race_put(SharedData *sharedServerData, const char *key, const char *value);

// schreibt den entsprechenden wert eines schlüssels, in value rein. - NICHT SYNCHRONISIERT!
int sh_store_race_get(SharedData *sharedServerData, const char *key, char *value);

// löscht einen schlüssel-wert eintrag - NICHT SYNCHRONISIERT!
int sh_store_race_del(SharedData *sharedServerData, const char *key);

// stellt exklusiven zugriff, auf den store, her --> zum synchronisieren
int sh_store_lock(SharedData *sharedServerData);

// hebt exklusiven zugriff, auf den store, auf --> zum synchronisieren
int sh_store_unlock(SharedData *sharedServerData);


// fügt eine subscription hinzu,
// also hält fest, welche prozess an welchem schlüssel
// interessiert ist - SYNCHRONISIERT
int sh_subs_sync_add(SharedData *sharedServerData, const char *key, int communicationId);

// sendet eine nachricht an alle, von unseren, prozessen,
// die an einem bestimmten key interessiert sind - SYNCHRONISIERT
int sh_subs_sync_notify(SharedData *sharedServerData, const char *key, const char *message);

// gegenstück zu add - - SYNCHRONISIERT
int sh_subs_sync_del(SharedData *sharedServerData, const char *key, int pid);

// löscht alle subscription einträge
// für einen bestimmten prozess - SYNCHRONISIERT
int sh_subs_sync_del_all_for(SharedData *sharedServerData, int communicationId);


#endif //PRAKTIKUM_SHARED_H
