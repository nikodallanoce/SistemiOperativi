#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "trainUtils.h"

#define DEFAULT_PROTOCOL 0


void occupySegment(int segNumber);

void leaveSegment(int segNumber);

int readCheckAvailability(int fd);

int writeAvailability(int fd, int ris);

int isValidSegment(int segNumber);

int convert(char *number);

int checkArray(int number);

void updateSegments(int fd);

char **splitString(char *string);

int isNumber(char c);

int isStation(char *segment);

int digits(char *segName);

int convertSegment(char *segment);

void createRBCLog(char *Tname, char *myPos, char *nextPos, int aut);

void occupyStation(char *station);

void leaveStation(char *station);

void serverRoute();

void printStation();

int *binaries;
int *stations;

/*
 * main() crea le strutture necessarie per la creazione della socket
 * e mette in ascolto il server.
 */

int main(void) {

    binaries = calloc(16, sizeof(int));
    int s[] = {0, 1, 1, 1, 1, 1, 0, 0};
    stations = s;
    cleanLog("RBC");
    serverRoute();
    // binaries[0] = 1;
    int serverFd, clientFd, serverLen, clientLen;
    struct sockaddr_un serverUNIXAddress;/* Server address */
    struct sockaddr_un clientUNIXAddress; /* Client address */
    struct sockaddr *serverSockAddrPtr; /* Ptr to server address */
    struct sockaddr *clientSockAddrPtr; /* Ptr to client address */
/* Ignore death-of-child signals to prevent zombies */
    signal(SIGCHLD, SIG_IGN);
    serverSockAddrPtr = (struct sockaddr *) &serverUNIXAddress;
    serverLen = sizeof(serverUNIXAddress);
    clientSockAddrPtr = (struct sockaddr *) &clientUNIXAddress;
    clientLen = sizeof(clientUNIXAddress);
/* Create a UNIX socket, bidirectional, default protocol */
    serverFd = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
    serverUNIXAddress.sun_family = AF_UNIX; /* Set domain type */
    strcpy(serverUNIXAddress.sun_path, "RBCServer"); /* Set name */
    unlink("RBCServer"); /* Remove file if it already exists */
    bind(serverFd, serverSockAddrPtr, serverLen); /* Create file */
    listen(serverFd, 5); /* Maximum pending connection length */
    while (1) { /* Loop forever */
/* Accept a client connection */
        clientFd = accept(serverFd, clientSockAddrPtr, &clientLen);

        int r = readCheckAvailability(clientFd);
        writeAvailability(clientFd, r); /* Send the recipe */

        updateSegments(clientFd);
        //exit(/* EXIT_SUCCESS */ 0); /* Terminate */
        //close(clientFd); /* Close the client descriptor */
    }

}

/*
 *serverRoute() crea e stampa la struttura dati in cui sono contenuti
 * i percorsi di ogni treno.
 */

void serverRoute() {
    char *TName = malloc(20);

    for (int i = 1; i < 6; i++) {
        sprintf(TName, "T%d", i);
        printf("%s", TName);
        char **route = TRoute(TName);
        printStringArray(route);
        free(route);
    }
    printf("\n");

}

/*
 * readCheckAvailability(int fd) controlla se un treno puo avanzare nel successivo
 * segmento, controllando cosa ce in *binaries[nextPos] e lo restituisce.
 * Se ritorna 0 non puo avanzare, altrimenti si.
 */

int readCheckAvailability(int fd) {

    char *buffer = malloc(sizeof(char) * 200);
    read(fd, buffer, 100);
    char **splitted;
    splitted = splitString(buffer);
    char *Tname = splitted[0];
    char *mySeg = splitted[1];
    char *nextSeg = splitted[2];
    int segToCheck = convertSegment(nextSeg);
    int ris = checkArray(segToCheck);
    if (ris == 0) {
        printf("Autorizzato %s  %s -> %s\n", splitted[0], mySeg, nextSeg);
        createRBCLog(Tname, mySeg, nextSeg, ris);
    } else {
        printf("Waits %s  %s -> %s\n", splitted[0], mySeg, nextSeg);
        createRBCLog(Tname, mySeg, nextSeg, ris);
    }
    //free(splitted);
    return ris;

}

/*
 *updateSegments(int fd) aggiorna lo stato delle boe quando il server
 * concede autorizzazione ad un treno di avanzare.
 */

void updateSegments(int fd) {

    char *buffer = malloc(sizeof(char) * 200);
    read(fd, buffer, 100);

    if (strcmp("WAIT", buffer) == 0) {
        return;
    }

    char **splitted;
    splitted = splitString(buffer);
    char *mySeg = splitted[1];
    char *nextSeg = splitted[2];
    int segToLeave = convertSegment(mySeg);
    int segNext = convertSegment(nextSeg);

    occupySegment(segNext);
    leaveSegment(segToLeave);
    occupyStation(nextSeg);
    leaveStation(mySeg);
    printStation();

}

/*
 * printStation() stampa lo stato delle stazioni
 */

void printStation() {
    printf("Stations: [");
    for (int i = 0; i < 8; ++i) {
        printf("%d", stations[i]);
    }
    printf("]\n");
}

/*
 * occupyStation(char *station) aggiunge un treno alla stazione
 * indicata da *station (indica che un treno e arrivato).
 */

void occupyStation(char *station) {
    if (isStation(station)) {
        int number = station[1] - 48;
        stations[number - 1] += 1;
    }
}

/*
 * leaveStation(char *station) sottrae un treno dalla stazione
 * quando parte.
 */

void leaveStation(char *station) {
    if (isStation(station)) {
        int number = station[1] - 48;
        stations[number - 1] -= 1;
    }
}


/*
 * **splitString(char *string) ritorna un array di stringhe composto
 *   dalle stringhe comprese tra il carattere '|'.
 */

char **splitString(char *string) {
    char *str = malloc(300);
    strcpy(str, string);

    char **ris = malloc(sizeof(char) * 300);

    /* get the first token */
    char *token = strtok(str, "|");

    /* walk through other tokens */
    int i = 0;
    while (token != NULL) {

        ris[i] = malloc(30);
        strcpy(ris[i], token);
        //printf(" %s\n", ris[i]);
        i++;

        token = strtok(NULL, "|");
    }

    return ris;
}

/*
 * checkArray(int number) controlla se un binario e libero dentro
 * binaries e restituisce 0 se libero, 1 se occupato.
 */

int checkArray(int number) {
    int avail = -1;
    if (number == -1) {
        avail = 0;
    } else if (number >= 0) {
        avail = binaries[number - 1];
    }

    return avail;
}



/*
 * writeAvailability(int fd, int ris) scrive nella socket se il segmento
 * richiesto dal client è libero. (0 libero, 1 occupato).
 */

int writeAvailability(int fd, int ris) {
    char *line1 = malloc(sizeof(char) * 2);

    sprintf(line1, "%d", ris);
    write(fd, line1, strlen(line1) + 1); /* Write first line */

}

/*
 * isValidSegment(int segNumber) controlla se il numero passato
 * puo essere una boa. (un numero negativo di segNumber indica
 * solitamente una stazione).
 */

int isValidSegment(int segNumber) {
    return segNumber >= 0 ? 1 : 0;
}

/*
 * occupySegment(int segNumber) scrive 1 in posizione segNumber-1
 * di binaries.
 */

void occupySegment(int segNumber) {
    if (isValidSegment(segNumber)) {
        binaries[segNumber - 1] = 1;
    }
}

/*
 *occupySegment(int segNumber) scrive 0 in posizione segNumber-1
 *di binaries.
 */

void leaveSegment(int segNumber) {
    if (isValidSegment(segNumber)) {
        binaries[segNumber - 1] = 0;
    }
}

/*
 * createRBCLog(...) crea il file di log per il server.
 */

void createRBCLog(char *Tname, char *myPos, char *nextPos, int aut) {
    char *TLog = logName("RBC");
    FILE *log = fopen(TLog, "a");
    if (log == NULL) {
        log = fopen(TLog, "w");
    }
    char *autor[2] = {"SI", "NO"};
    char *a = autor[aut];
    fprintf(log, "[Treno %s][Attuale: %s][Next: %s][Autorizzato: %s] [%s] \n", Tname, myPos, nextPos, a, dateNow());
    fclose(log);
}
