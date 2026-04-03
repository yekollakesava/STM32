#ifndef GPIO_H
#define GPIO_H

#include "main.h"
#include <stdint.h>

void GPIO_Init(void);
uint8_t Button_Pressed(void);
void LED_On(void);
void LED_Off(void);
void LED_Toggle(void);

#endif
