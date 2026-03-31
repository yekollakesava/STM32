#include "stm32l476xx.h"
#include "rcc.h"
#include "gpio.h"

void delay_ms(uint32_t ms)
{
    for(volatile uint32_t i = 0; i < ms * 4000; i++);
}

int main(void)
{
    RCC_Init();

    /* Enable GPIOA clock */
    RCC->AHB2ENR |= (1 << 0);

    /* Initialize PA5 */
    GPIO_Init_Output(GPIOA, 5);

    while(1)
    {
        GPIO_TogglePin(GPIOA, 5);
        delay_ms(500);
    }
}
