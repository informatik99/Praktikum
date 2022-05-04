//
// Created by Tobi on 02.05.2022.
//
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>
#include "keyValStore.h"

const char *welcomeInfo = "Herzlich willkommen!\n";
const char *unavailableInfo = "Bitte später wiederkommen.";
const char *transactionInfo = "TRANSACTION BEGIN\n";
const char *transactionCloseInfo = "TRANSACTION END\n";

const char *userInputPrefix = "$ ";
const char *serverOutputPrefix = "> ";

int user_interact_inside_transaction(int fileDescriptor, KeyValueDatabase *db) {

    int inputBuffSize = MAX_KEY_LENGTH + MAX_VALUE_LENGTH;
    char inputBuff[inputBuffSize];

    int outputBuffSize = 512;
    char outputBuff[outputBuffSize];

    char keyBuff[MAX_KEY_LENGTH] = "";
    char valBuff[MAX_VALUE_LENGTH] = "";
    int status;

    memset(outputBuff, 0, outputBuffSize);
    snprintf(outputBuff,outputBuffSize,"%s", transactionInfo);
    write(fileDescriptor, outputBuff, outputBuffSize);

    while (read(fileDescriptor, inputBuff, inputBuffSize) > 0) {

        if (strncmp(inputBuff, "QUIT", 4) == 0) {

            snprintf(outputBuff, outputBuffSize,
                     "%sYou are inside a transaction. Please say END first\n%s",
                     serverOutputPrefix, userInputPrefix);
            write(fileDescriptor,outputBuff,outputBuffSize);

        } else if (strncmp(inputBuff, "BEG", 3) == 0) {

            snprintf(outputBuff, outputBuffSize,
                     "%sYou are already inside the transaction\n%s",
                     serverOutputPrefix, userInputPrefix);

            write(fileDescriptor,outputBuff,outputBuffSize);

        } else if (strncmp(inputBuff, "END", 3) == 0) {

            snprintf(outputBuff, outputBuffSize,
                     "%sTRANSACTION END\n%s\n%s",
                     serverOutputPrefix, transactionCloseInfo, userInputPrefix);

            write(fileDescriptor,outputBuff, outputBuffSize);
            break;

        } else if (sscanf(inputBuff, "PUT %s %s", keyBuff, valBuff) == 2) {

            // execute command
            status = db_put(db, keyBuff, valBuff);

            // print result
            if (status < 0) {
                snprintf(outputBuff, outputBuffSize,
                         "%sPUT FAILED\n%s",
                         serverOutputPrefix, userInputPrefix);
                write(fileDescriptor, outputBuff,outputBuffSize);
            } else {

                snprintf(outputBuff,outputBuffSize,
                        "%sPUT %s:%s\n%s",
                        serverOutputPrefix, keyBuff, valBuff, userInputPrefix);
                write(fileDescriptor, outputBuff,outputBuffSize);
            }

        } else if (sscanf(inputBuff, "DEL %s", keyBuff) == 1) {

            // execute command
            status = db_del(db, keyBuff);

            // print result
            if (status < 0) {
                snprintf(outputBuff,outputBuffSize,
                        "%sDEL failed\n%s",
                        serverOutputPrefix, userInputPrefix);
                write(fileDescriptor, outputBuff, outputBuffSize);
            } else {
                snprintf(outputBuff, outputBuffSize,
                        "%sDEL %s\n%s",
                        serverOutputPrefix, keyBuff, userInputPrefix);
            }

        } else if (sscanf(inputBuff, "GET %s", keyBuff)) {

            // execute command
            status = db_get(db, keyBuff, valBuff);
            // print result
            if (status < 0) {
                snprintf(outputBuff, outputBuffSize,
                        "%sGET failed\n%s",
                        serverOutputPrefix, userInputPrefix);
                write(fileDescriptor, outputBuff, outputBuffSize);
            } else {
                snprintf(outputBuff, outputBuffSize,
                        "%sGET:%s:%s\n%s",
                        serverOutputPrefix, keyBuff, valBuff, userInputPrefix);
                write(fileDescriptor,outputBuff, outputBuffSize);
            }

        } else {
            snprintf(outputBuff, outputBuffSize,
                    "%sCOMMAND UNKNOWN. what did you mean by %s?\n%s",
                    serverOutputPrefix, inputBuff ,userInputPrefix);
            write(fileDescriptor, outputBuff, outputBuffSize);
        }
    }
    return 0;
}


int user_interact_with_db_locking(int fileDescriptor, KeyValueDatabase *db) {

    int inputBuffSize = MAX_KEY_LENGTH + MAX_VALUE_LENGTH;
    char inputBuff[inputBuffSize];

    char keyBuff[MAX_KEY_LENGTH] = "";
    char valBuff[MAX_VALUE_LENGTH] = "";
    int status;

    int outputBuffSize = MAX_KEY_LENGTH + MAX_VALUE_LENGTH;
    char outputBuff[outputBuffSize];
    memset(outputBuff, 0, outputBuffSize);

    snprintf(outputBuff, outputBuffSize, "%s\n$ ", welcomeInfo);
    write(fileDescriptor, outputBuff, outputBuffSize);


    while (read(fileDescriptor, inputBuff, inputBuffSize) > 0) {

        memset(outputBuff, 0, outputBuffSize);

        if (strncmp(inputBuff, "QUIT", 4) == 0) {

            snprintf(outputBuff, outputBuffSize,
                    "%sThank you for using our service!\n%s",
                    serverOutputPrefix, userInputPrefix);
            write(fileDescriptor, outputBuff, outputBuffSize);

            break;

        } else if (strncmp(inputBuff, "BEG", 3) == 0) {

            // execute command
            db_lock(db);
            snprintf(outputBuff, outputBuffSize,
                    "%sTRANSACTION START\n%s",
                    serverOutputPrefix, userInputPrefix);
            write(fileDescriptor, outputBuff, outputBuffSize);
            user_interact_inside_transaction(fileDescriptor, db);
            db_unlock(db);

        } else if (strncmp(inputBuff, "END", 3) == 0) {

            snprintf(outputBuff, outputBuffSize,
                    "%sYou are NOT inside a transaction\n%s",
                    serverOutputPrefix, userInputPrefix);
            write(fileDescriptor, outputBuff, outputBuffSize);

        } else if (sscanf(inputBuff, "PUT %s %s", keyBuff, valBuff) == 2) {

            // execute command
            db_lock(db);
            status = db_put(db, keyBuff, valBuff);
            db_unlock(db);

            // print result
            if (status < 0) {
                snprintf(outputBuff, outputBuffSize,
                        "%sPUT FAILED\n%s",
                        serverOutputPrefix, userInputPrefix);
                write(fileDescriptor, outputBuff, outputBuffSize);
            } else {
                snprintf(outputBuff, outputBuffSize,
                        "%sPUT %s:%s\n%s",
                        serverOutputPrefix, keyBuff, valBuff, userInputPrefix);
                write(fileDescriptor, outputBuff, outputBuffSize);
            }

        } else if (sscanf(inputBuff, "DEL %s", keyBuff) == 1) {

            // execute command
            db_lock(db);
            status = db_del(db, keyBuff);
            db_unlock(db);

            // print result
            if (status < 0) {
                snprintf(outputBuff, outputBuffSize,
                        "%sDEL failed\n%s",
                        serverOutputPrefix, userInputPrefix);
                write(fileDescriptor, outputBuff, outputBuffSize);
            } else {
                snprintf(outputBuff, outputBuffSize,
                        "%sDEL %s\n%s",
                        serverOutputPrefix, keyBuff, userInputPrefix);
                write(fileDescriptor, outputBuff, outputBuffSize);
            }

        } else if (sscanf(inputBuff, "GET %s", keyBuff)) {

            // execute command
            db_lock(db);
            status = db_get(db, keyBuff, valBuff);
            db_unlock(db);

            // print result
            if (status < 0) {
                snprintf(outputBuff, outputBuffSize,
                        "%sGET failed\n%s",
                        serverOutputPrefix, userInputPrefix);
                write(fileDescriptor, outputBuff, outputBuffSize);
            } else {
                snprintf(outputBuff, outputBuffSize,
                        "%sGET:%s:%s\n%s",
                        serverOutputPrefix, keyBuff, valBuff, userInputPrefix);
                write(fileDescriptor, outputBuff, outputBuffSize);
            }

        } else {
            snprintf(outputBuff, outputBuffSize,
                    "%sCOMMAND UNKNOWN. what did you mean?\n%s",
                    serverOutputPrefix, userInputPrefix);
            write(fileDescriptor, outputBuff, outputBuffSize);
        }
    }
    return 0;
}

int user_interact(int fileDescriptor, KeyValueDatabase *db){
    return user_interact_with_db_locking(fileDescriptor,db);
}

int user_show_unavailable(int fileDescriptor) {
    dprintf(fileDescriptor, "%s\n", unavailableInfo);
    return 0;
}