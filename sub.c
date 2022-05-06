#include <stdio.h>
#include <string.h>
#include "sub.h"

int sub_init(SubscriptionList *subs)
{
    // am anfang, haben wir keine
    // subscriptions in der liste
    subs->subLen = 0;
    return STATUS_OK;
}

int sub_free(SubscriptionList *subs)
{
    // wir speichern nichts mehr in der liste
    subs->subLen = 0;
    return STATUS_OK;
}

int sub_add(SubscriptionList *subs, int processId, const char *key)
{
    // erst einmal testen, ob wir genügend platz haben
    if (subs->subLen >= MAX_SUBSCRIPTIONS)
    {
        fprintf(stderr, "Too many subs, no space!\n");
        return STATUS_FAIL_NOT_ENOUGH_SPACE;
    }

    // nehme den ersten, nicht initialisierten eintrag
    SubscriptionEntry *entry = &subs->subs[subs->subLen];

    // in diesen eintrag sollen jetzt die eingabedaten reingeschrieben werden
    strncpy(entry->key, key, MAX_KEY_LENGTH);
    entry->key[MAX_KEY_LENGTH - 1] = '\0';
    entry->processId = processId;

    // wir haben jetzt einen neuen eintrag in der liste
    subs->subLen++;
    return STATUS_OK;
}

int sub_del(SubscriptionList *subs, int processId, const char *key)
{
    // falls kein eintrag in der Liste ist,
    // gibt es auch nichts zu löschen
    if (subs->subLen <= 0)
    {
        fprintf(stderr, "subs are empty\n");
        return STATUS_FAIL_EMPTY;
    }

    // finde die stelle in der liste,
    // wo der key übereinstimmt
    int keyIndex;
    for (keyIndex = 0; keyIndex < subs->subLen; keyIndex++)
    {
        if (subs->subs[keyIndex].processId == processId
            && strncmp(subs->subs[keyIndex].key, key, MAX_KEY_LENGTH) == 0)
        {
            break;
        }
    }

    // sind wir über die maximal erlaubte länge um eins
    // hinausgeschossen, wissen wir, dass der key nicht
    // in der liste existiert
    if (keyIndex >= subs->subLen)
    {
        fprintf(stderr, "keyType entry not there\n");
        return STATUS_KEY_NOT_EXISTENT;
    }

    // zum Löschen, ersetzen wir diesen eintrag einfach
    // mit dem letzten validen Eintrag.
    // der letzte valide Eintrag wird dann herausgenommen
    subs->subs[keyIndex] = subs->subs[subs->subLen - 1];
    subs->subLen--;
    return STATUS_OK;
}

int sub_del_all(SubscriptionList *subs, int processId)
{
    // falls kein eintrag in der Liste ist,
    // gibt es auch nichts zu löschen
    if (subs->subLen <= 0)
    {
        fprintf(stderr, "subs are empty\n");
        return STATUS_FAIL_EMPTY;
    }

    // überschreibe alle einträge wo der
    // eingabeparameter übereinstimmt
    int keyIndex;
    for (keyIndex = 0; keyIndex < subs->subLen; keyIndex++)
    {
        while (keyIndex < subs->subLen && subs->subs[keyIndex].processId == processId)
        {
            subs->subs[keyIndex] = subs->subs[subs->subLen - 1];
            subs->subLen--;
            break;
        }
    }

    return STATUS_OK;
}


void sub_print(SubscriptionList *subs)
{
    printf("Subscriptions:\n");
    for (int i = 0; i < subs->subLen; i++)
    {
        printf("(type:%d | key: \"%s\")\n", subs->subs[i].processId, subs->subs[i].key);
    }
}
