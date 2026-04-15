#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

void Button_Init(void);
uint8_t Button_Pressed(void);
void GPIO_Init(void);
void GPIO_Init_Output(GPIO_TypeDef *GPIOx, uint8_t pin);
void GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint8_t pin);
#endif
