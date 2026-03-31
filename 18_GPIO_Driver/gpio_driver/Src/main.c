#include "gpio.h"

void delay()
{
    for(volatile uint32_t i = 0; i < 2000000; i++);
}

int main(void)
{
    GPIO_EnableClock(GPIOA);
    GPIO_SetMode(GPIOA, 5, GPIO_OUTPUT);

    while(1)
    {
        GPIO_TogglePin(GPIOA, 5);
        delay();
    }
}
