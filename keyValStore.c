//
// Created by leonp on 28.04.2022.
//

#include "keyValStore.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SIZE 500 //Größe des KeyVal Arrays

typedef struct KeyVal {
    char key[100];
    char value[100];
} KeyVal;

struct KeyVal * keyVal;


static int *keyValI;
static int idKV;
static int idi;

int put(char *key, char *value) {
    for(int j = 0; j < *keyValI; j++) {
        if(strcmp(keyVal[j].key, key) == 0) {
            strcpy(keyVal[j].value, value);
            return 0;
        }
    }
    strcpy(keyVal[*keyValI].key, key);
    strcpy(keyVal[*keyValI].value, value);

    *keyValI += 1;
    return 0;
}

int get(char *key, char *value) {
    for(int j = 0; j < *keyValI; j++) {
        if(strcmp(keyVal[j].key, key) == 0) {
            strcpy(value, keyVal[j].value);
            return 0;
        }
    }

    return -1;
}

int del(char *key) {
    for(int j = 0; j < *keyValI; j++) {
        if(strcmp(keyVal[j].key, key) == 0) {
            for(; j < *keyValI - 1; j++) {
                strcpy(keyVal[j].key, keyVal[j + 1].key);
                strcpy(keyVal[j].value, keyVal[j + 1].value);
            }
            *keyValI -= 1;
            return 0;
        }
    }
    return -1;
}


int initializeKeyValShM() {
    idKV = shmget(IPC_PRIVATE, sizeof(KeyVal) * SIZE, IPC_CREAT|0777); // 0600 // 0666
    printf("Shared Memory mit id: %d wurde erstellt\n", idKV);
    keyVal = (struct KeyVal *) shmat(idKV, 0, 0);

    idi = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT|0777); // 0600 // 0666
    printf("Shared Memory mit id: %d wurde erstellt\n", idi);
    keyValI = (int *) shmat(idi, 0, 0);

    *keyValI = 0;
}

int dtKeyValShM() {
    shmdt(keyValI);
    shmdt(keyVal);
}

int rmKeyValShM() {
    shmctl(idi, IPC_RMID, 0);
    shmctl(idKV, IPC_RMID, 0);
}

