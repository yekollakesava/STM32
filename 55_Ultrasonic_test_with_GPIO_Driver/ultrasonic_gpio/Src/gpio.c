#include "gpio.h"

void GPIO_Init()
{
    /* Enable GPIOA clock */
    RCC_AHB2ENR |= (1<<0);

    /* PA1 -> TRIG -> OUTPUT */
    GPIOA_MODER &= ~(3<<(1*2));
    GPIOA_MODER |= (1<<(1*2));

    /* PA2 -> ECHO -> INPUT */
    GPIOA_MODER &= ~(3<<(2*2));
}

void GPIO_WritePin(uint8_t pin, uint8_t value)
{
    if(value)
        GPIOA_ODR |= (1<<pin);
    else
        GPIOA_ODR &= ~(1<<pin);
}

uint8_t GPIO_ReadPin(uint8_t pin)
{
    return (GPIOA_IDR & (1<<pin));
}
