/*
Copyright (c) 2018 Stanislav Brtna

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef MAIN_H
#define MAIN_H

#define TOUCH_USE_BATTERY_MEASUREMENT

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_cortex.h"
#include "DRIVERS/touch.h"
#include "DRIVERS/speaker.h"
#include "DRIVERS/lcd.h"
#include "DRIVERS/rtc.h"
#include "DRIVERS/usart.h"
#include "DRIVERS/usart3.h"
#include "DRIVERS/power_management.h"
#include "FATFS/ff.h"
#include "sda_fs_umc.h"
#include "SDA_OS/SDA_OS.h"
#include "SDA_OS/SVS/svs_basics.h"
#include "hw_misc.h"


extern FATFS FatFs;

typedef enum {REV_0, REV_1} BoardRevType;

void __initialize_hardware();

#endif
