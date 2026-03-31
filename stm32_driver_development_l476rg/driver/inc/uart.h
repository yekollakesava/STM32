/*
 * uart.h
 *
 *  Created on: Mar 12, 2026
 *      Author: yekol
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "stm32l47xx.h"
#include <stdint.h>

void UART2_Init(void);
void UART_SendChar(char c);
void UART_SendString(const char *str);


#endif /* INC_UART_H_ */
