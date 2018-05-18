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

#include "gpioControl.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>

#define BLOCK_SIZE (4*1024)
#define MAX_PIN 28

// I/O access
volatile uint32_t *gpio;

int initGPIO() {
    int  mem_fd = open("/dev/mem", O_RDWR|O_SYNC); // open /dev/mem
    if (mem_fd < 0) {
        printf("can't open /dev/mem \n");
        return 1;
    }

    /* mmap GPIO */
    gpio = mmap(
        NULL,             //Any adddress in our space will do
        BLOCK_SIZE,       //Map length
        PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
        MAP_SHARED,       //Shared with other processes
        mem_fd,           //File to map
        GPIO_BASE         //Offset to GPIO peripheral
        );

    close(mem_fd); //No need to keep mem_fd open after mmap

    if (gpio == MAP_FAILED) {
        printf("mmap error 0x%08x\n", (uint32_t)GPIO_BASE);//errno also set!
        return 1;
    }

    //Before using pin as output mode(001), must be config to input mode(000)
    // GPIO pins 00-09
    gpio[GPIO_FUNCTION_SELECT_0] &= 0xC0000000;
    gpio[GPIO_FUNCTION_SELECT_0] |= 0x09249249;
    // GPIO pins 10-19
    gpio[GPIO_FUNCTION_SELECT_1] &= 0xC0000000;
    gpio[GPIO_FUNCTION_SELECT_1] |= 0x09249249;
    // GPIO pins 20-27
    gpio[GPIO_FUNCTION_SELECT_2] &= 0xFF000000;
    gpio[GPIO_FUNCTION_SELECT_2] |= 0x00249249;

    // Clear all 28 pins
    gpio[GPIO_OUTPUT_CLEAR] = 0x0FFFFFFF;

    return 0;
}

int exitGPIO() {
    gpio[GPIO_OUTPUT_CLEAR] = 0x0FFFFFFF;
    return 0;
}

int setSignal(int pin, int action, volatile uint32_t **addr, uint32_t *val) {
    int returnVal = 0;
    if (pin<0) {
        // reserved for analogue output, NOT IMPLEMENTED
        printf("pin number was negative (%d)\n", pin);
        return -1;
    } else {
        if (pin > MAX_PIN) {
            printf("pin was bigger than %d (%d)", MAX_PIN, pin);
            return -1;
        }

        if (action < 0) {
            // input - TODO use GPIO Event Detect Status Registers
            // action -1 = wait for signal to be 1
            // action -2 = wait for signal to be 0
            // action -3 = wait for edge
            if (action < -3) {
                printf("WARNING! Action was %i. Digital input action should be -1, -2 or -3 (-3 apply by default)\n", action);
                action = -3;
                returnVal = 1;
            }
            *addr = &gpio[GPIO_LEVEL];
        } else {
            // output
            // action 0 = clear pin
            // action 1 = set pin
            if (action > 1) {
                printf("WARNING! Action was %i. Digital output action should be 1 or 0 (1 apply by default)\n", action);
                action = 1;
                returnVal = 1;
            }
            if (action) {
                *addr = &gpio[GPIO_OUTPUT_SET];
            } else {
                *addr = &gpio[GPIO_OUTPUT_CLEAR];
            }
        }
        *val = (1<<pin);
    }

    return returnVal;
}

volatile uint32_t * getPinFunctionSelector(int pinNumber) {
    int offset = pinNumber/10;
    return (volatile uint32_t *) &gpio[GPIO_FUNCTION_SELECT+offset];
}
