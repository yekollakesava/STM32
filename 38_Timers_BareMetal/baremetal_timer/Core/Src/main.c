#include <stdint.h>

/* ================= RCC ================= */
#define RCC_BASE        0x40021000UL
#define RCC_AHB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x4C))
#define RCC_APB1ENR1    (*(volatile uint32_t *)(RCC_BASE + 0x58))

/* ================= GPIO ================= */
#define GPIOA_BASE      0x48000000UL
#define GPIOA_MODER     (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_AFRL      (*(volatile uint32_t *)(GPIOA_BASE + 0x20))

/* ================= USART2 ================= */
#define USART2_BASE     0x40004400UL
#define USART2_CR1      (*(volatile uint32_t *)(USART2_BASE + 0x00))
#define USART2_BRR      (*(volatile uint32_t *)(USART2_BASE + 0x0C))
#define USART2_ISR      (*(volatile uint32_t *)(USART2_BASE + 0x1C))
#define USART2_TDR      (*(volatile uint32_t *)(USART2_BASE + 0x28))

/* ================= TIM2 ================= */
#define TIM2_BASE       0x40000000UL
#define TIM2_CR1        (*(volatile uint32_t *)(TIM2_BASE + 0x00))
#define TIM2_DIER       (*(volatile uint32_t *)(TIM2_BASE + 0x0C))
#define TIM2_SR         (*(volatile uint32_t *)(TIM2_BASE + 0x10))
#define TIM2_PSC        (*(volatile uint32_t *)(TIM2_BASE + 0x28))
#define TIM2_ARR        (*(volatile uint32_t *)(TIM2_BASE + 0x2C))

/* ================= NVIC ================= */
#define NVIC_ISER0      (*(volatile uint32_t*)0xE000E100)

volatile uint32_t seconds = 0;

/* ================= UART SEND ================= */

void uart_send_char(char c)
{
    while (!(USART2_ISR & (1 << 7)));   // Wait TXE
    USART2_TDR = c;
}

void uart_send_string(char *str)
{
    while (*str)
    {
        uart_send_char(*str++);
    }
}

/* ================= TIM2 INTERRUPT ================= */

void TIM2_IRQHandler(void)
{
    if (TIM2_SR & 1)
    {
        TIM2_SR &= ~(1);

        seconds++;

        uart_send_string("1 sec completed\r\n");
    }
}

/* ================= MAIN ================= */

int main(void)
{
    /* Enable GPIOA clock */
    RCC_AHB2ENR |= (1 << 0);

    /* Enable USART2 clock */
    RCC_APB1ENR1 |= (1 << 17);

    /* Enable TIM2 clock */
    RCC_APB1ENR1 |= (1 << 0);

    /* PA2 → TX (AF7) */
    GPIOA_MODER &= ~(3 << (2 * 2));
    GPIOA_MODER |=  (2 << (2 * 2));     // Alternate function

    GPIOA_AFRL &= ~(0xF << (2 * 4));
    GPIOA_AFRL |=  (7 << (2 * 4));      // AF7 = USART2

    /* USART Config */
    USART2_BRR = 417;       // 9600 baud for 4MHz
    USART2_CR1 |= (1 << 3); // TE
    USART2_CR1 |= (1 << 0); // UE

    /* Timer Config */
    TIM2_PSC = 4000 - 1;
    TIM2_ARR = 1000 - 1;

    TIM2_DIER |= 1;    // Enable update interrupt
    TIM2_CR1  |= 1;    // Start timer

    NVIC_ISER0 |= (1 << 28);   // Enable TIM2 IRQ

    while (1)
    {
    }
}
