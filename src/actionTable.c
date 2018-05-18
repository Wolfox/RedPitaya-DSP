/*
* Copyright (C) 2015 Tom Parks
* Copyright (C) 2017-2018 Tiago Susano Pinto <tiagosusanopinto@gmail.com>
*
* This file is part of RedPitaya-DSP.
*
* RedPitaya-DSP is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* RedPitaya-DSP is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with RedPitaya-DSP. If not, see <http://www.gnu.org/licenses/>.
*/

#include <actionTable.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gpioControl.h"
#include "timeControl.h"

#define MAXLINELEN 50
#define DELIM " "
#define PRINT_READ_LINES 0

int readActionTable(FILE *fp, long lines);
int readActionLine(char *line, long lineNum);

/******************************************************************************/

long int createActionTable(char *path) {
    FILE *fp;
    fp = fopen(path, "r"); //open file
    if (fp == NULL) {
        printf("Could not open file %s\n", path);
        return -2;
    }

    long int lines = 0;
    int ch;
    while (EOF != (ch=getc(fp))) { // counting lines
        if (ch=='\n') ++lines;
    }
    printf("ActionTable is %lu lines long\n", lines);
    rewind(fp);

    printf("Memory allocation for ActionTable...");
    actionTable = malloc(sizeof(actionLine)*lines);
    if (actionTable == NULL) {
        printf(" Failed\n");
        return -1;
    }
    printf(" Completed\n");


    if(readActionTable(fp, lines) != 0) {
        printf("Failed to read action table file.\n");
        return -3;
    }
    fclose(fp);
    printf("read action table file.\n");

    return lines;
}

int readActionTable(FILE *fp, long lines) {
    long lineNum;
    size_t lineLength = (size_t)MAXLINELEN;
    char *lineStr = (char *) malloc(sizeof(char)*(lineLength+1));

    for(lineNum = 0; lineNum < lines; lineNum++){
        if(getline(&lineStr, &lineLength, fp) <= 0){
            printf("read %lu lines of action table, but was empty\n", lineNum);
            free(lineStr);
            return -1;
        }
        if (readActionLine(lineStr, lineNum) < 0){
            printf("read ActionLine failed on %lu\n", lineNum);
            free(lineStr);
            return -1;
        }
    }
    free(lineStr);
    return 0;
}

int readActionLine(char *line, long lineNum) {
    char *nstime_s;
    char *pin_s;
    char *act_s;

    if(PRINT_READ_LINES) {
        printf("processing line num %lu - '%s", lineNum, line);
    }

// REMINDER: first strtok call needs the string.
    nstime_s = strtok(line, DELIM);
    if (nstime_s == NULL){
        printf("action nstime is NULL for '%s'\n", line);
        return -1;
    }
    pin_s = strtok(NULL, DELIM);
    if (pin_s == NULL){
        printf("pin is NULL for %s\n", line);
        return -1;
    }
    act_s = strtok(NULL, DELIM);
    if (act_s == NULL){
        printf("act is NULL for %s\n", line);
        return -1;
    }

    actionTable[lineNum].nsec = strtoull(nstime_s, NULL, 10);
    actionTable[lineNum].pin = atoi(pin_s);
    actionTable[lineNum].action = atoi(act_s);

    actionTable[lineNum].clocks = turnNSecToTicks(actionTable[lineNum].nsec);

    setSignal(actionTable[lineNum].pin, actionTable[lineNum].action,
        &actionTable[lineNum].pinAddr, &actionTable[lineNum].valToWrit);
    // actionTable[lineNum].executedTime = 0;

    if(PRINT_READ_LINES) {
        printf("row:%lu time:%llu pin:%i (@0x%p) action:%i (0x%08x)",
            lineNum, // row
            (unsigned long long int) actionTable[lineNum].clocks,
            actionTable[lineNum].pin, actionTable[lineNum].pinAddr,
            actionTable[lineNum].action,
            (unsigned int)actionTable[lineNum].valToWrit);
    }
    return 0;
}
