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

#define TOUCH_DBG
//#include "../main.h"

extern ADC_HandleTypeDef g_AdcHandle;
extern ADC_HandleTypeDef g_AdcHandle2;


static void touchAdcInit(void);

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

void touchAdcInitExt(void) {
	touchAdcInit();
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

void touchSleep() {
	__ADC1_CLK_DISABLE();
	__ADC2_CLK_DISABLE();
}


void touchWake() {
	__ADC1_CLK_ENABLE();
	__ADC2_CLK_ENABLE();
}


void touchInit() {
	touchAdcInit();
}

static void set_ain(GPIO_TypeDef *port, uint32_t pin){
	GPIO_InitTypeDef gpioInit;
	HAL_GPIO_DeInit(port, pin);
	gpioInit.Pin   = pin;
	gpioInit.Mode  = GPIO_MODE_ANALOG;
	gpioInit.Pull  = GPIO_NOPULL;
	gpioInit.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(port, &gpioInit);
}


static void set_gnd(GPIO_TypeDef *port, uint32_t pin) {
	GPIO_InitTypeDef gpioInit;
	HAL_GPIO_DeInit(port, pin);
	gpioInit.Pin   = pin;
	gpioInit.Mode  = GPIO_MODE_OUTPUT_PP;
	gpioInit.Speed = GPIO_SPEED_HIGH;
	gpioInit.Pull  = GPIO_NOPULL;
	HAL_GPIO_Init(port, &gpioInit);
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}


static void set_in(GPIO_TypeDef *port, uint32_t pin) {
	GPIO_InitTypeDef gpioInit;
	HAL_GPIO_DeInit(port, pin);
	gpioInit.Pin   = pin;
	gpioInit.Mode  = GPIO_MODE_INPUT;
	gpioInit.Speed = GPIO_SPEED_HIGH;
	gpioInit.Pull  = GPIO_NOPULL;
	HAL_GPIO_Init(port, &gpioInit);
}


static void set_out(GPIO_TypeDef *port, uint32_t pin) {
	GPIO_InitTypeDef gpioInit;
	HAL_GPIO_DeInit(port, pin);
	gpioInit.Pin   = pin;
	gpioInit.Mode  = GPIO_MODE_OUTPUT_PP;
	gpioInit.Speed = GPIO_SPEED_HIGH;
	gpioInit.Pull  = GPIO_NOPULL;
	HAL_GPIO_Init(port, &gpioInit);
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
}


uint8_t touch_read_adc_xy(touchXY *result) {
	volatile uint32_t x = 0;
	volatile uint32_t y = 0;

	set_gnd(GPIOA, GPIO_PIN_7); // ADC 2 OK
	set_ain(GPIOA, GPIO_PIN_6); // ADC 1 OK
	set_out(GPIOB, GPIO_PIN_9); // P2 OK
	set_in(GPIOB, GPIO_PIN_8);  // P1 OK

	HAL_ADC_Start(&g_AdcHandle);

	if (HAL_ADC_PollForConversion(&g_AdcHandle, 1000000) == HAL_OK) {
		x = HAL_ADC_GetValue(&g_AdcHandle);
	}
	HAL_ADC_Stop(&g_AdcHandle);

	set_ain(GPIOA, GPIO_PIN_7); // ADC 2 ok
	set_gnd(GPIOA, GPIO_PIN_6); // ADC 1 ok
	set_in(GPIOB, GPIO_PIN_9);  // P2 ok
	set_out(GPIOB, GPIO_PIN_8); // P1 ok


	HAL_ADC_Start(&g_AdcHandle2);

	if (HAL_ADC_PollForConversion(&g_AdcHandle2, 1000000) == HAL_OK) {
		y = HAL_ADC_GetValue(&g_AdcHandle2);
	}
	HAL_ADC_Stop(&g_AdcHandle2);

	result->x = x;
	result->y = y;

	//printf("x: %u, y:%u\n", x, y);

	return 1; // result is at this point always valid
}

/*
uint8_t touch_read_adc_xy(touchXY *result) {
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

	//PA7 ain
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

	//printf("x: %u, y:%u\n", x, y);

	return valid;
}
*/

