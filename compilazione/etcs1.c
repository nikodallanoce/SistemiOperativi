#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <time.h>
#include <stdatomic.h>
#include <sys/file.h>
#include "trainUtils.h"


void ciufCiuf(char *Tname);

int main(int argc, char *argv[]) {

    synchro(argv);
    ciufCiuf(argv[0]);

    return 0;
}

/*
 * ciufCiuf(char *Tname) controlla il treno:
 * se la boa successiva è libera si muove, altrimenti
 * resta fermo e attende.
 */

void ciufCiuf(char *Tname) {
    cleanLog(Tname);
    printf("%s ", Tname);
    char **route = TRoute(Tname);

    printf("Start : %s ", Tname);
    printStringArray(route);//*********

    for (int i = 1; i < getRouteLength(); i++) {
        char *nextPos = route[i];
        char *myPos = route[i - 1];
        int *files = lockFile(myPos, nextPos);
        if (readAvailability(files[1]) == 0) {

            takePosition(files[1]);
            createLog(route[i - 1], nextPos, Tname);
            leavePosition(files[0]);
        } else {
            createLog("waiting", nextPos, Tname);
            i--;
        }
        unlockFile(files[0], files[1]);
        sleep(3);

    }
}
