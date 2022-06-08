
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "sub.h"
#include <arpa/inet.h>
#include "keyValStore.h"
#include "subscription.h"
#include <sys/sem.h>


#define BUFSIZE 1024 // Größe des Buffers
#define ENDLOSSCHLEIFE 1
#define PORT 5678


int leave(int sem_id) {
    struct sembuf leave; // Structs für den Semaphor
    leave.sem_num = 0;  // Semaphor 0 in der Gruppe
    leave.sem_flg = SEM_UNDO;
    leave.sem_op = 1;   // freigeben, UP-Operation

    semop(sem_id, &leave, 1);
    return 0;
}

int enter(int sem_id) {
    struct sembuf enter; // Structs für den Semaphor
    enter.sem_num = 0;  // Semaphor 0 in der Gruppe
    enter.sem_flg = SEM_UNDO;
    enter.sem_op = -1; // blockieren, DOWN-Operation

    semop(sem_id, &enter, 1);

    return 0;
}



int main() {

    int cfd; // Verbindungs-Descriptor
    int rfd; // Rendevouz-Descriptor
    int pid; // ProzessID



    struct sockaddr_in client; // Socketadresse eines Clients
    socklen_t client_len; // Länge der Client-Daten
    char in[BUFSIZE]; // Daten vom Client an den Server
    unsigned short marker[1];
    int sem_id;


    // Es folgt das Anlegen der Semaphorgruppe. Es wird hier nur ein
    // Semaphor erzeugt

    sem_id = semget (IPC_PRIVATE, 1, IPC_CREAT|0644);
    if (sem_id == -1) {
        perror ("Die Gruppe konnte nicht angelegt werden!");
        exit(1);
    }

    // Anschließend wird der Semaphor auf 1 gesetzt
    marker[0] = 1;
    semctl(sem_id, 1, SETALL, marker);  // alle Semaphore auf 1


    // Socket erstellen
    rfd = socket(AF_INET, SOCK_STREAM, 0);
    if (rfd < 0 ){
        fprintf(stderr, "Socket konnte nicht erstellt werden\n");
        exit(-1);
    }


    // Socket Optionen setzen für schnelles wiederholtes Binden der Adresse
    int option = 1;
    setsockopt(rfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &option, sizeof(int));


    // Socket binden
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
    int brt = bind(rfd, (struct sockaddr *) &server, sizeof(server));
    if (brt < 0 ){
        fprintf(stderr, "Socket konnte nicht gebunden werden\n");
        exit(-1);
    }

    // Socket lauschen lassen
    int lrt = listen(rfd, 5);
    if (lrt < 0 ){
        fprintf(stderr, "Socket konnte nicht listen gesetzt werden\n");
        exit(-1);
    }

    //Shared Memory wird angelegt
    initializeKeyValShM();
    initializeSubShM();
    initializeSubscriptionShM();

    //msg anlegen
    initializeMsg();


    int clientsConnected = 0;
    int quit = 0;
    while(ENDLOSSCHLEIFE) {
        // Neuer Verbindungs-Descriptor
        cfd = accept(rfd, (struct sockaddr *)&client, &client_len);

        if(cfd < 0) {
            exit(-1);
        }

        printf("Neue Verbindung von IP Adresse: %s : %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        printf("Verbundene Clients: %d\n\n", ++clientsConnected);

        if((pid = fork()) == 0) { // Neuer Child-Prozess
            printf("Neuer Prozess wurde erstellt\n");
            close(rfd);
            initializeSocket(cfd);

            if((pid = fork()) == 0) {
                printf("Neuer Prozess wurde erstellt\n");
                while (quit == 0) {

                    read(cfd, in, BUFSIZE);
                    enter(sem_id);
                    quit = executeCommand(in);
                    leave(sem_id);
                }
                close(cfd);
                dtKeyValShM();
                dtSubShM();
                exit(0);
            } else {
                subService(pid);
            }

        }

        close(cfd);
    }

    return 0;
}




