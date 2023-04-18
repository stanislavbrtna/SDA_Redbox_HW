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

#include "lcd.h"
//optimised GPIO manipulation macros
#define lcd_set_RST_low() do { GPIOC->BSRR = (uint32_t)GPIO_PIN_9 << 16; } while (0)
#define lcd_set_RST_high() do { GPIOC->BSRR = GPIO_PIN_9; } while (0)

#define lcd_set_CS_low() do { GPIOC->BSRR = (uint32_t)GPIO_PIN_8 << 16; } while (0)
#define lcd_set_CS_high() do { GPIOC->BSRR = GPIO_PIN_8; } while (0)

#define lcd_set_RS_low() do { GPIOA->BSRR = (uint32_t)GPIO_PIN_8 << 16; } while (0)
#define lcd_set_RS_high() do { GPIOA->BSRR = GPIO_PIN_8; } while (0)

#define lcd_set_WR_low() do { GPIOA->BSRR = (uint32_t)GPIO_PIN_11 << 16; } while (0)
#define lcd_set_WR_high() do { GPIOA->BSRR = GPIO_PIN_11; } while (0)

#define lcd_set_RD_low() do { GPIOA->BSRR = (uint32_t)GPIO_PIN_12 << 16; } while (0)
#define lcd_set_RD_high() do { GPIOA->BSRR = GPIO_PIN_12; } while (0)

TIM_HandleTypeDef blTimer;

// internal functions headers
static void lcd_Delay(__IO uint32_t nCount);
static void LCD_Write_COM(uint8_t x);
static void LCD_Write_DATA(uint8_t x);
static void setBacklight(TIM_HandleTypeDef timer, uint32_t channel, uint16_t pulse);

void lcd_Init_Seq();
void lcd_GPIO_Init();

void writeRegister16(uint8_t x, uint16_t data);

//actual send function
void lcd_send_data(uint8_t data);
void lcd_send_cmd(uint8_t data);

/*
 * for GR2:
 *
 * uint8_t lcd_hw_init();
 * void lcd_hw_Draw_Point(uint16_t color);
 * void lcd_hw_set_xy(uint16_t px1, uint16_t py1, uint16_t px2, uint16_t py2);
 *
 * for SDA_OS:
 *
 * void lcd_hw_sleep();
 * void lcd_hw_wake();
 *
 * void lcd_bl_on();
 * void lcd_bl_off();
 *
 * void lcd_hw_set_backlight(uint8_t val);
 *
 *
 */

uint8_t lcd_hw_init(){
	lcd_GPIO_Init();
	lcd_Init_Seq();
	return 0;
}

void lcd_hw_sleep() {
	LCD_Write_COM(0x10);
	lcd_Delay(20);
}

void lcd_hw_wake() {
	LCD_Write_COM(0x11);
	lcd_Delay(20);
}

void lcd_bl_on() {
	GPIO_InitTypeDef GPIO_InitStructure;
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1);
	GPIO_InitStructure.Pin       = GPIO_PIN_1;
	GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed     = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull      = GPIO_PULLUP;
	GPIO_InitStructure.Alternate = GPIO_AF1_TIM2;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
	svp_set_backlight(svpSGlobal.lcdBacklight);
}

void lcd_bl_off() {
	GPIO_InitTypeDef GPIO_InitStructure;
	svp_set_backlight(0);
	HAL_TIM_PWM_Stop(&blTimer, TIM_CHANNEL_2);

	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1);
	GPIO_InitStructure.Pin   = GPIO_PIN_1;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull  = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, 0);
}

void lcd_hw_set_backlight(uint8_t val) {
	setBacklight(blTimer, TIM_CHANNEL_2, val);
}

void backlight_timer_init() {
	__TIM2_CLK_ENABLE();
	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_OC_InitTypeDef sConfigOC;

	blTimer.Instance         = TIM2;
	blTimer.Channel          = HAL_TIM_ACTIVE_CHANNEL_2;
	blTimer.Init.Prescaler   = SystemCoreClock / 200000;
	blTimer.Init.CounterMode = TIM_COUNTERMODE_UP;
	blTimer.Init.Period      = 256;
	blTimer.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
	blTimer.Init.RepetitionCounter = 0;

	if(HAL_TIM_PWM_Init(&blTimer) != HAL_OK) {
		printf("HAL: TIM2 init error!\n");
	}

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;

	if(HAL_TIMEx_MasterConfigSynchronization(&blTimer, &sMasterConfig) != HAL_OK) {
		printf("HAL: TIM2 init error (2)!\n");
	}

	sConfigOC.OCMode       = TIM_OCMODE_PWM1;
	sConfigOC.Pulse        = 128;
	sConfigOC.OCPolarity   = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode   = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState  = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;

	if(HAL_TIM_PWM_ConfigChannel(&blTimer, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) {
		printf("HAL: TIM2 init error (3)!\n");
	}

	if(HAL_TIM_PWM_Start(&blTimer, TIM_CHANNEL_2) != HAL_OK) {
		printf("HAL: TIM2 init error!(4)\n");
	}

}

void lcd_bl_timer_OC_update() {
	HAL_TIM_PWM_Stop(&blTimer, TIM_CHANNEL_2);
	HAL_TIM_PWM_DeInit(&blTimer);
	// stop generation of pwm
	blTimer.Init.Prescaler = SystemCoreClock / 200000;
	if (HAL_TIM_PWM_Init(&blTimer)!= HAL_OK) {
		printf("HAL: TIM2 setPWM error (0)!\n");
	}
	if (HAL_TIM_PWM_Start(&blTimer, TIM_CHANNEL_2)!= HAL_OK) {
		printf("HAL: TIM2 setPWM error (2)!\n");
	}
}

static void setBacklight(TIM_HandleTypeDef timer, uint32_t channel, uint16_t pulse) {
	TIM_OC_InitTypeDef sConfigOC;

	sConfigOC.OCMode       = TIM_OCMODE_PWM1;
	sConfigOC.Pulse        = pulse;
	sConfigOC.OCPolarity   = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity  = TIM_OCNPOLARITY_LOW;
	sConfigOC.OCFastMode   = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState  = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;

	if (HAL_TIM_PWM_ConfigChannel(&timer, &sConfigOC, channel) != HAL_OK) {
		printf("HAL: TIM2 setPWM error (1)!\n");
	}

	if (HAL_TIM_PWM_Start(&timer, channel) != HAL_OK) {
		printf("HAL: TIM2 setPWM error (2)!\n");
	}
}

static void lcd_Delay(__IO uint32_t nCount) {
	nCount *= 3;
  for(; nCount != 0; nCount--);
}

static void LCD_Write_COM(uint8_t x) {
	lcd_send_cmd(x);
}

static void LCD_Write_DATA(uint8_t x) {
	lcd_send_data(x);
}

void writeRegister16(uint8_t x,uint16_t data) {
	lcd_send_cmd(x);
	lcd_send_data(data >> 8);
	lcd_send_data(0x00FF & data);
}

// ILI9481
void lcd_Init_Seq(){

	//reset
	lcd_set_RST_low();
	lcd_Delay(300);
	lcd_set_RST_high();
	lcd_Delay(300);

	LCD_Write_COM(0x11);
	lcd_Delay(20);
	LCD_Write_COM(0xD0);
	LCD_Write_DATA(0x07);
	LCD_Write_DATA(0x42);
	LCD_Write_DATA(0x18);

	LCD_Write_COM(0xD1);
	LCD_Write_DATA(0x00);
	LCD_Write_DATA(0x07);
	LCD_Write_DATA(0x10);

	LCD_Write_COM(0xD2);
	LCD_Write_DATA(0x01);
	LCD_Write_DATA(0x00); // fosc (2)

	LCD_Write_COM(0xC0);
	LCD_Write_DATA(0x10);
	LCD_Write_DATA(0x3B);
	LCD_Write_DATA(0x00);
	LCD_Write_DATA(0x02);
	LCD_Write_DATA(0x11);

	LCD_Write_COM(0xC5);
	LCD_Write_DATA(0x03);
/* better to just leave the defaults...
	// gamma settings
	LCD_Write_COM(0xC8);
	LCD_Write_DATA(0x00); // 1
	LCD_Write_DATA(0x32); // 2
	LCD_Write_DATA(0x36); // 3
	LCD_Write_DATA(0x45); // 4 RP1 RP0 (45) 75
	LCD_Write_DATA(0x08); // 5 VRP0 (6)
	LCD_Write_DATA(0x19); // 6 VRP1 (16)
	LCD_Write_DATA(0x37); // 7
	LCD_Write_DATA(0x75); // 8
	LCD_Write_DATA(0x77); // 9
	LCD_Write_DATA(0x54); // 10
	LCD_Write_DATA(0x0C); // 11
	LCD_Write_DATA(0x00); // 12
*/
	/* orig gamma
	 * LCD_Write_COM(0xC8);
	LCD_Write_DATA(0x00);
	LCD_Write_DATA(0x32);
	LCD_Write_DATA(0x36);
	LCD_Write_DATA(0x45);
	LCD_Write_DATA(0x06);
	LCD_Write_DATA(0x16);
	LCD_Write_DATA(0x37);
	LCD_Write_DATA(0x75);
	LCD_Write_DATA(0x77);
	LCD_Write_DATA(0x54);
	LCD_Write_DATA(0x0C);
	LCD_Write_DATA(0x00);*/

	LCD_Write_COM(0x36);
	LCD_Write_DATA(0x0A);

	LCD_Write_COM(0x3A);
	LCD_Write_DATA(0x55);

	LCD_Write_COM(0x2A);
	LCD_Write_DATA(0x00);
	LCD_Write_DATA(0x00);
	LCD_Write_DATA(0x01);
	LCD_Write_DATA(0x3F);

	LCD_Write_COM(0x2B);
	LCD_Write_DATA(0x00);
	LCD_Write_DATA(0x00);
	LCD_Write_DATA(0x01);
	LCD_Write_DATA(0xE0);
	lcd_Delay(120);
	LCD_Write_COM(0x29);
	LCD_Write_COM(0x2C);
	//lcd_set_RD(0);
}

void lcd_send_data(uint8_t data){
	lcd_set_RD_high();
	lcd_set_RS_high();
	lcd_set_CS_low();

	GPIOC->ODR &= 0xFF00;
	GPIOC->ODR |= data;

	lcd_set_WR_low();

	lcd_set_WR_high();

	GPIOC->ODR &= 0xFF00;
}

void lcd_send_cmd(uint8_t data) {
	lcd_set_RD_high();
	lcd_set_RS_low();
	lcd_set_CS_low();

	GPIOC->ODR &= 0xFF00;
	GPIOC->ODR |= data;

	lcd_set_WR_low();
	lcd_set_WR_high();
	lcd_set_CS_high();
	GPIOC->ODR &= 0xFF00;
}


void lcd_hw_set_xy(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
	lcd_send_cmd(0x2A);
	lcd_send_data(x1 >> 8);
	lcd_send_data(0x00FF & x1);
	lcd_send_data(x2 >> 8);
	lcd_send_data(0x00FF & x2);

	lcd_send_cmd(0x2B);
	lcd_send_data(y1 >> 8);
	lcd_send_data(0x00FF & y1);
	lcd_send_data(y2 >> 8);
	lcd_send_data(0x00FF & y2);

	lcd_send_cmd(0x2C);
}

void lcd_hw_set_cursor(uint16_t Xpos, uint16_t Ypos) {
	lcd_send_cmd(0x2A);
	lcd_send_data(Xpos >> 8);
	lcd_send_data(0x00FF & Xpos);
	lcd_send_cmd(0x2B);
	lcd_send_data(Ypos >> 8);
	lcd_send_data(0x00FF & Ypos);
	lcd_send_cmd(0x2C);
}

void lcd_hw_Draw_Point(uint16_t color) {
	lcd_send_data(color >> 8);
	lcd_send_data(color & 0x00FF);
}

void lcd_GPIO_Init() {
	GPIO_InitTypeDef GPIO_InitStructure;

	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	//Data pins are PC0 - PC7
	GPIO_InitStructure.Pin = GPIO_PIN_0
													|GPIO_PIN_1
													|GPIO_PIN_2
													|GPIO_PIN_3
													|GPIO_PIN_4
													|GPIO_PIN_5
													|GPIO_PIN_6
													|GPIO_PIN_7;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull  = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

	//RST - PC9
	//CS  - PC8
	//RS  - PA8
	//WR  - PA11
	//RD  - PA12

	GPIO_InitStructure.Pin   = GPIO_PIN_8 | GPIO_PIN_9;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull  = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.Pin   = GPIO_PIN_8 | GPIO_PIN_11 | GPIO_PIN_12;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull  = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

}
