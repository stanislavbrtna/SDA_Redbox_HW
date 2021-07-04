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

/*
 * touch.h
 *
 *  Created on: 5. 11. 2016
 *      Author: stanislaw
 */

#ifndef TOUCH_TOUCH_H_
#define TOUCH_TOUCH_H_

typedef struct {
	float a;
	float b;
	float c;
	float d;
	float e;
} touchCalibDataStruct;

#include "../main.h"

typedef struct {
	uint16_t x;
	uint16_t y;
} touchXY;

extern touchCalibDataStruct touchCalibData;

void touchInit(); // inits touch adcs, inits default calibration values
void touchSleep();
void touchWake();

void sda_setLcdCalibrationFlag(uint8_t val);

uint8_t svp_getLcdCalibrationFlag();

uint8_t touch_get_xy(touchXY *result); // returns if touch occurred, with coordinates in touch XY

void sda_calibrate();

#ifdef TOUCH_USE_BATTERY_MEASUREMENT
uint16_t getBatteryVoltage();
#endif

#endif /* TOUCH_TOUCH_H_ */
