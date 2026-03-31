#include <stdint.h>

/* Base addresses */
#define RCC_BASE        0x40021000UL
#define GPIOA_BASE      0x48000000UL

/* RCC registers */
#define RCC_AHB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x4C))

/* GPIOA registers */
#define GPIOA_MODER     (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_OTYPER    (*(volatile uint32_t *)(GPIOA_BASE + 0x04))
#define GPIOA_OSPEEDR   (*(volatile uint32_t *)(GPIOA_BASE + 0x08))
#define GPIOA_PUPDR     (*(volatile uint32_t *)(GPIOA_BASE + 0x0C))
#define GPIOA_ODR       (*(volatile uint32_t *)(GPIOA_BASE + 0x14))
#define GPIOA_BSRR      (*(volatile uint32_t *)(GPIOA_BASE + 0x18))

/* LED pin for NUCLEO-L476RG */
#define LED_PIN         5U

void delay(volatile uint32_t count)
{
    while(count--);
}

int main(void)
{
    /* 1. Enable clock for GPIOA */
    RCC_AHB2ENR |= (1U << 0);

    /* Small delay after clock enable */
    delay(1000);

    /* 2. Configure PA5 as General Purpose Output */
    GPIOA_MODER &= ~(3U << (LED_PIN * 2));   // Clear mode bits
    GPIOA_MODER |=  (1U << (LED_PIN * 2));   // Set as output mode

    /* 3. Output type = Push-pull */
    GPIOA_OTYPER &= ~(1U << LED_PIN);

    /* 4. Output speed = Low */
    GPIOA_OSPEEDR &= ~(3U << (LED_PIN * 2));

    /* 5. No pull-up / pull-down */
    GPIOA_PUPDR &= ~(3U << (LED_PIN * 2));

    while(1)
    {
        /* LED ON */
        GPIOA_BSRR = (1U << LED_PIN);
        delay(500000);

        /* LED OFF */
        GPIOA_BSRR = (1U << (LED_PIN + 16U));
        delay(500000);
    }
}
