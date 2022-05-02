//
// Created by Tobi on 02.05.2022.
//
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "keyValStore.h"

const char *welcomeInfo =
        "/////////////////////////////////////////\n"
        " _ _ _ _ _ _ _                           \n"
        "| | | |_| | | |_ ___ _____ _____ ___ ___ \n"
        "| | | | | | | '_| . |     |     | -_|   |\n"
        "|_____|_|_|_|_,_|___|_|_|_|_|_|_|___|_|_|\n"
        "                                         \n"
        "   +--------------+                      \n"
        "   |.------------.|                      \n"
        "   || Willkommen ||                      \n"
        "   || > PUT k v  ||                      \n"
        "   || > GET k    ||                      \n"
        "   || > DEL k    ||                      \n"
        "   |+------------+|                      \n"
        "   +-..--------..-+                      \n"
        "   .--------------.                      \n"
        "  / /============\\ \\                   \n"
        " / /==============\\ \\                  \n"
        "/____________________\\                  \n"
        "\\____________________/                  \n"
        "\n"
        "/////////////////////////////////////////\n";


const char *unavailableInfo =
    " ____|    \\         ___ ___ ___   \n"
    "(____|     `._____  | | |   | | |  \n"
    " ____|       _|___  |_  | | |_  |  \n"
    "(____|     .'         |_|___| |_|  \n"
    "     |____/                        \n";



int user_interact(int fileDescriptor, KeyValueDatabase *db) {
    int inputBuffSize = MAX_KEY_LENGTH + MAX_VALUE_LENGTH;
    char inputBuff[inputBuffSize];

    char keyBuff[MAX_KEY_LENGTH] = "";
    char valBuff[MAX_VALUE_LENGTH] = "";
    int status = 0;

    dprintf(fileDescriptor,"%s\n", welcomeInfo);
    dprintf(fileDescriptor, "input> ");
    while (read(fileDescriptor, inputBuff, inputBuffSize) > 0) {
        dprintf(fileDescriptor, "response> ");
        if (strncmp(inputBuff, "QUIT", 4) == 0) {
            break;
        } else if (strncmp(inputBuff, "BEG", 3) == 0) {
            dprintf(fileDescriptor, "BEG NOT YET SUPPORTED\n");
            dprintf(fileDescriptor, "input> ");
        } else if (strncmp(inputBuff, "END", 3) == 0) {
            dprintf(fileDescriptor, "END NOT YET SUPPORTED\n");
            dprintf(fileDescriptor, "input> ");
        } else if (sscanf(inputBuff, "PUT %s %s", keyBuff, valBuff) == 2) {
            status = db_put(db, keyBuff, valBuff);
            if (status < 0) {
                dprintf(fileDescriptor, "PUT FAILED\n");
                dprintf(fileDescriptor, "input> ");
            } else {
                dprintf(fileDescriptor, "PUT %s:%s\n", keyBuff, valBuff);
                dprintf(fileDescriptor, "input> ");
            }
        } else if (sscanf(inputBuff, "DEL %s", keyBuff) == 1) {
            status = db_del(db, keyBuff);
            if (status < 0) {
                dprintf(fileDescriptor, "DEL failed\n");
                dprintf(fileDescriptor, "input> ");
            } else {
                dprintf(fileDescriptor, "DEL %s\n", keyBuff);
                dprintf(fileDescriptor, "input> ");
            }
        } else if (sscanf(inputBuff, "GET %s", keyBuff)) {
            status = db_get(db, keyBuff, valBuff);
            if (status < 0) {
                dprintf(fileDescriptor, "GET failed\n");
                dprintf(fileDescriptor, "input> ");
            } else {
                dprintf(fileDescriptor, "GET:%s:%s\n", keyBuff, valBuff);
                dprintf(fileDescriptor, "input> ");
            }
        } else {
            dprintf(fileDescriptor, "COMMAND UNKNOWN. what did you mean?\n");
        }
    }
    return 0;
}

int user_show_unavailable(int fileDescriptor){
    dprintf(fileDescriptor,"%s\n",unavailableInfo);
    return 0;
}