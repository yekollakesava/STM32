#ifndef USART_H
#define USART_H

#include <stdint.h>

void USART2_Init(void);
void USART2_SendChar(char c);
void USART2_SendString(char *str);

#endif
