//
// Created by leonp on 04.06.2022.
//

#include "keyValStore.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/msg.h>
#include "sub.h"

#define SIZE 500 //Größe des Sub Arrays

typedef struct Sub {
    int pid;
    char key[100];
} Sub;

struct text_message {
    long mtype;
    char mtext[100];
};

struct Sub *sub;

static int msgId;
static int *subI;
static int subId;
static int subIdI;

int subscribe(char *key) {
        int pid = getpid();

        sub[*subI].pid = pid;
        strcpy(sub[*subI].key, key);

        *subI += 1;
        return 0;
}

int notifySubscribers(char *key, char *msg) {
    int v;
    struct text_message mess;
    strcpy(mess.mtext, msg);
    for(int j = 0; j < *subI; j++) {
        if(strcmp(sub[j].key, key) == 0) {
            if(sub[j].pid != getpid()) {
                mess.mtype = sub[j].pid;
                v = msgsnd(msgId, &mess, strlen(msg) + 1, 0);
                if (v < 0) {
                    printf("error writing to queue\n");
                }
            }
        }
    }
}

int subService(int pid) {
    long v;
    struct text_message mess;
    while(1) {
        v = msgrcv(msgId, &mess, 100, pid, 0);
        if(v < 0) {
            printf("no appropriate message on queue\n");
        } else {
            writeMsg(mess.mtext);
        }
    }

}

int initializeMsg() {
    msgId = msgget(IPC_PRIVATE, IPC_CREAT|0666);
    if(msgId== -1) {
        printf("cannot get message queue\n");
        return -1;
    }
}

int initializeSubscriptionShM() {
    subId = shmget(IPC_PRIVATE, sizeof(Sub) * SIZE, IPC_CREAT|0777); // 0600 // 0666
    printf("Shared Memory mit id: %d wurde erstellt\n", subId);
    sub = (struct Sub *) shmat(subId, 0, 0);

    subIdI = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0777); // 0600 // 0666
    printf("Shared Memory mit id: %d wurde erstellt\n", subIdI);
    subI = (int *) shmat(subIdI, 0, 0);

    *subI = 0;
}


