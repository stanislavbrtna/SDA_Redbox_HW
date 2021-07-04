#include "hw_misc.h"

extern volatile float ADC_Measurement_const;

// battery measurement
uint16_t batt_array[60];
uint16_t vreff_array[60];
volatile uint8_t batt_measured_flag;
volatile uint32_t batt_val;
volatile uint32_t voltage_ref_val;
volatile float batt_adc_const;


void tick_update_buttons() {
  static uint8_t btn[6];
  static uint8_t btn_old[6];

  if(HAL_GPIO_ReadPin(SDA_BASE_BTN_A_PORT, SDA_BASE_BTN_A_PIN) == GPIO_PIN_SET) {
    btn[0] = 1;
  } else {
    btn[0] = 0;
  }
  if(HAL_GPIO_ReadPin(SDA_BASE_BTN_LEFT_PORT, SDA_BASE_BTN_LEFT_PIN) == GPIO_PIN_SET) {
    btn[1] = 1;
  } else {
    btn[1] = 0;
  }
  if(HAL_GPIO_ReadPin(SDA_BASE_BTN_UP_PORT, SDA_BASE_BTN_UP_PIN) == GPIO_PIN_SET) {
    btn[2] = 1;
  } else {
    btn[2] = 0;
  }
  if(HAL_GPIO_ReadPin(SDA_BASE_BTN_DOWN_PORT, SDA_BASE_BTN_DOWN_PIN) == GPIO_PIN_SET) {
    btn[3] = 1;
  } else {
    btn[3] = 0;
  }
  if(HAL_GPIO_ReadPin(SDA_BASE_BTN_RIGHT_PORT, SDA_BASE_BTN_RIGHT_PIN) == GPIO_PIN_SET) {
    btn[4] = 1;
  } else {
    btn[4] = 0;
  }
  if(HAL_GPIO_ReadPin(SDA_BASE_BTN_B_PORT, SDA_BASE_BTN_B_PIN) == GPIO_PIN_SET) {
    btn[5] = 1;
  } else {
    btn[5] = 0;
  }

  uint8_t i;
  for (i = 0; i < 6; i++) {
    if (svpSGlobal.keyEv[i] == EV_NONE) {
      if ((btn[i] == 1) && (btn_old[i] == 0)) {
        svpSGlobal.keyEv[i] = EV_PRESSED;
        svpSGlobal.btnFlag = 1;
      }

      if ((btn[i] == 1) && (btn_old[i] == 1)) {
        svpSGlobal.keyEv[i] = EV_HOLD;
        svpSGlobal.btnFlag = 1;
      }

      if ((btn[i] == 0) && (btn_old[i] == 1)) {
        svpSGlobal.keyEv[i] = EV_RELEASED;
        svpSGlobal.btnFlag = 1;
      }

      btn_old[i] = btn[i];
    }
  }
}


float get_batt_voltage() {
    return (((float)batt_val) * batt_adc_const);
}


void updateTouchScreen() {
  static uint8_t touchPrev;
  static uint8_t touchNow;
  touchXY pRetval;

  svpSGlobal.touchValid = touch_get_xy(&pRetval);

  if (svpSGlobal.touchValid) {
    svpSGlobal.touchX = LCD_rotr_x(pRetval.x, pRetval.y);
    svpSGlobal.touchY = LCD_rotr_y(pRetval.x, pRetval.y);
  }

  if (svpSGlobal.touchValid) {
    touchNow = 1;
  } else {
    touchNow = 0;
  }
  if (svpSGlobal.touchType == EV_NONE) {
    if ((touchNow == 1) && (touchPrev == 0)) {
      svpSGlobal.touchType = EV_PRESSED;
    }

    if ((touchNow == 0) && (touchPrev == 1)) {
      svpSGlobal.touchType = EV_RELEASED;
    }
    if ((touchNow == 1) && (touchPrev == 1)) {
      svpSGlobal.touchType = EV_HOLD;
    }
    if ((touchNow == 0) && (touchPrev == 0)) {
      svpSGlobal.touchType = EV_NONE;
    }
    touchPrev = touchNow;
  }
}


void measureBatteryVoltage() {
  static uint16_t batt_cnt;
  static systemPwrType oldBattState;

  if (oldBattState != svpSGlobal.pwrType && svpSGlobal.pwrType == POWER_BATT) {
    batt_cnt = 0;
    batt_val = 0;
  }

  if (batt_cnt < 29) {
    batt_array[batt_cnt] = getBatteryVoltage();
    //vreff_array[batt_cnt] = getRefVoltage();
    batt_cnt++;
  } else {
    uint32_t temp;
    //uint32_t temp2;
    uint16_t i;

    temp = 0;
    //temp2 = 0;
    for(i = 0; i < 29; i++) {
      temp += batt_array[i];
      //temp2 += vreff_array[i];
    }

    //voltage_ref_val = temp2 / 29 + temp2 % 29;
    batt_val = temp / 29 + temp % 29;

    batt_measured_flag = 1;

    batt_cnt = 0;
  }

  oldBattState = svpSGlobal.pwrType;
}


void show_splash() {
  if (svp_fexists((uint8_t *) "splash.p16")) {
    draw_ppm(0, 0, 1,(uint8_t *) "splash.p16");
  } else {
    if (ppm_get_width((uint8_t *) "splash.ppm") == 320) {
      draw_ppm(0, 0, 1,(uint8_t *) "splash.ppm");
    }else if (ppm_get_width((uint8_t *) "splash.ppm") == 160) {
      draw_ppm(0, 0, 2,(uint8_t *) "splash.ppm");
    }
  }

  for(uint32_t count = 25000000; count != 0; count--) {
    if(HAL_GPIO_ReadPin(SDA_BASE_BTN_B_PORT, SDA_BASE_BTN_B_PIN) == GPIO_PIN_SET) {
      break;
    }
    if(HAL_GPIO_ReadPin(SDA_BASE_BTN_A_PORT, SDA_BASE_BTN_A_PIN) == GPIO_PIN_SET) {
      break;
    }
    if(HAL_GPIO_ReadPin(SDA_BASE_BTN_LEFT_PORT, SDA_BASE_BTN_LEFT_PIN) == GPIO_PIN_SET) {
      break;
    }
    if(HAL_GPIO_ReadPin(SDA_BASE_BTN_RIGHT_PORT, SDA_BASE_BTN_RIGHT_PIN) == GPIO_PIN_SET) {
      break;
    }
    if(HAL_GPIO_ReadPin(SDA_BASE_BTN_DOWN_PORT, SDA_BASE_BTN_DOWN_PIN) == GPIO_PIN_SET) {
      break;
    }
  }
}


void lcd_bw_test() {
  LCD_Fill(0xFFFF);
  sda_set_led(1);
  Delay(2000000);
  LCD_Fill(0x0);
  sda_set_led(0);
  Delay(2000000);
  LCD_Fill(0xFFFF);
  LCD_Fill(0x0);
  sda_set_led(1);
  LCD_setDrawArea(0, 0, 319, 479);
}


// led pattern array
uint8_t led_pattern[10];
uint16_t led_counter;


void tick_update_status_led() {
  static uint8_t led_state;
  static uint16_t led_cnt;

  if (led_cnt < 100) {
    led_cnt++;
  } else {
    led_cnt = 0;
    //práce s patternem ledky
    if (led_pattern[led_counter] != led_state) {
      if (led_pattern[led_counter] == 1) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
        led_state = 1;
      } else {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
        led_state = 0;
      }
    }
    if (led_counter < 9) {
      led_counter++;
    } else {
      led_counter = 0;
    }
  }
}

//extension dummy functions

void sda_internal_pin_def(uint8_t pinNum, uint8_t pinType, uint8_t pull) {
	return;
}

void sda_internal_pin_set(uint8_t pinNum, uint8_t val) {
	return;
}

uint8_t sda_internal_pin_get(uint8_t pinNum) {
	return 0;
}

void sda_external_pin_def(uint8_t pinNum, uint8_t pinType, uint8_t pull) {
	return;
}

void sda_external_pin_set(uint8_t pinNum, uint8_t val) {
	return;
}

uint8_t sda_external_pin_get(uint8_t pinNum) {
	return 0;
}

float sda_external_ADC_get() {
	return 0;
}
