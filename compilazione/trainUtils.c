//
// Created by root on 20/07/18.
//

#include "trainUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <time.h>
#include <stdatomic.h>
#include <sys/file.h>


int ROUTE_LENGTH = 0;

/*
 * takePosition(int toOpen) dato un file descriptor,
 * si posiziona all inizio del file e ci scrive 1.
 */

void takePosition(int toOpen) {

    if (toOpen == -1) {
        return;
    }
    char ch = '1';
    lseek(toOpen, 0L, SEEK_SET);
    write(toOpen, &ch, 1);
}

/*
 * *lockFile(char *myFile, char *nextFile) restituisce i file descriptor
 *  corrispondenti ai nomi dei files dati in ingresso e ne garantisce
 *  l accesso esclusivo.
 */

int *lockFile(char *myFile, char *nextFile) {
    int *files = malloc(200 * sizeof(int));

    int fdmy = open(myFile, O_RDWR, 0777);
    flock(fdmy, LOCK_EX);
    files[0] = fdmy;

    int fdnext = open(nextFile, O_RDWR, 0777);
    flock(fdnext, LOCK_EX);
    files[1] = fdnext;


    return files;
}

/*
 * unlockFile(int myFile, int nextFile) toglie l esclusivita di accesso
 * ai files corrispondenti ai files descriptor.
 */

void unlockFile(int myFile, int nextFile) {
    if (myFile != -1) {
        close(myFile);

    }
    if (nextFile != -1) {
        close(nextFile);
    }
}

/*
 * *logName(char *TName) restituisce il nome dei file log
 *  dato il nome di un treno e lo concatena con .log
 *  es: T2 -> T2.log
 */

char *logName(char *TName) {
    char *TLog = malloc(20);
    strcpy(TLog, TName);
    strcat(TLog, ".log");
    return TLog;
}

/*
 * cleanLog(char *TName) elimina il vecchio file di log di
 * un treno.
 */

void cleanLog(char *TName) {
    char *TLog = logName(TName);
    if (fopen(TLog, "a") != NULL) {
        fopen(TLog, "w");
    }
}

/*
 * createLog(...) crea il file di log di un treno e lo aggiorna
 * secondo le specifiche del progetto.
 */

void createLog(char *pos, char *nextPos, char *TName) {
    char *TLog = logName(TName);
    FILE *log = fopen(TLog, "a");
    if (log == NULL) {
        log = fopen(TLog, "w");
    }
    fprintf(log, "[Attuale: %s][Next: %s] %s \n", pos, nextPos, dateNow());
    fclose(log);

}

/*
 * *dateNow() ritorna una stringa contenente la data e l ora corrente.
 */

char *dateNow() {

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char *s = malloc(64);
    strftime(s, 64, "%c", tm);

    return s;
}

/*
 * leavePosition(int toOpen) dato un file descriptor, scrive
 * 0 dentro al file corrispondente. Viene usato dai treni per
 * lasciare la posizione corrente.
 */

void leavePosition(int toOpen) {
    if (toOpen == -1) {
        return;
    }
    char ch = '0';
    lseek(toOpen, 0L, SEEK_SET);
    write(toOpen, &ch, 1);
}

/*
 * int readAvailability(int toOpen) restituisce 0 o 1
 * in base a cio che e scritto all interno del file.
 */

int readAvailability(int toOpen) {

    if (toOpen == -1) { return 0; }
    char *buffer = malloc(sizeof(char));
    read(toOpen, buffer, 1);
    int ris = buffer[0] - 48;

    return ris;
}

/*
 * getRouteLength() restituisce la lunghezza del percorso
 * di un treno.
 */

int getRouteLength() {
    return ROUTE_LENGTH;
}

/*
 * char **TRoute(char *Tname) dato il nome di un treno,
 * restituisce un array di stringhe contentente in ciascuna
 * posizione un segmento del percorso. Aggiorna ROUTE_LENGTH.
 */

char **TRoute(char *Tname) {

    FILE *toRead = fopen(Tname, "r");
    char *readBuffer = firstFileRow(toRead);
    char **saves = malloc(200 * sizeof(char));
    char *appo = malloc(20);
    int j = 0;
    for (int i = 0; i <= strlen(readBuffer); i++) {

        if (readBuffer[i] == ',' || readBuffer[i] == '\n' || readBuffer[i] == '\0') {

            saves[j] = malloc(20);
            strcpy(saves[j], appo);
            appo = malloc(20);
            j++;
            ROUTE_LENGTH = j;
            continue;
        }

        char c = {readBuffer[i]};
        if (c != ' ') {
            strncat(appo, &c, 1);
        }

    }
    fclose(toRead);

    return saves;
}

/*
 * *firstFileRow(FILE *toRead) legge la prima riga di un file
 * e la restituisce.
 */

char *firstFileRow(FILE *toRead) {
    char *readBuffer = malloc(sizeof(char) * 200);

    fgets(readBuffer, sizeof(char) * 200, toRead);

    return readBuffer;

}

/*
 * printStringArray(char **array) stampa una matrice.
 */

void printStringArray(char **array) {

    for (int i = 0; i < getRouteLength(); i++) {
        printf("[");
        for (int j = 0; j < strlen(array[i]); j++) {
            char c = array[i][j];
            printf("%c", array[i][j]);
        }
        printf("]");

    }
    printf("\n");
}

/*
 * synchro(char *argv[]) mette in pausa un processo
 * per n microsecondi. n e contenuto in argv[1].
 */

void synchro(char *argv[]) {
    long r = strtol(argv[1], NULL, 10);
    printf("%li\n", r);
    fflush(stdout);
    usleep((__useconds_t) r);

}

/*
 * isStation(char *segment) restituisce un valore positivo
 * se *segment e una stazione, altrimenti zero.
 */

int isStation(char *segment) {
    int ris = segment[0] == 'S';
    return ris;
}

/*
 * digits(char *segName) conta il numero di interi presenti
 * nella stringa *segname.
 */

int digits(char *segName) {
    int i = (int) strlen(segName) - 1;
    char actual = segName[i];
    int count = 0;
    while (isNumber(actual)) {

        i--;
        actual = segName[i];
        count++;
    }
    return count;
}

int isNumber(char c) {
    return c >= 48 && c <= 57;
}

/*
 *convert(char *number) converte una stringa contenente
 * un numero ad intero.
 */

int convert(char *number) {

    char *stringPart;
    int ris = (int) strtol(number, &stringPart, 10);
    return ris;
}

/*
 * convertSegment(char *segment) converte da char a intero
 * il numero del segmento passato. Restituisce -1 se il segmento
 * e una stazione.
 */

int convertSegment(char *segment) {
    if (isStation(segment)) return -1;
    int j = digits(segment) - 1;
    char tmp[j + 1];
    size_t i = strlen(segment) - 1;
    char current = segment[i];

    do {
        tmp[j] = current;
        i--;
        j--;
        current = segment[i];
    } while (isNumber(current));
    int ris = convert(tmp);
    return ris;
}

