#include "speaker.h"

TIM_HandleTypeDef beeptimer;

volatile uint8_t beep_flag;
volatile uint16_t beep_t;
volatile uint32_t beep_scaler;


void beep_timer_init() {
	__TIM3_CLK_ENABLE();
	beeptimer.Instance = TIM3;
	beeptimer.Channel = HAL_TIM_ACTIVE_CHANNEL_4;
	beeptimer.Init.Prescaler = 20;
	beeptimer.Init.CounterMode = TIM_COUNTERMODE_UP;
	beeptimer.Init.Period = 210;
	beeptimer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	beeptimer.Init.RepetitionCounter = 0;

	if(HAL_TIM_Base_Init(&beeptimer) != HAL_OK) {
		printf("HAL: TIM3 init error!\n");
	}

	if(HAL_TIM_Base_Stop_IT(&beeptimer) != HAL_OK) {
		printf("HAL: TIM3 start error!\n");
	}

	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
	svp_beep_set_def();
}

void TIM3_IRQHandler() {
	HAL_TIM_IRQHandler(&beeptimer);
	SDA_BASE_SPEAKER_PORT->ODR ^= SDA_BASE_SPEAKER_PIN;
}

void svp_beep() {
	if (svpSGlobal.mute == 0) {
		beep_flag = 1;
	} else {
		led_set_pattern(LED_ALARM);
	}
}

void svp_beep_set_t(uint16_t time) {
	beep_t = time;
}

void svp_beep_set_pf(uint16_t val) {
  if (val < 27) {
    val = 27;
  }
  if (val > 20000) {
    val = 20000;
  }
	beep_scaler = val;

	// auto reload value
	TIM3->ARR = 20000/val;
}

void svp_beep_set_def() {
	beep_scaler = 1000;
	beep_t = 250;
}

void sda_beep_setup(uint16_t freq) {
	__TIM3_CLK_ENABLE();

	if (freq < 27) {
	 freq = 27;
	}
	if (freq > 20000){
	  freq = 20000;
	}

	HAL_TIM_Base_DeInit(&beeptimer);
	beeptimer.Instance               = TIM3;
	beeptimer.Channel                = HAL_TIM_ACTIVE_CHANNEL_4;
	beeptimer.Init.Prescaler         = 8400;
	beeptimer.Init.CounterMode       = TIM_COUNTERMODE_UP;
	beeptimer.Init.Period            = 20000/freq;
	beeptimer.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
	beeptimer.Init.RepetitionCounter = 0;

	if(HAL_TIM_Base_Init(&beeptimer) != HAL_OK) {
		printf("HAL: TIM3 init error!\n");
	}

	if(HAL_TIM_Base_Stop_IT(&beeptimer) != HAL_OK) {
		printf("HAL: TIM3 start error!\n");
	}

	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

void sda_base_beep_start() {
	sda_beep_setup(beep_scaler);
	HAL_TIM_Base_Start_IT(&beeptimer);
}

// runs in systick about every ms, starts and stops the beeps
void sda_base_spkr_irq_handler() {
	static uint16_t beep_cnt;

	if (beep_flag == 1) {
		beep_flag = 2;
		sda_base_beep_start();
		beep_cnt = beep_t;
	}

	if (beep_cnt > 0) {
		beep_cnt--;
	} else {
		if (beep_flag == 2) {
			beep_flag = 0;
			HAL_TIM_Base_Stop_IT(&beeptimer);
			HAL_GPIO_WritePin(
					SDA_BASE_SPEAKER_PORT,
					SDA_BASE_SPEAKER_PIN,
					GPIO_PIN_RESET
					);
		}
	}
}
