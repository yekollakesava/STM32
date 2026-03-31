#ifndef TIMER_H
#define TIMER_H

#include "main.h"
#include "stm32l4xx_hal_tim.h"

extern TIM_HandleTypeDef htim2;

void Timer2_Start_IT(void);

#endif
