#ifndef SPI_H_
#define SPI_H_

#include "stm32l47xx.h"
#include <stdint.h>

void SPI1_GPIO_Init(void);
void SPI1_Init_Slow(void);
void SPI1_SetFast(void);
uint8_t SPI1_Transfer(uint8_t data);

void UART2_Init(void);
void UART2_SendChar(char c);
void UART2_SendString(const char *str);
void UART2_SendHex(uint8_t data);

void delay_ms(uint32_t ms);

#define SD_CS_LOW()     (GPIOA->ODR &= ~(1U << 4))
#define SD_CS_HIGH()    (GPIOA->ODR |=  (1U << 4))

#endif
