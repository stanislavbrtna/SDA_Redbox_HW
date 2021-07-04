#include "sda_platform.h"
#include "math.h"

#define MIN_VOLTAGE 3.0
#define MAX_VOLTAGE 4.0

/*===========================================================================*/
/*                            HAL Globals                                    */
ADC_HandleTypeDef g_AdcHandle;
ADC_HandleTypeDef g_AdcHandle2;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
RNG_HandleTypeDef rng;

TIM_HandleTypeDef beeptimer;

/*===========================================================================*/
/*                            SVP Globals                                    */

FATFS FatFs;
svpStatusStruct svpSGlobal;

BoardRevType boardRevision;

uint32_t uptimeSleepStart;

// misc system locks
volatile sdaLockState touch_lock;  // disables touch in irq
volatile sdaLockState irq_lock;    // disables touch, redraw in irq and battery measurement
volatile sdaLockState tick_lock;   // disables all systick stuff

// systick globals
volatile uint32_t counter;
volatile uint16_t sdaAppCounter;
volatile uint16_t svsLoadCounter;

volatile uint8_t serialBuf[16];
volatile uint8_t Lcd_on_flag;
volatile uint8_t Lcd_off_flag;

volatile uint32_t backlight_scaler;

volatile uint8_t sdaWakeupFlag;

volatile uint8_t beep_flag;
volatile uint16_t beep_t;
volatile uint32_t beep_scaler;

volatile uint8_t sdaSerialEnabled;
volatile uint8_t sdaDbgSerialEnabled;

// led pattern array
uint8_t led_pattern[10];
uint16_t led_counter;

// battery measurement
uint16_t batt_array[60];

// battery measurement
extern volatile uint32_t batt_val;
extern volatile float batt_adc_const;
volatile float ADC_Measurement_const;

/*===========================================================================*/


void Delay(__IO uint32_t nCount) {
  for(; nCount != 0; nCount--);
}

//TODO: implement auto-enable & init for serial output & input

uint8_t get_batt_percent() {
	uint8_t percent;
	percent = (uint8_t)((get_batt_voltage() - MIN_VOLTAGE) / ((MAX_VOLTAGE - MIN_VOLTAGE) / 100 ));
	if (percent > 100) {
		percent = 100;
	}
	return percent;
}

/* just here for the old pinout, TBR
void tick_update_buttons(uint8_t *btn) {
	if (boardRev == REV_0) {
		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_SET){
			btn[0] = 1;
		} else {
			btn[0] = 0;
		}
		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_SET){
			btn[3] = 1;
		} else {
			btn[3] = 0;
		}
		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) == GPIO_PIN_SET){
			btn[2] = 1;
		} else {
			btn[2] = 0;
		}
		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15) == GPIO_PIN_SET){
			btn[5] = 1;
		} else {
			btn[5] = 0;
		}

		if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9) == GPIO_PIN_SET){
			btn[4] = 1;
		} else {
			btn[4] = 0;
		}
		if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10) == GPIO_PIN_SET){
			btn[1] = 1;
		} else {
			btn[1] = 0;
		}
	}

	if (boardRev == REV_1) {
		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_SET){
			btn[5] = 1;
		} else {
			btn[5] = 0;
		}
		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_SET){
			btn[3] = 1;
		} else {
			btn[3] = 0;
		}
		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) == GPIO_PIN_SET){
			btn[2] = 1;
		} else {
			btn[2] = 0;
		}
		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15) == GPIO_PIN_SET){
			btn[0] = 1;
		} else {
			btn[0] = 0;
		}

		if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9) == GPIO_PIN_SET){
			btn[4] = 1;
		} else {
			btn[4] = 0;
		}
		if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10) == GPIO_PIN_SET){
			btn[1] = 1;
		} else {
			btn[1] = 0;
		}
	}
}
*/


void SysTick_Handler(void) {
	static uint16_t sec;
	static uint8_t oldsec;

	static uint8_t powerOnLck;
  static uint8_t pwrBtnPrev;

	HAL_IncTick();
	svsLoadCounter++;
	svpSGlobal.uptimeMs++;

	if (Lcd_on_flag > 1){
		Lcd_on_flag--;
	}

	if (Lcd_on_flag == 1) {
		Lcd_on_flag = 0;
		lcd_bl_on();
		svp_set_backlight(svpSGlobal.lcdBacklight);
		if (Lcd_off_flag == 0) {
			setRedrawFlag();
		}
	}

	sda_base_spkr_irq_handler();

	if (tick_lock == SDA_LOCK_UNLOCKED) {
		counter++;

		static uint32_t pwrLongPressCnt;
		// Power on with just press
		if((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET) && (pwrBtnPrev == 0)) {
			if (svpSGlobal.lcdState == LCD_OFF) {
				svp_set_lcd_state(LCD_ON);
				system_clock_set_normal();
				powerOnLck = 1;
				svpSGlobal.powerMode = SDA_PWR_MODE_NORMAL;
				pwrLongPressCnt = 0;
			}
		}
		// pwr long press detection
		if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET && svpSGlobal.lcdState == LCD_ON) {
			pwrLongPressCnt++;
		}

		if (pwrLongPressCnt == 1500) {
			svpSGlobal.systemPwrLongPress = 1;
		}

		if((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) && (pwrBtnPrev == 1)) {
			if (pwrLongPressCnt > 1500) {
				pwrLongPressCnt = 0;
			} else {
				if (powerOnLck) {
					powerOnLck = 0;
				} else {
					// power off with release
					if (svpSGlobal.lcdState == LCD_ON) {
						svp_set_lcd_state(LCD_OFF);
					}
				}
			}
		}
		sdaWakeupFlag = 0; // button was handled
		pwrBtnPrev = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);

		tick_update_buttons();

		tick_update_status_led();

		if (irq_lock == SDA_LOCK_UNLOCKED) {
			irq_lock = SDA_LOCK_LOCKED;
			update_power_status();
			if(sec < 300) {
				sec++;
			} else {
				sec = 0;
				//event update času
				rtc_update_struct();

				sda_irq_update_timestruct(rtc.year, rtc.month, rtc.day, rtc.weekday, rtc.hour, rtc.min, rtc.sec);

				if(rtc.sec != oldsec) {
					if (Lcd_off_flag > 1) {
						Lcd_off_flag--;
					}
					if (Lcd_off_flag == 1) {
						Lcd_off_flag = 0;
						if(svpSGlobal.lcdState == LCD_OFF) {
							sda_hw_sleep();
						}
					}
				}
				oldsec = rtc.sec;
				measureBatteryVoltage();
			}

			if ((counter > 15)) { // Touch update and redraw of the system bar
				counter = 0;

				if ((touch_lock == SDA_LOCK_UNLOCKED) && (svpSGlobal.lcdState == LCD_ON)) {
					updateTouchScreen();
				}

				svp_irq(svpSGlobal);
			}
			irq_lock = SDA_LOCK_UNLOCKED;
		}

		// counter for use inside SVS apps
		if (sdaAppCounter > 0) {
			sdaAppCounter--;
		}
	}
}


void USART3_IRQHandler(void) {
  HAL_UART_IRQHandler(&huart3);
}


int main(void) {
	__initialize_hardware();

	ADC_Measurement_const = BATT_ADC_CONST_DEF;
	batt_adc_const = BATT_ADC_CONST_DEF;

	tick_lock = SDA_LOCK_LOCKED;
	irq_lock = SDA_LOCK_UNLOCKED;

	sda_platform_gpio_init();
	sda_dbg_serial_enable();

	HAL_UART3_MspInit(&huart3);
	MX_USART3_UART_Init();
	svp_beep_set_def();

	printf("hello world!\n");

	printf("System clock: %u Hz\n", (unsigned int)SystemCoreClock);

	if (boardRevision == REV_0) {
		printf("Board revision: REV_0\n");
	} else if (boardRevision == REV_1) {
		printf("Board revision: REV_1\n");
	} else {
		printf("Board revision: unknown\n");
	}

	// drivers init
	rtc_init();
	rrand_init();
	beep_timer_init();
	backlight_timer_init();
	touchInit();

	//backlight hotfix
	svpSGlobal.lcdBacklight = 255;
	svp_set_lcd_state(LCD_ON);
	svp_set_backlight(255);

	LCD_init(319,479, OR_NORMAL);

	// power status update
	update_power_status();

	// if not powered from usb:
	if (svpSGlobal.pwrType == POWER_BATT) {
		batt_val = 0;
		//measure the initial battery state
		while (batt_val == 0) {
			measureBatteryVoltage();
		}
		// and eventualy halt
		lowBattCheckAndHalt();
	}

	lcd_bw_test();

	// UP on both board revisions goes straight to calibration
	if (HAL_GPIO_ReadPin(SDA_BASE_BTN_UP_PORT, SDA_BASE_BTN_UP_PIN) == GPIO_PIN_SET) {
		sda_calibrate();
		sda_setLcdCalibrationFlag(1);
	}

	svp_mount();

	printf("ff init\n");

	sda_set_led(0);

	show_splash();

	// update time before jump into main
	rtc_update_struct();
	sda_irq_update_timestruct(rtc.year, rtc.month, rtc.day, rtc.weekday, rtc.hour, rtc.min, rtc.sec);
	while(1) {
		sda_main_loop();

		// Sleep mode handling
		if (svpSGlobal.powerMode == SDA_PWR_MODE_SLEEP && Lcd_off_flag == 0 && sdaWakeupFlag == 0) {
			tick_lock = SDA_LOCK_LOCKED;
			sda_sleep();
			// update time
			rtc_update_struct();
			sda_irq_update_timestruct(rtc.year, rtc.month, rtc.day, rtc.weekday, rtc.hour, rtc.min, rtc.sec);
			// update uptime
			svpSGlobal.uptime = (uint32_t)svpSGlobal.timestamp - uptimeSleepStart;
			svpSGlobal.uptimeMs = svpSGlobal.uptime * 1000;
			// update battery state
			if (svpSGlobal.powerSleepMode != SDA_PWR_MODE_SLEEP_LOW) {
			  tick_lock = SDA_LOCK_LOCKED;
			  measureBatteryVoltage();
			}
			tick_lock = SDA_LOCK_UNLOCKED;
		}

		// Also halt if battery voltage is too low
		lowBattCheckAndHalt();
	}

}

