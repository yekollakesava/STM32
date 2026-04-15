#ifndef UART_H
#define UART_H

#include <stdint.h>

void UART2_Init(void);
void UART2_Print(char *msg);
void UART2_Print_Char(char c);
void UART2_Print_Int(int32_t num);
void UART2_Print_Float(float num);
void UART_SendString(char *str);

#endif
