#include "stm32l476xx.h"

/* Function Prototypes */
void GPIO_Init(void);
void EXTI_Init(void);

int main(void)
{
    GPIO_Init();
    EXTI_Init();

    while (1)
    {
        // Nothing needed here
        // LED toggles inside interrupt
    }
}

/* ================= GPIO INIT ================= */
void GPIO_Init(void)
{
    /* Enable Clocks */
    RCC->AHB2ENR |= (1 << 0);   // GPIOA clock
    RCC->AHB2ENR |= (1 << 2);   // GPIOC clock

    /* PA5 → Output (LD2 LED) */
    GPIOA->MODER &= ~(3 << (5 * 2));
    GPIOA->MODER |=  (1 << (5 * 2));   // Output mode

    GPIOA->OTYPER &= ~(1 << 5);        // Push-pull
    GPIOA->OSPEEDR |= (3 << (5 * 2));  // High speed
    GPIOA->PUPDR &= ~(3 << (5 * 2));   // No pull

    /* PC13 → Input (User Button B1) */
    GPIOC->MODER &= ~(3 << (13 * 2));  // Input mode
}

/* ================= EXTI INIT ================= */
void EXTI_Init(void)
{
    /* Enable SYSCFG Clock */
    RCC->APB2ENR |= (1 << 0);

    /* Connect PC13 to EXTI13 */
    SYSCFG->EXTICR[3] &= ~(0xF << 4);   // Clear EXTI13 bits
    SYSCFG->EXTICR[3] |=  (0x2 << 4);   // Port C selected

    /* Enable EXTI13 */
    EXTI->IMR1 |= (1 << 13);    // Interrupt unmask
    EXTI->FTSR1 |= (1 << 13);   // Falling edge trigger

    /* Enable NVIC */
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/* ================= INTERRUPT HANDLER ================= */
void EXTI15_10_IRQHandler(void)
{
    if (EXTI->PR1 & (1 << 13))   // Check pending
    {
        EXTI->PR1 |= (1 << 13);  // Clear pending

        GPIOA->ODR ^= (1 << 5);  // Toggle LED
    }
}
