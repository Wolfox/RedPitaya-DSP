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

#include <stdint.h>

/*
* GPIO pinout (https://pinout.xyz)
* GPIO - Physical
*  00  -   27
*  01  -   28
*  02  -   3
*  03  -   5
*  04  -   7
*  05  -   29
*  06  -   31
*  07  -   26
*  08  -   24
*  09  -   21
*  10  -   19
*  11  -   23
*  12  -   32
*  13  -   33
*  14  -   8
*  15  -   10
*  16  -   36
*  17  -   11
*  18  -   12
*  19  -   35
*  20  -   38
*  21  -   40
*  22  -   15
*  23  -   16
*  24  -   18
*  25  -   22
*  26  -   37
*  27  -   13
* WARNING: Some pins [0-8] have high pull
*/

#define BCM2708_PERI_BASE        0x3F000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x00200000) /* GPIO controller */

// From the 54 GPIO lines in the Raspberry Pi,
// only 28 are available in its board
#define GPIO_FUNCTION_SELECT    0
#define GPIO_FUNCTION_SELECT_0  0
#define GPIO_FUNCTION_SELECT_1  1
#define GPIO_FUNCTION_SELECT_2  2
// #define GPIO_FUNCTION_SELECT_3  3
// #define GPIO_FUNCTION_SELECT_4  4
// #define GPIO_FUNCTION_SELECT_5  5
#define GPIO_OUTPUT_SET         7
// #define GPIO_OUTPUT_SET_0       7
// #define GPIO_OUTPUT_SET_1       8
#define GPIO_OUTPUT_CLEAR       10
// #define GPIO_OUTPUT_CLEAR_0     10
// #define GPIO_OUTPUT_CLEAR_1     11
#define GPIO_LEVEL              13
// #define GPIO_LEVEL_0            13
// #define GPIO_LEVEL_1            14

int initGPIO();
int exitGPIO();

int setSignal(int pin, int act, volatile uint32_t **addr, uint32_t *val);

volatile uint32_t * getPinFunctionSelector(int pinNumber);
