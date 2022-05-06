#ifndef PRAKTIKUM_SUB_H
#define PRAKTIKUM_SUB_H

#include "constants.h"

// ein eintrag soll festhalten
// welcher prozess an welchem key interessiert ist
typedef struct SubscriptionEntry
{
    int processId;
    char key[MAX_KEY_LENGTH];
} SubscriptionEntry;

typedef struct SubscriptionList
{
    SubscriptionEntry subs[MAX_SUBSCRIPTIONS];
    int subLen;
} SubscriptionList;

int sub_init(SubscriptionList *subs);

int sub_free(SubscriptionList *subs);

int sub_add(SubscriptionList *subs, int processId, const char *key);

int sub_del(SubscriptionList *subs, int processId, const char *key);

int sub_del_all(SubscriptionList *subs, int processId);

void sub_print(SubscriptionList *subs);


#endif //PRAKTIKUM_SUB_H
