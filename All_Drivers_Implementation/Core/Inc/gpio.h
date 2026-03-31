#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

void Button_Init(void);
uint8_t Button_Pressed(void);
void GPIO_Init(void);

#endif
