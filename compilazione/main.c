#include <stdio.h>
#include <wait.h>
#include <unistd.h>
#include <malloc.h>
#include <memory.h>
#include <stdlib.h>
#include <time.h>

void gas(int childProcesses, char *param);

void serverRun();

int isServer(char *argv[]);

void createSegment(int segNumber);

int main(int argc, char *argv[]);

/*
 * main(...) permette la selezione delle tre modalita di avvio,
 * in base a al contenuto di argv: etcs1, etcs2, etcs2 rbc.
 *
 */

int main(int argc, char *argv[]) {

    if (isServer(argv)) {
        serverRun();
    } else {
        char *param = argv[1];
        printf("%s", param);

        int childProcesses = 5;
        int segNumber = 16;
        createSegment(segNumber);

        gas(childProcesses, param);
    }
    return 0;
}

/*
 * isServer(...) restituisce 0 se argv non contiene il comando
 * per lanciare il server, altimenti un valore positivo.
 */

int isServer(char *argv[]) {
    int serv = 0;
    if (argv[2] != NULL) {
        serv = strcmp(argv[2], "RBC") == 0 || strcmp(argv[2], "rbc") == 0;
    }
    return serv;
}

/*
 * serverRun() lancia il server.
 */
void serverRun() {

    int pidServer = fork();
    if (pidServer == 0) {
        execl("RBC", NULL);
    }
}

/*
 * gas(...) crea i processi figli assegnandogli un nome e fornisce i tempi di avvio
 * permettendogli di avviarsi contemporaneamente.
 */

void gas(int childProcesses, char *param) {
    // pid_t child_pid, wpid;
    int status;
    pid_t child_pids[childProcesses];
    long timeLimit = 100000;
    char *TName = malloc(20);
    for (int i = 1; i <= childProcesses; i++) {

        clock_t time = clock();
        long double timeMicro = (((long double) time / CLOCKS_PER_SEC)) * 1000 * 1000;
        printf("clock: %Lf\n", timeMicro);
        fflush(stdout);

        char *waitTime = malloc(10 * sizeof(int));

        int pid = child_pids[i - 1] = fork();
        if (pid == 0) {
            long int mill = (long int) timeMicro;
            timeLimit -= mill;
            sprintf(waitTime, "%li", timeLimit);
            sprintf(TName, "T%d", i);
            execl(param, TName, waitTime, NULL);

        }
        //sleep(1);
    }

    wait(NULL);

    for (int i = 0; i < childProcesses; ++i) {
        int status;

        waitpid(child_pids[i], &status, 0);

        printf("PARENT: Child: %d returned value is: %d\n", child_pids[i], WEXITSTATUS(status));
    }

}

/*
 *createSegment(...) crea un numero di file boa MAx,
 * tanti quanto vale segNumber.
 */

void createSegment(int segNumber) {

    char *seg = malloc(sizeof(char) * 5);

    for (int i = 1; i <= segNumber; i++) {
        sprintf(seg, "MA%d", i);
        FILE *file = fopen(seg, "w+");
        fprintf(file, "0");
        fclose(file);

    }
    free(seg);
}