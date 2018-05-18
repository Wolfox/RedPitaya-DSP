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

#include <stdint.h>

/*
* struct actionLine
* nsec - action time, in nano seconds
* pin - pin number
* action - action:
    1: set pin on;
    0: clear pin off;
    -1: wait for pin to be high;
    -2: wait for pin to be low:
    -3: wait for pin to change
* clocks - action time, in clock ticks
* actAddr - pin's virtual address to write the action
    (given by gpioControl)
* actVal - value to write in order to execute the action on the pin
    (given by gpioControl)
* executedTime - ~time when the action was executed
    (CURRENTLY NOT IMPLEMENT, due to time optimization)
*/
typedef struct {
    uint64_t nsec;
    int pin;
    int action;
    uint64_t clocks;
    volatile uint32_t * pinAddr;
    uint32_t valToWrit;
    // unsigned uint64_t executedTime;
} actionLine;

actionLine *actionTable;

long createActionTable(char *path);
