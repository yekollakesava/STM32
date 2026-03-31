#include "gpio.h"

void GPIO_Init(void)
{
    /* Enable clocks for GPIOA and GPIOC */
    RCC->AHB2ENR |= (1U << 0);   /* GPIOAEN */
    RCC->AHB2ENR |= (1U << 2);   /* GPIOCEN */

    /* PA5 output */
    GPIOA->MODER &= ~(3U << (5U * 2U));
    GPIOA->MODER |=  (1U << (5U * 2U));
    GPIOA->OTYPER &= ~(1U << 5U);
    GPIOA->OSPEEDR &= ~(3U << (5U * 2U));
    GPIOA->PUPDR &= ~(3U << (5U * 2U));

    /* PC13 input with pull-up */
    GPIOC->MODER &= ~(3U << (13U * 2U));
    GPIOC->PUPDR &= ~(3U << (13U * 2U));
    GPIOC->PUPDR |=  (1U << (13U * 2U));
}


void LED_On(void)
{
    GPIOA->ODR |= (1U << 5U);
}

void LED_Off(void)
{
    GPIOA->ODR &= ~(1U << 5U);
}

void LED_Toggle(void)
{
    GPIOA->ODR ^= (1U << 5U);
}
