#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

#define RCC_AHB2ENR (*(volatile uint32_t*)0x4002104C)

#define GPIOA_MODER (*(volatile uint32_t*)0x48000000)
#define GPIOA_IDR   (*(volatile uint32_t*)0x48000010)
#define GPIOA_ODR   (*(volatile uint32_t*)0x48000014)

void GPIO_Init(void);
void GPIO_WritePin(uint8_t pin, uint8_t value);
uint8_t GPIO_ReadPin(uint8_t pin);

#endif
