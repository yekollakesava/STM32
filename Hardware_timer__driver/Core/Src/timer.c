#include "timer.h"

extern TIM_HandleTypeDef htim2;

void Timer2_Start_IT(void)
{
    if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
}
