/*
Copyright (c) 2021 Stanislav Brtna

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

#include "power_management.h"
extern volatile BoardRevType boardRevision;

volatile uint8_t cpuClkLowFlag; // for the current state of the CPU speed

extern volatile uint8_t Lcd_off_flag;

// battery measurement
extern volatile uint32_t batt_val;

extern volatile uint8_t sdaWakeupFlag;

extern uint32_t uptimeSleepStart;


void sda_sleep() { // should be called when tick is locked
  touchSleep();
  if(cpuClkLowFlag == 0) {
    system_clock_set_low();
  }

  if (svpSGlobal.powerSleepMode == SDA_PWR_MODE_SLEEP_LOW) {
    rtc_set_wkup(128);
  } else if (svpSGlobal.powerSleepMode == SDA_PWR_MODE_SLEEP_NORMAL) {
    rtc_set_wkup(1000);
  } else  if (svpSGlobal.powerSleepMode == SDA_PWR_MODE_SLEEP_DEEP) {
    rtc_set_wkup(8000);
  }

  // enter sleep state
  HAL_SuspendTick();
  // 3.7 mA PWR_LOWPOWERREGULATOR_ON 3.8 with main power, on main power it wakes up immediately
  HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI);
  //HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI); // orig 8.6 mA
  HAL_ResumeTick();
  sda_irq_update_timestruct(rtc.year, rtc.month, rtc.day, rtc.weekday, rtc.hour, rtc.min, rtc.sec);
  touchWake();
}


void EXTI0_IRQHandler(void) {
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  (void) (GPIO_Pin);
  if (svpSGlobal.powerMode == SDA_PWR_MODE_SLEEP && Lcd_off_flag == 0) {
    sdaWakeupFlag = 1;
  }
}


void lowBattCheckAndHalt() {
  if (((uint32_t)(get_batt_voltage() * 10) < 31) && (svpSGlobal.pwrType == POWER_BATT) && (batt_val != 0)) {
    while(1) {
      tick_lock = SDA_LOCK_LOCKED;
      LCD_Fill(LCD_MixColor(255, 0, 0));
      LCD_DrawText_ext(32, 100, 0xFFFF, (uint8_t *)"Low battery!");
      Delay(90000000);
      svp_set_lcd_state(LCD_OFF);
      sda_set_led(0);
      lcd_hw_sleep();
      HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
      update_power_status();
      if (svpSGlobal.pwrType == POWER_USB) {
        HAL_NVIC_SystemReset();
        while(1);
      }
    }
  }
}


void update_power_status() {
	if (boardRevision == REV_1) {
		if (HAL_GPIO_ReadPin(SDA_BASE_USB_DETECT_PORT, SDA_BASE_USB_DETECT_PIN) == GPIO_PIN_SET) {
			svpSGlobal.pwrType = POWER_USB;
		} else {
			svpSGlobal.pwrType = POWER_BATT;
		}
	} else if (boardRevision == REV_0){
		svpSGlobal.pwrType = POWER_BATT;
	}
}


void sda_hw_sleep() {
  uptimeSleepStart = (uint32_t) svpSGlobal.timestamp - svpSGlobal.uptime;
  lcd_hw_sleep();
}
