#include <stdio.h>
#include <netinet/in.h>
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



int main() {
    testStore();
    return 0;
}
