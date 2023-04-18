#include "sda_platform.h"

extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern RNG_HandleTypeDef rng;

extern uint8_t led_pattern[10];
extern uint16_t led_counter;
extern volatile uint8_t cpuClkLowFlag;
extern uint8_t beep_flag;
extern volatile sdaLockState tick_lock;

extern volatile uint8_t Lcd_on_flag;
extern volatile uint8_t Lcd_off_flag;

volatile uint8_t sdaSerialEnabled;
volatile uint8_t sdaDbgSerialEnabled;


void sda_set_led(uint8_t set) {
	if (set) {
		HAL_GPIO_WritePin(SDA_BASE_SYSLED_PORT, SDA_BASE_SYSLED_PIN, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(SDA_BASE_SYSLED_PORT, SDA_BASE_SYSLED_PIN, GPIO_PIN_RESET);
	}
}

uint8_t sda_serial_recieve(uint8_t *str, uint32_t len, uint32_t timeout) {
	return uart3_recieve(str, len, timeout);
}

uint8_t sda_serial_recieve_init() {
  return uart3_recieve_IT();
}

uint8_t sda_serial_get_rdy() {
  return uart3_get_rdy();
}

uint16_t sda_serial_get_str(uint8_t *str) {
  return uart3_get_str(str);
}

void sda_serial_transmit(uint8_t *str, uint32_t len) {
	uart3_transmit(str, len);
}

void sda_serial_enable() {
	sdaSerialEnabled = 1;
	HAL_UART3_MspInit(&huart3);
	MX_USART3_UART_Init();
}

void sda_serial_disable() {
	HAL_UART_AbortReceive(&huart3);
	MX_USART3_UART_DeInit();
	sdaSerialEnabled = 0;
}

uint8_t sda_serial_is_enabled() {
	return sdaSerialEnabled;
}

void sda_dbg_serial_enable() {
	HAL_UART_MspInit(&huart2);
	MX_USART2_UART_Init();
	sdaDbgSerialEnabled = 1;
}

void sda_dbg_serial_disable() {
	sdaDbgSerialEnabled = 0;
	MX_USART2_UART_DeInit();
}

uint8_t sda_dbg_serial_is_enabled() {
	return sdaDbgSerialEnabled;
}


uint8_t sda_usb_serial_recieve(uint8_t *str, uint32_t len, uint32_t timeout) {
  //return uart2_recieve(str, len, timeout);
  return 0;
}

void sda_usb_serial_transmit(uint8_t *str, uint32_t len) {
  //uart2_transmit(str, len);
  return;
}

void sda_usb_serial_enable() {
  return;
}

void sda_usb_serial_disable() {
  return;
}

uint8_t sda_usb_serial_is_enabled() {
  return 0;
}


uint32_t svp_random() {
	uint32_t ret = 0;
	HAL_RNG_GenerateRandomNumber(&rng, &ret);
	return ret/2; // to get rid of negative values
}


void svp_set_lcd_state(lcdStateType state) {
	if(state == LCD_ON) {
		lcd_hw_wake();
		Lcd_on_flag = 200;
	} else if (state == LCD_OFF) {
		lcd_bl_off();
		Lcd_off_flag = 30;
	}
	svpSGlobal.lcdState = state;
}

void svp_set_backlight(uint8_t val) {
	lcd_hw_set_backlight(val);
}

void led_set_pattern(ledPatternType pat) {
	uint8_t x;
	if (pat == LED_ON) {
		for (x = 0; x < 10; x++) {
			led_pattern[x] = 1;
		}
	}

	if (pat == LED_OFF) {
		for (x = 0; x < 10; x++) {
			led_pattern[x] = 0;
		}
	}

	if (pat == LED_BLINK) {
		led_pattern[0] = 1;
		led_pattern[1] = 1;
		led_pattern[2] = 1;
		led_pattern[3] = 1;
		led_pattern[4] = 1;
		led_pattern[5] = 1;
		led_pattern[6] = 0;
		led_pattern[7] = 0;
		led_pattern[8] = 0;
		led_pattern[9] = 0;
	}

	if (pat == LED_SHORTBLINK) {
		led_pattern[0] = 0;
		led_pattern[1] = 0;
		led_pattern[2] = 0;
		led_pattern[3] = 1;
		led_pattern[4] = 0;
		led_pattern[5] = 0;
		led_pattern[6] = 0;
		led_pattern[7] = 0;
		led_pattern[8] = 0;
		led_pattern[9] = 0;
	}

	if (pat == LED_ALARM) {
		led_pattern[0] = 1;
		led_pattern[1] = 0;
		led_pattern[2] = 1;
		led_pattern[3] = 0;
		led_pattern[4] = 1;
		led_pattern[5] = 0;
		led_pattern[6] = 1;
		led_pattern[7] = 0;
		led_pattern[8] = 1;
		led_pattern[9] = 0;
	}

	led_counter = 0;
}

void system_clock_set_low(void){
	// abychom si nekazili beeper
	// do not set low clock speed if beeper is on
	if(beep_flag != 0) {
		return;
	}

	// Enable Power Control clock
	__PWR_CLK_ENABLE();

	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;

	HAL_RCC_DeInit();
	//hse config
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = (HSE_VALUE/1000000u);
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV8; /* 168 MHz */ //div2
	RCC_OscInitStruct.PLL.PLLQ = 7; /* To make USB work. */
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct)==HAL_ERROR)
		printf("ERROR kdyz menim freq! na 42mhz\n");

	// Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
	// clocks dividers
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK
	  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;

	// This is expected to work for most large cores.
	// Check and update it for your own configuration.
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

	SystemCoreClockUpdate();

	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2); //max 144Mhz

	MX_USART2_UART_Init();
	lcd_bl_timer_OC_update();

	if (sda_serial_is_enabled()) {
		sda_serial_disable();
		sda_serial_enable();
	}
}

void system_clock_set_normal(void){
	// do not change clock speed if beeper is on
	// abychom si nekazili beeper
	if (beep_flag != 0) {
		return;
	}

	HAL_RCC_DeInit();
	SystemClock_Config();
	SystemCoreClockUpdate();
	MX_USART2_UART_Init();
	lcd_bl_timer_OC_update();

	if (sda_serial_is_enabled()) {
		sda_serial_disable();
		sda_serial_enable();
	}
}

void svs_hardErrHandler() {
	tick_lock = SDA_LOCK_LOCKED;
	LCD_Fill(LCD_MixColor(255, 0, 0));
	LCD_DrawText_ext(32, 100, 0xFFFF, (uint8_t *)"Hard error occured!\nSDA-os will now reset!");
	Delay(50000000);
	HAL_NVIC_SystemReset();
	while(1);
}

extern volatile uint8_t batt_measured_flag;
float get_batt_voltage();

uint8_t sda_is_battery_measured() {
  if (batt_measured_flag == 1) {
  	batt_measured_flag = 0;
  	return 1;
  } else {
  	return 0;
  }
}

float sda_get_battery_voltage() {
  return get_batt_voltage();
}
