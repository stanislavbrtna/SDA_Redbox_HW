#ifndef __usart3_H
#define __usart3_H


#include "../sda_platform.h"

extern void Error_Handler(void);

void MX_USART3_UART_Init(void);
void MX_USART3_UART_DeInit(void);
void HAL_UART3_MspInit(UART_HandleTypeDef* uartHandle);
void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle);

uint8_t uart3_recieve(uint8_t *str, uint32_t len, uint32_t timeout);
HAL_StatusTypeDef HAL_UART_AbortReceive(UART_HandleTypeDef *huart);
void uart3_transmit(uint8_t *str, uint32_t len);

uint8_t uart3_recieve_IT();
uint8_t uart3_get_rdy();
uint16_t uart3_get_str(uint8_t *str); // return: len

#endif 
