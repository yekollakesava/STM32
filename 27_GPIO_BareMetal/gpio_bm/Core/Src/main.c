#include "main.h"
#include "stm32l476xx.h"

void delay_ms(volatile uint32_t t)
{
    while(t--);
}

int main(void)
{
    // 1) Enable clock for GPIOA and GPIOC
    RCC->AHB2ENR |= (1 << 0);   // GPIOAEN
    RCC->AHB2ENR |= (1 << 2);   // GPIOCEN

    // 2) Configure PA5 as output
    // MODER5 = 01
    GPIOA->MODER &= ~(3 << (5 * 2));
    GPIOA->MODER |=  (1 << (5 * 2));

    // 3) Configure PC13 as input
    // MODER13 = 00
    GPIOC->MODER &= ~(3 << (13 * 2));

    // Optional: enable pull-up for PC13 (button is usually active low)
    GPIOC->PUPDR &= ~(3 << (13 * 2));
    GPIOC->PUPDR |=  (1 << (13 * 2));  // Pull-up

    while(1)
    {
        // Read PC13: pressed = 0
        if ((GPIOC->IDR & (1 << 13)) == 0)
        {
            // debounce delay
            delay_ms(200000);

            // Toggle LED PA5 using ODR XOR
            GPIOA->ODR ^= (1 << 5);

            // wait until button released
            while ((GPIOC->IDR & (1 << 13)) == 0);
            delay_ms(200000);
        }
    }
}
