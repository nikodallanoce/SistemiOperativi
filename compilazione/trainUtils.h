//
// Created by root on 20/07/18.
//

#include <bits/types/FILE.h>

#ifndef PROGETTOCECCA_TRAINUTILS_H
#define PROGETTOCECCA_TRAINUTILS_H

#endif //PROGETTOCECCA_TRAINUTILS_H

void synchro(char *argv[]);

char **TRoute(char *Tname);

char *firstFileRow(FILE *toRead);

int readAvailability(int toOpen);

void takePosition(int toOpen);

void leavePosition(int toOpen);

char *logName(char *TName);

void createLog(char *pos, char *nextPos, char *TName);

char *dateNow();

void cleanLog(char *TName);

void unlockFile(int myFile, int nextFile);

int getRouteLength();

int *lockFile(char *myFile, char *nextFile);

void printStringArray(char **array);

int isStation(char *segment);

int isNumber(char c);

int convert(char *number);

int convertSegment(char *segment);