#include "gpio.h"

/* Interrupt Handler */
void EXTI15_10_IRQHandler(void)
{
    if (EXTI_PR & (1 << 13))
    {
        GPIO_Toggle(GPIOA, 5);
        EXTI_PR |= (1 << 13);
    }
}

int main(void)
{
    GPIO_EnableClock(GPIOA);
    GPIO_EnableClock(GPIOC);

    GPIO_ConfigOutput(GPIOA, 5);   // LED
    GPIO_ConfigInput(GPIOC, 13);   // Button

    EXTI_Config_PC13();

    while(1)
    {
        // waiting for interrupt
    }
}
