//
// Created by leonp on 28.04.2022.
//
#include "sub.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "keyValStore.h"
#include "subscription.h"
#include <sys/ipc.h>
#include <sys/shm.h>


static int idPrivate;
static int * shPrivate;
static int private;
static int socketD;

int cleanString(char *string1) {
    int i = 0;
    char *newString;
    if(string1 != NULL) {
        while (string1[i] != '\r' && string1[i] != '\n' && string1[i] != '\0') {
            newString[i] = string1[i];
            i++;
        }
        newString[i] = '\0';

        strcpy(string1, newString);
    }
    return 0;
}

int writeMsg(char *msg) {
    write(socketD, msg, strlen(msg));
    return 0;
}


int responed(char *command, char *key, char *value) {
    char output[150];

    snprintf(output, sizeof(output), "%s:%s:%s\n",command, key, value);

    write(socketD, output, strlen(output));

    if(strncmp(command, "DEL", 3) == 0 || strncmp(command, "PUT", 3) == 0) {
        notifySubscribers(key, output);
    }
    return 0;
}


int executeCommand(const char *commandAndInput) {
    char get1[3] = "GET";
    char put1[3] = "PUT";
    char del1[3] = "DEL";
    char quit[4] = "QUIT";
    char beg[3] = "BEG";
    char end[3] = "END";
    char sub[3] = "SUB";

    char *command = NULL;
    char *key = NULL;
    char *value = NULL;
    char *input = malloc(150 * sizeof(char));

    int i = 0;

    if(commandAndInput != NULL) {
        while (commandAndInput[i] != '\r' && commandAndInput[i] != '\n' && commandAndInput[i] != '\0') {
            input[i] = commandAndInput[i];
            i++;
        }
        input[i] = '\0';
    }


    command = strtok(input, "\r\n ");
    key = strtok(NULL, "\r\n ");
    value = strtok(NULL, "\r\n ");

    if(*shPrivate == 1 && private != 1) {
        write(socketD, "Somebody else is in transaktion.\n", strlen("Somebody else is in transaktion.\n"));
    }
    else if (strncmp(command, get1, 3) == 0) {
        if(key != NULL) {
            value = malloc(100);

            int success = get(key, value);

            if (success == 0) {
                responed(command, key, value);
            } else {
                responed(command, key, "key_nonexistent");
            }

            free(value);
        } else {
            write(socketD, "No valid key.\n", strlen("No valid key.\n"));
        }
    }
    else if(strncmp(command, put1, 3) == 0) {
        if(key == NULL || value == NULL) {
            write(socketD, "No valid key or value.\n", strlen("No valid key or value.\n"));
            return 0;
        }

        put(key, value);

        responed(command, key, value);
    }
    else if(strncmp(command, del1, 3) == 0) {
        if(key == NULL) {
            write(socketD, "No valid key.\n", strlen("No valid key.\n"));
            return 0;
        }
        int success = del(key);
        if(success == 0) responed(command, key, "key_deleted");
        if(success == -1) responed(command, key, "key_nonexistent");
    }
    else if(strncmp(command, quit, 4) == 0) {
        write(socketD, "Au revoir.\n", strlen("Au revoir.\n"));
        return 1;
    }
    else if(strncmp(command, beg, 3) == 0) {
        *shPrivate = 1;
        private = 1;
        write(socketD, "Transaktion started.\n", strlen("Transaktion started.\n"));
    }
    else if(strncmp(command, end, 3) == 0) {
        *shPrivate = 0;
        private = 0;
        write(socketD, "Transaktion closed.\n", strlen("Transaktion closed.\n"));
    }
    else if(strncmp(command, sub, 3) == 0) {
        if(key == NULL) {
            write(socketD, "No valid key.\n", strlen("No valid key.\n"));
            return 0;
        }
        subscribe(key);
        responed(command, key, "subscribed");
    }
    else {
        char *noValidCommand = "No valid command.\n";
        write(socketD, noValidCommand, strlen(noValidCommand));
    }
    free(input);
    return 0;
}

int initializeSubShM() {

    idPrivate = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT|0777); // 0600 // 0666
    printf("Shared Memory mit id: %d wurde erstellt\n", idPrivate);
    shPrivate = (int *) shmat(idPrivate, 0, 0);

    *shPrivate = 0;
    private = 0;
    return 0;
}

int initializeSocket(int cfd) {
    socketD = cfd;
}

int dtSubShM() {
    shmdt(shPrivate);
    return 0;
}

int rmSubShM() {
    shmctl(idPrivate, IPC_RMID, 0);
    return 0;
}


