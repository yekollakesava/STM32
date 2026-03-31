#include "gpio.h"

void GPIO_EnableClock(GPIO_TypeDef *GPIOx)
{
    if(GPIOx == GPIOA)
        RCC_AHB1ENR |= (1 << 0);
    else if(GPIOx == GPIOB)
        RCC_AHB1ENR |= (1 << 1);
    else if(GPIOx == GPIOC)
        RCC_AHB1ENR |= (1 << 2);
}

void GPIO_SetMode(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t mode)
{
    GPIOx->MODER &= ~(3 << (2 * pin));
    GPIOx->MODER |=  (mode << (2 * pin));
}

void GPIO_WritePin(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t state)
{
    if(state)
        GPIOx->BSRR = (1 << pin);
    else
        GPIOx->BSRR = (1 << (pin + 16));
}

void GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint8_t pin)
{
    GPIOx->ODR ^= (1 << pin);
}

uint8_t GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint8_t pin)
{
    return (GPIOx->IDR & (1 << pin)) ? 1 : 0;
}
