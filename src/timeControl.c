/*
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
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "timeControl.h"

#define BILLION 1000000000 // nano seconds in a second
#define BLOCK_SIZE (4*1024)

// nSec per clock tick
const long double nSecPerTick = BILLION/(long double)PROC_FREQ;

// clock access
volatile uint32_t *ARMControl;
volatile uint32_t *ARMTimer;

/******************************************************************************/

int initARMTimer() {
    int  memoryFileDescriptor = open("/dev/mem", O_RDWR|O_SYNC);
    if (memoryFileDescriptor < 0) {
        printf("can't open /dev/mem (errno %d) \n", errno);
        return 1;
    }

    /* mmap ARM Timer */
    ARMControl = mmap(
        NULL,
        BLOCK_SIZE,             // Map length
        PROT_READ|PROT_WRITE,   // Enable reading & writting to mapped memory
        MAP_SHARED,             // Shared with other processes
        memoryFileDescriptor,   // File to map
        ARM_QA7_CONTROL_REG     // Offset to ARM control logic
    );
    close(memoryFileDescriptor);

    if (ARMControl == MAP_FAILED) {
        printf("mmap(0x%08x) failed (errno %d)\n",
            (uint32_t)ARM_QA7_CONTROL_REG, errno);
        return 1;
    }
    currentTime = 0;
    ARMTimer = (volatile uint32_t *)&currentTime;

    return 0;
}

void testARMTimer() {
    uint64_t previousTime = 0;
    while(1) {
        updateARMTimer();
        if(currentTime < previousTime) {
            printf("UPPSY DAISY!\n");
            printf("prev: %llu\n",(long long unsigned int) previousTime);
            printf("curr: %llu\n",(long long unsigned int) currentTime);
            return;
        }
        previousTime = currentTime;
    }
}

void updateARMTimer() {
    /*
    When reading the current 32 LS bit of the 64 timer, returns it and
    triggers storing a copy of the MS 32 bits

    When reading the current 32 MS bit of the 64 timer, returns the
    status of the core timer-read-hold register.
    That register is loaded when the user does a read of the LS-32 timer bits.
    There is little sense in reading this register without first doing a read
    from the LS-32 bit register.
    */
    ARMTimer[0] = ARMControl[CLOCK_LSB];
    ARMTimer[1] = ARMControl[CLOCK_MSB];
    /* Otherwise use
    do {
        ARMTimer[1] = *(volatile uint32_t *) ARMControl[CLOCK_MSB];
        ARMTimer[0] = *(volatile uint32_t *) ARMControl[CLOCK_LSB];
    } while (*(volatile uint32_t *) ARMControl[CLOCK_MSB] != ARMTimer[1]);
    */
}

uint64_t turnNSecToTicks(unsigned long long int nSec) {
    return (uint64_t) (long long int) (nSec/nSecPerTick);
}
