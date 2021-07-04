#include "sda_platform.h"

RNG_HandleTypeDef rng;

extern volatile BoardRevType boardRevision;

void rrand_init() {
	__HAL_RCC_RNG_CLK_ENABLE ();
	rng.Instance = RNG;
	if(HAL_RNG_Init(&rng) != HAL_OK) {
		printf("HAL: RNG init error!\n");
	}
}

static void platform_gpio_ain(GPIO_TypeDef *port, uint32_t pin) {
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin   = pin;
	GPIO_InitStructure.Mode  = GPIO_MODE_ANALOG;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull  = GPIO_NOPULL;
	HAL_GPIO_Init(port, &GPIO_InitStructure);
}

static void platform_gpio_out(GPIO_TypeDef *port, uint32_t pin) {
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin   = pin;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull  = GPIO_PULLUP;
	HAL_GPIO_Init(port, &GPIO_InitStructure);
}

static void platform_gpio_in(GPIO_TypeDef *port, uint32_t pin) {
	GPIO_InitTypeDef GPIO_InitStructure;
	HAL_GPIO_DeInit(port, pin);
	GPIO_InitStructure.Pin = pin;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(port, &GPIO_InitStructure);
}

void pwr_btn_gpio_init() {
	GPIO_InitTypeDef GPIO_InitStruct;
	//printf("sda entering deep sleep\n");
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0);
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* Enable and set Button EXTI Interrupt to the lowest priority */
	HAL_NVIC_SetPriority(EXTI0_IRQn, 0x0F, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);

	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
	HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
}


void sda_platform_gpio_init() {
	GPIO_InitTypeDef GPIO_InitStructure;

	// timing
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();


	//ledpin
	platform_gpio_out(SDA_BASE_SYSLED_PORT, SDA_BASE_SYSLED_PIN);
	sda_set_led(1);

	//spkrpin
	platform_gpio_out(SDA_BASE_SPEAKER_PORT, SDA_BASE_SPEAKER_PIN);

	//PA0 in - sw-on
	platform_gpio_in(GPIOA, GPIO_PIN_0);
	pwr_btn_gpio_init();


	//PA1 - Backlight control
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1);
	GPIO_InitStructure.Pin = GPIO_PIN_1;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Alternate = GPIO_AF1_TIM2;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	//buttons
	platform_gpio_in(SDA_BASE_BTN_A_PORT, SDA_BASE_BTN_A_PIN);
	platform_gpio_in(SDA_BASE_BTN_B_PORT, SDA_BASE_BTN_B_PIN);
	platform_gpio_in(SDA_BASE_BTN_LEFT_PORT, SDA_BASE_BTN_LEFT_PIN);
	platform_gpio_in(SDA_BASE_BTN_RIGHT_PORT, SDA_BASE_BTN_RIGHT_PIN);
	platform_gpio_in(SDA_BASE_BTN_UP_PORT, SDA_BASE_BTN_UP_PIN);
	platform_gpio_in(SDA_BASE_BTN_DOWN_PORT, SDA_BASE_BTN_DOWN_PIN);

	//PB4 - board detect
	GPIO_InitStructure.Pin = GPIO_PIN_4;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

	sda_platform_board_rev_detect();

	if (boardRevision == REV_1) {
		platform_gpio_in(SDA_BASE_USB_DETECT_PORT, SDA_BASE_USB_DETECT_PIN);
	}

	// TODO: set unused pins as ain
}

void sda_platform_board_rev_detect() {
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_RESET) {
		// white box
		boardRevision = REV_0;
	} else {
		// red device
		boardRevision = REV_1;
	}
}
