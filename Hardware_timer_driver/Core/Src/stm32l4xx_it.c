#include "timer.h"

void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
}
