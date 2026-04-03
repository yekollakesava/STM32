#include <stdint.h>

/* RCC */
#define RCC_BASE        0x40021000UL
#define RCC_AHB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x4C))

/* GPIOA */
#define GPIOA_BASE      0x48000000UL
#define GPIOA_MODER     (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_ODR       (*(volatile uint32_t *)(GPIOA_BASE + 0x14))

/* SysTick */
#define SYST_CSR        (*(volatile uint32_t *)0xE000E010)
#define SYST_RVR        (*(volatile uint32_t *)0xE000E014)
#define SYST_CVR        (*(volatile uint32_t *)0xE000E018)

void systick_delay_ms(uint32_t ms)
{
    for(uint32_t i = 0; i < ms; i++)
    {
        SYST_RVR = 4000 - 1;   // 1ms (for 16MHz)
        SYST_CVR = 0;
        SYST_CSR = 5;           // Enable, no interrupt

        while((SYST_CSR & (1 << 16)) == 0);  // Wait for COUNTFLAG

        SYST_CSR = 0;           // Stop timer
    }
}

int main(void)
{
    RCC_AHB2ENR |= (1 << 0);

    GPIOA_MODER &= ~(3 << (5 * 2));
    GPIOA_MODER |=  (1 << (5 * 2));

    while(1)
    {
        GPIOA_ODR ^= (1 << 5);
        systick_delay_ms(500);
    }
}
