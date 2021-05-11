#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include "trainUtils.h"

#define DEFAULT_PROTOCOL 0


void printStringArray(char **array);

void ciufCiuf(char *argv[]);

int isNumber(char c);

int digits(char *segName);

int convertSegment(char *segment);

int isStation(char *segment);

void sendAvailability(int fd, char *myPos, char *nextPos);

int readCheckAvailability(int fd);

void sendGo(int fd, char *myPos, char *nextPos);

int convert(char *number);

void sendWait(int fd);

char *Tname;

int main(int argc, char *argv[]) {

    //argv[0] = "T2";
    ciufCiuf(argv);
    exit(0);

}

/*
 * ciufCiuf(char *argv[]) crea le strutture per la socket e controlla il treno:
 * se il server da il via libera per avanzare, il treno controlla la boa successiva
 * e se è libera avanza, altrimenti resta in attesa.
 * */


void ciufCiuf(char *argv[]) {

    Tname = argv[0];
    cleanLog(Tname);
    

    char **route = TRoute(Tname);
    synchro(argv);
    printf("Start : %s ", Tname);

    fflush(stdout);
    int clientFd, serverLen, result;
    struct sockaddr_un serverUNIXAddress;
    struct sockaddr *serverSockAddrPtr;
    serverSockAddrPtr = (struct sockaddr *) &serverUNIXAddress;
    serverLen = sizeof(serverUNIXAddress);
    serverUNIXAddress.sun_family = AF_UNIX; /* Server domain */
    strcpy(serverUNIXAddress.sun_path, "RBCServer"); /* Server name */

    for (int i = 1; i < getRouteLength(); i++) {

        clientFd = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
        do { /* Loop until a connection is made with the server */
            result = connect(clientFd, serverSockAddrPtr, serverLen);
            if (result == -1) sleep(1); /* Wait and then try again */
        } while (result == -1);

        char *nextPos = route[i];
        char *myPos = route[i - 1];
        int *files = lockFile(myPos, nextPos);


        sendAvailability(clientFd, myPos, nextPos);
        int avail = readCheckAvailability(clientFd); /* Read the recipe */

        if (avail == 0 && readAvailability(files[1]) == 0) {
            sendGo(clientFd, myPos, nextPos);
            takePosition(files[1]);
            leavePosition(files[0]);
            createLog(route[i - 1], nextPos, Tname);

        } else {
            createLog("waiting", nextPos, Tname);
            sendWait(clientFd);
            i--;
        }
        unlockFile(files[0], files[1]);
        close(clientFd); /* Close the socket */
        sleep(3);

    }
}

/*
 * sendWait(int fd) invia al server un segnale di wait
 * nel caso in cui lui abbia dato il via libera per avanzare,
 * ma la boa successiva non e libera.
 */

void sendWait(int fd) {

    char *buffer = "WAIT";
    write(fd, buffer, strlen(buffer) + 1);
}
/*
 * sendGo(...) comunica al server la propria posizione e
 * quella successiva in cui vuole andare
 */

void sendGo(int fd, char *myPos, char *nextPos) {
    sendAvailability(fd, myPos, nextPos);
}

/*
 * endAvailability(...) concatena usando il carattere '|', le informazioni
 * sulla propria posizione e quella successiva. Invia il tutto al server.
 */

void sendAvailability(int fd, char *myPos, char *nextPos) {

    char *buffer = malloc(sizeof(char) * 20);
    sprintf(buffer, "%s|%s|%s", Tname, myPos, nextPos);
    write(fd, buffer, 100);

}

/*
 *readCheckAvailability(int fd) legge se il server gli consente di avanzare
 * oppure di stare fermo e attendere. (0 attendi, 1 avanza).
 */

int readCheckAvailability(int fd) {
    char str[200];
    read(fd, str, sizeof(char) * 200); /* Read lines until end-of-input */

    printf("%s\n", str); /* Echo line from socket */
    int ris = convert(str);
    return ris;
}




