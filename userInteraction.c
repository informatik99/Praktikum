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


const char *transactionInfo =
        " ______                           __  _                 \n"
        "/_  __/______ ____  ___ ___ _____/ /_(_)__  ___         \n"
        " / / / __/ _ `/ _ \\(_-</ _ `/ __/ __/ / _ \\/ _ \\     \n"
        "/_/ /_/  \\_,_/_//_/___/\\_,_/\\__/\\__/_/\\___/_//_/   \n"
        "                                                ";

const char *transactionCloseInfo =
        "   ____        __  ______                           __  _         \n"
        "  / __/__  ___/ / /_  __/______ ____  ___ ___ _____/ /_(_)__  ___ \n"
        " / _// _ \\/ _  /   / / / __/ _ `/ _ \\(_-</ _ `/ __/ __/ / _ \\/ _ \\\n"
        "/___/_//_/\\_,_/   /_/ /_/  \\_,_/_//_/___/\\_,_/\\__/\\__/_/\\___/_//_/\n"
        "                                                                  ";

const char *userInputPrefix = "$ ";
const char *serverOutputPrefix = "> ";


int user_interact_inside_transaction(int fileDescriptor, KeyValueDatabase *db){
    int inputBuffSize = MAX_KEY_LENGTH + MAX_VALUE_LENGTH;
    char inputBuff[inputBuffSize];

    char keyBuff[MAX_KEY_LENGTH] = "";
    char valBuff[MAX_VALUE_LENGTH] = "";
    int status = 0;

    dprintf(fileDescriptor,"%s\n", transactionInfo);
    while (read(fileDescriptor, inputBuff, inputBuffSize) > 0) {

        if (strncmp(inputBuff, "QUIT", 4) == 0) {

            dprintf(fileDescriptor,
                    "%sYou are inside a transaction. Please say END first\n%s",
                    serverOutputPrefix, userInputPrefix);

        } else if (strncmp(inputBuff, "BEG", 3) == 0) {

            dprintf(fileDescriptor,
                    "%sYou are already inside the transaction\n%s",
                    serverOutputPrefix, userInputPrefix);

        } else if (strncmp(inputBuff, "END", 3) == 0) {

            dprintf(fileDescriptor,
                    "%sTRANSACTION END\n%s\n%s",
                    serverOutputPrefix, transactionCloseInfo, userInputPrefix);
            break;

        } else if (sscanf(inputBuff, "PUT %s %s", keyBuff, valBuff) == 2) {

            // execute command
            status = db_put(db, keyBuff, valBuff);

            // print result
            if (status < 0) {
                dprintf(fileDescriptor,
                        "%sPUT FAILED\n%s",
                        serverOutputPrefix,userInputPrefix);
            } else {
                dprintf(fileDescriptor,
                        "%sPUT %s:%s\n%s",
                        serverOutputPrefix, keyBuff, valBuff, userInputPrefix);
            }

        } else if (sscanf(inputBuff, "DEL %s", keyBuff) == 1) {

            // execute command
            status = db_del(db, keyBuff);

            // print result
            if (status < 0) {
                dprintf(fileDescriptor,
                        "%sDEL failed\n%s",
                        serverOutputPrefix, userInputPrefix);
            } else {
                dprintf(fileDescriptor,
                        "%sDEL %s\n%s",
                        serverOutputPrefix, keyBuff, userInputPrefix);
            }

        } else if (sscanf(inputBuff, "GET %s", keyBuff)) {

            // execute command
            status = db_get(db, keyBuff, valBuff);
            // print result
            if (status < 0) {
                dprintf(fileDescriptor,
                        "%sGET failed\n%s",
                        serverOutputPrefix, userInputPrefix);
            } else {
                dprintf(fileDescriptor,
                        "%sGET:%s:%s\n%s",
                        serverOutputPrefix, keyBuff, valBuff, userInputPrefix);
            }

        } else {

            dprintf(fileDescriptor,
                    "%sCOMMAND UNKNOWN. what did you mean?\n%s",
                    serverOutputPrefix, userInputPrefix);
        }
    }
    return 0;
}

int user_interact(int fileDescriptor, KeyValueDatabase *db) {
    int inputBuffSize = MAX_KEY_LENGTH + MAX_VALUE_LENGTH;
    char inputBuff[inputBuffSize];

    char keyBuff[MAX_KEY_LENGTH] = "";
    char valBuff[MAX_VALUE_LENGTH] = "";
    int status = 0;

    dprintf(fileDescriptor,"%s\n$ ", welcomeInfo);

    while (read(fileDescriptor, inputBuff, inputBuffSize) > 0) {

        if (strncmp(inputBuff, "QUIT", 4) == 0) {

            dprintf(fileDescriptor,
                    "%sThank you for using our service!\n%s",
                    serverOutputPrefix,userInputPrefix);
            break;

        } else if (strncmp(inputBuff, "BEG", 3) == 0) {

            db_beg(db);
            dprintf(fileDescriptor,
                    "%sTRANSACTION START\n%s",
                    serverOutputPrefix, userInputPrefix);
            user_interact_inside_transaction(fileDescriptor,db);
            db_end(db);

        } else if (strncmp(inputBuff, "END", 3) == 0) {

            dprintf(fileDescriptor,
                    "%sYou are NOT inside a transaction\n%s",
                    serverOutputPrefix, userInputPrefix);

        } else if (sscanf(inputBuff, "PUT %s %s", keyBuff, valBuff) == 2) {

            // execute command
            db_beg(db);
            status = db_put(db, keyBuff, valBuff);
            db_end(db);

            // print result
            if (status < 0) {
                dprintf(fileDescriptor,
                        "%sPUT FAILED\n%s",
                        serverOutputPrefix, userInputPrefix);
            } else {
                dprintf(fileDescriptor,
                        "%sPUT %s:%s\n%s",
                        serverOutputPrefix, keyBuff, valBuff, userInputPrefix);
            }

        } else if (sscanf(inputBuff, "DEL %s", keyBuff) == 1) {

            // execute command
            db_beg(db);
            status = db_del(db, keyBuff);
            db_end(db);

            // print result
            if (status < 0) {
                dprintf(fileDescriptor,
                        "%sDEL failed\n%s",
                        serverOutputPrefix, userInputPrefix );
            } else {
                dprintf(fileDescriptor,
                        "%sDEL %s\n%s",
                        serverOutputPrefix, keyBuff, userInputPrefix);
            }

        } else if (sscanf(inputBuff, "GET %s", keyBuff)) {

            // execute command
            db_beg(db);
            status = db_get(db, keyBuff, valBuff);
            db_end(db);

            // print result
            if (status < 0) {
                dprintf(fileDescriptor,
                        "%sGET failed\n%s",
                        serverOutputPrefix, userInputPrefix);
            } else {
                dprintf(fileDescriptor,
                        "%sGET:%s:%s\n%s",
                        serverOutputPrefix, keyBuff, valBuff, userInputPrefix);
            }

        } else {
            dprintf(fileDescriptor,
                    "%sCOMMAND UNKNOWN. what did you mean?\n%s",
                    serverOutputPrefix, userInputPrefix);
        }
    }
    return 0;
}

int user_show_unavailable(int fileDescriptor){
    dprintf(fileDescriptor,"%s\n",unavailableInfo);
    return 0;
}