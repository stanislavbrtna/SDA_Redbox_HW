/*
Copyright (c) 2016 Stanislav Brtna

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

#include "rtc.h"

rtcTimeDate rtc;
RTC_HandleTypeDef RtcHandle;

void rtc_init() {
	RtcHandle.Instance = RTC;

	RtcHandle.Init.HourFormat     = RTC_HOURFORMAT_24;
	RtcHandle.Init.AsynchPrediv   = 127;
	RtcHandle.Init.SynchPrediv    = 255;
	RtcHandle.Init.OutPut         = RTC_OUTPUT_DISABLE;
	RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	RtcHandle.Init.OutPutType     = RTC_OUTPUT_TYPE_OPENDRAIN;

	if(HAL_RTC_Init(&RtcHandle) != HAL_OK) {
		printf("rtc init failure!\n");
	}
}


void rtc_set_time(
									uint16_t year,
									uint8_t  day,
									uint8_t  wkday,
									uint8_t  month,
									uint8_t  hour,
									uint8_t  min,
									uint8_t  sec
									) {
	RTC_TimeTypeDef stimestructure;
	RTC_DateTypeDef sdatestructure;

  if (year >= 2000) {
    year -= 2000;
  }

  if (month == 0) {
    month = 1;
  }

	sdatestructure.Year    = (uint8_t) year;
	sdatestructure.Month   = (uint8_t) month;
	sdatestructure.Date    = (uint8_t) day;
	sdatestructure.WeekDay = wkday;

	if(HAL_RTC_SetDate(&RtcHandle, &sdatestructure, FORMAT_BIN) != HAL_OK) {
		printf("rtc set date failure!\n");
	}

	stimestructure.Hours   = (uint8_t) hour;
	stimestructure.Minutes = (uint8_t)min;
	stimestructure.Seconds = (uint8_t)sec;
	stimestructure.TimeFormat     = RTC_HOURFORMAT12_AM;
	stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
	stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;

	if(HAL_RTC_SetTime(&RtcHandle,&stimestructure,FORMAT_BIN) != HAL_OK) {
		printf("rtc set time failure!\n");
	}
}

void rtc_update_struct() {
	RTC_DateTypeDef sdatestructureget;
	RTC_TimeTypeDef stimestructureget;

	HAL_RTC_GetTime(&RtcHandle, &stimestructureget, FORMAT_BIN);
	HAL_RTC_GetDate(&RtcHandle, &sdatestructureget, FORMAT_BIN);

	rtc.sec     = stimestructureget.Seconds;
	rtc.min     = stimestructureget.Minutes;
	rtc.hour    = stimestructureget.Hours;
	rtc.weekday = sdatestructureget.WeekDay;
	rtc.day     = sdatestructureget.Date;
	rtc.month   = sdatestructureget.Month;
	rtc.year    = 2000 + (uint16_t)sdatestructureget.Year;

	// if yoear is 2000 (stm32 rtc default), some sane defaults will be set:
	if (rtc.year == 2000) {
	  rtc_set_time(2018, 1, 1, 1, 0, 0, 0);
	  rtc.year = 2018;
	}
}

void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc) {
  RCC_OscInitTypeDef        RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
  (void)(hrtc);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_NONE;
  RCC_OscInitStruct.LSEState       = RCC_LSE_ON;
  RCC_OscInitStruct.LSIState       = RCC_LSI_OFF;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    printf("rtc init failure (1)\n");
    return;
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;

  if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
    printf("rtc init failure (2)\n");
    return;
  }

  __HAL_RCC_RTC_ENABLE();
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef *hrtc) {
	(void)(hrtc);
	__HAL_RCC_RTC_DISABLE();
	HAL_NVIC_DisableIRQ(TAMP_STAMP_IRQn);
}

void RTC_WKUP_IRQHandler(void) {
  HAL_RTCEx_WakeUpTimerIRQHandler(&RtcHandle);
}

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc) {
	// woke
	(void)(hrtc);
	HAL_RTCEx_DeactivateWakeUpTimer(&RtcHandle);
}

void rtc_set_wkup(uint32_t ms) {
	// 2000 = 1s
	HAL_RTCEx_SetWakeUpTimer_IT(&RtcHandle, 2*ms, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
}

void rtc_write_password(uint8_t *pwd) {
  __HAL_RTC_WRITEPROTECTION_DISABLE(&RtcHandle);
  HAL_PWR_EnableBkUpAccess();
  // Set password added flag
  HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR0, 1);
  HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR1, pwd[0] << 24 | pwd[1] << 16 | pwd[2] << 8 | pwd[3]);
  HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR2, pwd[4] << 24 | pwd[5] << 16 | pwd[6] << 8 | pwd[7]);
  HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR3, pwd[8] << 24 | pwd[9] << 16 | pwd[10] << 8 | pwd[11]);
  HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR4, pwd[12] << 24 | pwd[13] << 16 | pwd[14] << 8 | pwd[15]);
  HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR5, pwd[16] << 24 | pwd[17] << 16 | pwd[18] << 8 | pwd[19]);
  HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR6, pwd[20] << 24 | pwd[21] << 16 | pwd[22] << 8 | pwd[23]);
  HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR7, pwd[24] << 24 | pwd[25] << 16 | pwd[26] << 8 | pwd[27]);
  HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR8, pwd[28] << 24 | pwd[29] << 16 | pwd[30] << 8 | pwd[31]);
  //HAL_PWR_DisableBkUpAccess(); //??
  __HAL_RTC_WRITEPROTECTION_ENABLE(&RtcHandle);
}

uint8_t rtc_read_password(uint8_t *pwd) {
  uint32_t val = 0;
  // detect
  if (HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR0) == 0) {
    return 1;
  }
  // read
  val = HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR1);
  pwd[0] = (val & 255 << 24) >> 24;
  pwd[1] = (val & 255 << 16) >> 16;
  pwd[2] = (val & 255 << 8) >> 8;
  pwd[3] = val & 255;

  val = HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR2);
  pwd[4] = (val & 255 << 24) >> 24;
  pwd[5] = (val & 255 << 16) >> 8;
  pwd[6] = (val & 255 << 8) >> 16;
  pwd[7] = val & 255;

  val = HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR3);
  pwd[8] = (val & 255 << 24) >> 24;
  pwd[9] = (val & 255 << 16) >> 8;
  pwd[10] = (val & 255 << 8) >> 16;
  pwd[11] = val & 255;

  val = HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR4);
  pwd[12] = (val & 255 << 24) >> 24;
  pwd[13] = (val & 255 << 16) >> 8;
  pwd[14] = (val & 255 << 8) >> 16;
  pwd[15] = val & 255;

  val = HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR5);
  pwd[16] = (val & 255 << 24) >> 24;
  pwd[17] = (val & 255 << 16) >> 8;
  pwd[18] = (val & 255 << 8) >> 16;
  pwd[19] = val & 255;

  val = HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR6);
  pwd[20] = (val & 255 << 24) >> 24;
  pwd[21] = (val & 255 << 16) >> 8;
  pwd[22] = (val & 255 << 8) >> 16;
  pwd[23] = val & 255;

  val = HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR7);
  pwd[24] = (val & 255 << 24) >> 24;
  pwd[25] = (val & 255 << 16) >> 8;
  pwd[26] = (val & 255 << 8) >> 16;
  pwd[27] = val & 255;

  val = HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR8);
  pwd[28] = (val & 255 << 24) >> 24;
  pwd[29] = (val & 255 << 16) >> 8;
  pwd[30] = (val & 255 << 8) >> 16;
  pwd[31] = val & 255;

  return 0;
}
