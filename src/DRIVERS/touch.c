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

#include "touch.h"

// #define TOUCH_DBG
#include "../main.h"

touchCalibDataStruct touchCalibData;

extern ADC_HandleTypeDef g_AdcHandle;
extern ADC_HandleTypeDef g_AdcHandle2;


static void touch_delay(__IO uint32_t nCount);
static void touchAdcInit(void);

static void touch_delay(__IO uint32_t nCount) {
  for(; nCount != 0; nCount--);
}

uint8_t calibrationFlag;

void sda_setLcdCalibrationFlag(uint8_t val) {
	calibrationFlag = val;
}

uint8_t svp_getLcdCalibrationFlag() {
	return calibrationFlag;
}

static void touchAdcInit(void) {
	__GPIOB_CLK_ENABLE();
	__GPIOA_CLK_ENABLE();
	__ADC1_CLK_ENABLE();
	g_AdcHandle.Instance = ADC1;

	g_AdcHandle.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV2;
	g_AdcHandle.Init.Resolution            = ADC_RESOLUTION_12B;
	g_AdcHandle.Init.ScanConvMode          = ENABLE;
	g_AdcHandle.Init.ContinuousConvMode    = ENABLE;
	g_AdcHandle.Init.DiscontinuousConvMode = DISABLE;
	g_AdcHandle.Init.NbrOfDiscConversion   = 0;
	g_AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
	g_AdcHandle.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
	g_AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
	g_AdcHandle.Init.NbrOfConversion       = 1;
	g_AdcHandle.Init.DMAContinuousRequests = ENABLE;
	g_AdcHandle.Init.EOCSelection          = DISABLE;

	if(HAL_ADC_Init(&g_AdcHandle) != HAL_OK) {
		printf("touch_dbg: hal init FAIL\n");
	}

	ADC_ChannelConfTypeDef adcChannel;

	adcChannel.Channel = ADC_CHANNEL_6;
	adcChannel.Rank    = 1;
	adcChannel.SamplingTime = ADC_SAMPLETIME_480CYCLES;
	adcChannel.Offset = 0;

	if (HAL_ADC_ConfigChannel(&g_AdcHandle, &adcChannel) != HAL_OK) {
		printf("touch_dbg: hal c6 channel init FAIL\n");
	}

	__ADC2_CLK_ENABLE();
	g_AdcHandle2.Instance = ADC2;

	g_AdcHandle2.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV2;
	g_AdcHandle2.Init.Resolution            = ADC_RESOLUTION_12B;
	g_AdcHandle2.Init.ScanConvMode          = ENABLE;
	g_AdcHandle2.Init.ContinuousConvMode    = ENABLE;
	g_AdcHandle2.Init.DiscontinuousConvMode = DISABLE;
	g_AdcHandle2.Init.NbrOfDiscConversion   = 0;
	g_AdcHandle2.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
	g_AdcHandle2.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
	g_AdcHandle2.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
	g_AdcHandle2.Init.NbrOfConversion       = 1;
	g_AdcHandle2.Init.DMAContinuousRequests = ENABLE;
	g_AdcHandle2.Init.EOCSelection          = DISABLE;

	adcChannel.Channel = ADC_CHANNEL_7;
	adcChannel.Rank    = 1;
	adcChannel.SamplingTime = ADC_SAMPLETIME_480CYCLES;
	adcChannel.Offset = 0;

	if (HAL_ADC_ConfigChannel(&g_AdcHandle2, &adcChannel) != HAL_OK) {
		printf("touch_dbg: hal channel 7 init FAIL\n");
	}

	if(HAL_ADC_Init(&g_AdcHandle2)!= HAL_OK ) {
		printf("touch_dbg: hal init FAIL\n");
	}
}

#ifdef TOUCH_USE_BATTERY_MEASUREMENT
uint16_t getBatteryVoltage() {
	ADC_ChannelConfTypeDef adcChannel;
	GPIO_InitTypeDef gpioInit;
	__GPIOB_CLK_ENABLE();
	__GPIOA_CLK_ENABLE();
	__ADC1_CLK_ENABLE();
	__ADC2_CLK_ENABLE();

	// Deinit the ADCs, init battery measurement setup,
	// measure, then init everything back
	g_AdcHandle2.Instance = ADC2;
	HAL_ADC_DeInit(&g_AdcHandle2);
	uint16_t measuredVoltage = 0;

	g_AdcHandle2.Instance                   = ADC2;
	g_AdcHandle2.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV2;
	g_AdcHandle2.Init.Resolution            = ADC_RESOLUTION_12B;
	g_AdcHandle2.Init.ScanConvMode          = ENABLE;
	g_AdcHandle2.Init.ContinuousConvMode    = ENABLE;
	g_AdcHandle2.Init.DiscontinuousConvMode = DISABLE;
	g_AdcHandle2.Init.NbrOfDiscConversion   = 0;
	g_AdcHandle2.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
	g_AdcHandle2.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
	g_AdcHandle2.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
	g_AdcHandle2.Init.NbrOfConversion       = 1;
	g_AdcHandle2.Init.DMAContinuousRequests = ENABLE;
	g_AdcHandle2.Init.EOCSelection          = DISABLE;

	//ADC2 in 8 - batt
	adcChannel.Channel = ADC_CHANNEL_8;
	adcChannel.Rank    = 1;
	adcChannel.SamplingTime = ADC_SAMPLETIME_480CYCLES;
	adcChannel.Offset = 0;

	if(HAL_ADC_Init(&g_AdcHandle2) != HAL_OK) {
		printf("batt_dbg: hal init FAIL\n");
	}

	if (HAL_ADC_ConfigChannel(&g_AdcHandle2, &adcChannel) != HAL_OK) {
		printf("batt_dbg: hal channel 7 init FAIL\n");
	}

	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_6);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_7);

	//PB0 ain - battery
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0);
	gpioInit.Pin   = GPIO_PIN_0;
	gpioInit.Mode  = GPIO_MODE_ANALOG;
	gpioInit.Pull  = GPIO_NOPULL;
	gpioInit.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOB, &gpioInit);

	HAL_ADC_Start(&g_AdcHandle2);
	if (HAL_ADC_PollForConversion(&g_AdcHandle2, 1000000) == HAL_OK) {
		measuredVoltage = HAL_ADC_GetValue(&g_AdcHandle2);
	}
	HAL_ADC_Stop(&g_AdcHandle2);

	g_AdcHandle.Instance = ADC1;

	HAL_ADC_DeInit(&g_AdcHandle);
	HAL_ADC_DeInit(&g_AdcHandle2);

	touchAdcInit();

	return measuredVoltage;
}
#endif

void touchInit() {
	touchAdcInit();

	// defaults for calibration
	touchCalibData.a = 0.116;
	touchCalibData.b = -75;

	touchCalibData.c = 0.156;
	touchCalibData.d = -75;

	touchCalibData.e = +0.05;
}

void touchSleep() {
	__ADC1_CLK_DISABLE();
	__ADC2_CLK_DISABLE();
}
void touchWake() {
	__ADC1_CLK_ENABLE();
	__ADC2_CLK_ENABLE();
}

uint8_t touch_read_xy(touchXY *result) {
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitTypeDef gpioInit;
	volatile uint32_t x = 0;
	volatile uint32_t y = 0;
	uint8_t valid = 0;

	//PA6 ain
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_6);
	gpioInit.Pin   = GPIO_PIN_6;
	gpioInit.Mode  = GPIO_MODE_ANALOG;
	gpioInit.Pull  = GPIO_NOPULL;
	gpioInit.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOA, &gpioInit);

	//PA7 gnd
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_7);
	GPIO_InitStructure.Pin   = GPIO_PIN_7;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull  = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);

	//PB8 in
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8);
	GPIO_InitStructure.Pin   = GPIO_PIN_8;
	GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull  = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

	//PB9 out
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_9);
	GPIO_InitStructure.Pin   = GPIO_PIN_9;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull  = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);

	HAL_ADC_Start(&g_AdcHandle);

	if (HAL_ADC_PollForConversion(&g_AdcHandle, 1000000) == HAL_OK) {
		x = HAL_ADC_GetValue(&g_AdcHandle);
	}
	HAL_ADC_Stop(&g_AdcHandle);

	//PA6 gnd
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_6);
	gpioInit.Pin   = GPIO_PIN_6;
	gpioInit.Mode  = GPIO_MODE_OUTPUT_PP;
	gpioInit.Pull  = GPIO_NOPULL;
	gpioInit.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOA, &gpioInit);

	//PA7 gnd
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_7);
	GPIO_InitStructure.Pin   = GPIO_PIN_7;
	GPIO_InitStructure.Mode  = GPIO_MODE_ANALOG;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull  = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);

	//PB8 out
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8);
	GPIO_InitStructure.Pin   = GPIO_PIN_8;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull  = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

	//PB9 in
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_9);
	GPIO_InitStructure.Pin   = GPIO_PIN_9;
	GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull  = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);

	HAL_ADC_Start(&g_AdcHandle2);

	if (HAL_ADC_PollForConversion(&g_AdcHandle2, 1000000) == HAL_OK){
		y = HAL_ADC_GetValue(&g_AdcHandle2);
	}
	HAL_ADC_Stop(&g_AdcHandle2);

	valid = 1; // result is at this point always valid

	result->x = x;
	result->y = y;

	return valid;
}

uint8_t touch_read_xy_m30(touchXY *result) {
	touchXY prac;
	uint16_t x[31];
	uint16_t y[31];
	uint16_t i, j, temp;

	for (i = 0; i < 30; i++) {
		x[i] = 0;
		y[i] = 0;
	}

	for (i = 0; i < 30; i++) {
		prac.x = 0;
		prac.y = 0;
		if (touch_read_xy(&prac)) {
			x[i] = prac.x;
			y[i] = prac.y;
		} else {
			return 0;
		}
	}

	for (i = 0; i < 30 - 1; i++) {
		for (j = i + 1; j < 30; j++) {
			if (x[j] < x[i]) {
				// swap elements
				temp = x[i];
				x[i] = x[j];
				x[j] = temp;
			}
		}
	}

	for(i = 0; i < 30 - 1; i++) {
		for(j = i + 1; j < 30; j++) {
			if(y[j] < y[i]) {
				// swap elements
				temp = y[i];
				y[i] = y[j];
				y[j] = temp;
			}
		}
	}

	result->x = x[15];
	result->y = y[15];
	// validation values were
	if ((result->x < 3700)
			&& (result->y < 4000)
			&& (result->x > 350)
			&& (result->y > 350)) {
		return 1;
	} else {
		return 0;
	}
}

uint8_t touch_get_xy(touchXY *result) {
	touchXY res;
	float a, b, c, d, e;

	a = touchCalibData.a;
	b = touchCalibData.b;
	c = touchCalibData.c;
	d = touchCalibData.d;
	e = touchCalibData.e;

	// načtem a pokud dotyk, tak počkáme
	if (touch_read_xy_m30(&res)) {
		touch_delay(300000);
		// načteme dotyk po uklidnění displeje
		if (touch_read_xy_m30(&res)) {

			// hlavní výpočet
			result->x = a * res.x + b;
			result->y = c * res.y + d;

			// fix horní část lcd
			if ((result->y > 0) && (result->y < 140)) {
				result->y = c * res.y + e * result->x + d;
#ifdef TOUCH_DBG
				printf("Touch: Additional fix area\n");
#endif
			}
			#ifdef TOUCH_DBG
				else
				printf("Touch: Normal area\n");
			#endif


			//fix limitů
			if (result->y == 0) {
				result->y = 1;
			}

			if ((result->x) > 320) {
				result->x = 318;
			}

			if (result->y > 480) {
				result->y = 478;
			}

#ifdef TOUCH_DBG
			printf("Touch: x:%u y:%u out: x:%u y:%u \n",
					res.x,
					res.y,
					result->x,
					result->y
			);
			LCD_set_XY(result->x, result->y, result->x, result->y);
			LCD_draw_point_wrp(0xFFFF);
#endif
			return 1;

		} else {
			return 0;
		}

	} else {
		return 0;
	}
}

void svp_set_calibration_data(touchCalibDataStruct input) {
	touchCalibData.a = input.a;
	touchCalibData.b = input.b;
	touchCalibData.c = input.c;
	touchCalibData.d = input.d;
	touchCalibData.e = input.e;
}

void touch_draw_cpoint(uint16_t x, uint16_t y, uint16_t color) {
	LCD_DrawLine(x, y - 5, x, y + 5, color);
	LCD_DrawLine(x-  5, y, x + 5, y, color);
	LCD_DrawRectangle(x - 1, y - 1, x + 1, y + 1, color);
}

#define CPOINT1_Y 80

// Calibration routine
void sda_calibrate () {
	touchXY res;
	float rx1, ry1, rx2, ry2;
	float a, b, c, d, e;

	while (1) {
		LCD_setDrawArea(0, 0, 319, 479);
		LCD_Fill(0x0000);

		touch_draw_cpoint(40, CPOINT1_Y, LCD_MixColor(255, 0, 0));

		touch_draw_cpoint(280, 440, 0xFFFF);

		touch_draw_cpoint(120, 40, 0xFFFF);

		// RX1
		while (touch_read_xy_m30(&res) == 0);
		touch_delay(300000);
		touch_read_xy_m30(&res);
		rx1 = (float) res.x;
		ry1 = (float) res.y;

		while (touch_read_xy_m30(&res) == 1);
		touch_delay(300000);

		// RX2
		touch_draw_cpoint(40, CPOINT1_Y, 0xFFFF);

		touch_draw_cpoint(280, 440, LCD_MixColor(255, 0, 0));

		while (touch_read_xy_m30(&res) == 0);
		touch_delay(300000);
		touch_read_xy_m30(&res);
		rx2 = (float) res.x;
		ry2 = (float) res.y;

		// compute
		c = (440 - CPOINT1_Y) / (ry2 - ry1);
		a = (280 - 40) / (rx2 - rx1);

		b = 40 - a * rx1;
		d = CPOINT1_Y - c * ry1;

		while (touch_read_xy_m30(&res) == 1);
		touch_delay(300000);

		// E
		touch_draw_cpoint(280, 440, 0xFFFF);

		touch_draw_cpoint(120, 40, LCD_MixColor(255, 0, 0));

		while (touch_read_xy_m30(&res) == 0);
		touch_delay(300000);
		touch_read_xy_m30(&res);
		rx1 = (float) res.x;
		ry1 = (float) res.y;

		e = (40 - c * ry1 - d) / rx1;

		printf("calibration done: \n a:%u\n b:%u\n c:%u\n d:%u\n e:%u \n",
				(uint16_t)(a * 1000),
				(uint16_t)(b * -1),
				(uint16_t)(c * 1000),
				(uint16_t)(d * -1),
				(uint16_t)(e * 1000)
		);

		touchCalibData.a = a;
		touchCalibData.b = b;
		touchCalibData.c = c;
		touchCalibData.d = d;
		touchCalibData.e = e;

		LCD_Fill(0x0000);
#ifndef LANG_VAL
		LCD_DrawText_ext(80, 240, 0xFFFF, (uint8_t *)"Test Kalibrace");
		LCD_DrawText_ext(10, 420, 0xFFFF, (uint8_t *)"Rekalibrovat");
		LCD_DrawText_ext(280, 420, 0xFFFF, (uint8_t *)"Ok");
#else
#if LANG_VAL==0
		LCD_DrawText_ext(80, 240, 0xFFFF, (uint8_t *)"Test Kalibrace");
		LCD_DrawText_ext(40, 260, 0xFFFF, (uint8_t *)"(Použijte tlačítka k potvrzení)");
		LCD_DrawText_ext(10, 420, 0xFFFF, (uint8_t *)"Rekalibrovat");
		LCD_DrawText_ext(280, 420, 0xFFFF, (uint8_t *)"Ok");
#endif

#if LANG_VAL==1
		LCD_DrawText_ext(80, 240, 0xFFFF, (uint8_t *)"Calibration test");
		LCD_DrawText_ext(40, 260, 0xFFFF, (uint8_t *)"(use buttons to select option)");
		LCD_DrawText_ext(10, 420, 0xFFFF, (uint8_t *)"Calibrate again");
		LCD_DrawText_ext(280, 420, 0xFFFF, (uint8_t *)"Ok");
#endif

#endif

		while (1) {
			if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_SET) {
				break;
			}
			if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15) == GPIO_PIN_SET) {
				return;
			}
			if(touch_get_xy(&res)) {
				LCD_DrawPoint(res.x, res.y, 0xFFF);
			}
		}
	}
}

