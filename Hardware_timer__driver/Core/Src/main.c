#include "main.h"
#include "stm32l4xx_hal.h"
#include "timer.h"

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    }
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_Init();

    Timer2_Start_IT();

    while (1)
    {
    }
}
