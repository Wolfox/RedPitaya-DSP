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

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>

#include "timeControl.h"
#include "gpioControl.h"
#include "actionTable.h"

void initializeAll();
void maxPriority();
int execActionTable(long lines);
void sig_handler(int signo);
void _exit(int status);

/******************************************************************************/

int main(int argc, char *argv[]) {
    printf("Hello world!\n");

    if (argc != 2) {
        printf("num of args used was %u\n", argc);
        printf("USAGE: dsp \"action-table-file\"\n");
        _exit(1);
    }

    initializeAll();

    long int lines = createActionTable(argv[1]);
    if (lines < 0) {
        printf("Failed to read/create action table.\n");
        _exit(abs(lines));
    }

    // before we set ourselves to more important than the terminal, flush.
    fflush(stdout);

    maxPriority();

    int execStatus = execActionTable(lines);
    if (execStatus < 0) {
        printf("Failed to execute ActionTable (%i).\n", execstatus);
        _exit(5);
    }
    printf("ActionTable executed.\n");
    _exit(0);
}

/******************************************************************************/

void initializeAll() {
    printf("Initializing!\n");

    actionTable = NULL;

    if (initARMTimer() != 0) {
        printf("init ARM Timer access failed\n");
        _exit(2);
    }

    if (initGPIO() != 0) {
        printf("init GPIO access failed\n");
        _exit(2);
    }

    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        printf("Signal handler failed\n");
    }
}

void maxPriority() {
    struct sched_param params;
    // params.sched_priority = sched_get_priority_max(SCHED_FIFO);
    params.sched_priority = 99;
    if (sched_setscheduler(0, SCHED_FIFO, &params) == -1){
        printf("Failed to set priority.\n");
        _exit(4);
    }
}

int execActionTable(long lines) {
    printf("executing ActionTable\n");
    long line = 0;
    startARMTimer();

    printf("set time\n");
    uint64_t nextTime = 0;
    updateARMTimer();
    uint64_t startTime = currentTime;

    for (line = 0 ; line < lines; line++) {
        actionLine actLine = actionTable[line];
        nextTime = startTime + actLine.clocks;

        setNextTime(actLine.actionTime);

        if (actLine.action < 0) {
            while (currentTime < nextTime) updateARMTimer();
            while (!(*(actLine.pinAddr)& actLine.valToWrit)) {
                //IT'S ADVENTURE TIME
            }
            updateCurrentTime();
            startTime = currentTime;
        } else {
            while (currentTime < nextTime) updateARMTimer();
            *(actLine.pinAddr) = actLine.valToWrit;
        }
    }

    return 0;
}

/******************************************************************************/

void sig_handler(int signo){
    if (signo == SIGINT) {
        _exit(5);
    }
}

void _exit(int status) {
    free(actionTable);
    /*out_setpins_P(0); //TODO activate this (uncoment)
    out_setpins_N(0);*/
    exit(status);
}
