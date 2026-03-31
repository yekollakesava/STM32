#include "stm32l47xx.h"
#include "usart.h"

void USART2_Init(void)
{
    /* Enable clocks for GPIOA and USART2 */
    RCC_AHB2ENR  |= GPIOA_EN;
    RCC_APB1ENR1 |= USART2_EN;

    /* PA2 = TX, PA3 = RX -> Alternate Function mode */
    GPIOA_MODER &= ~(0xFU << 4);
    GPIOA_MODER |=  (0xAU << 4);

    /* AF7 for USART2 on PA2 and PA3 */
    GPIOA_AFRL &= ~(0xFFU << 8);
    GPIOA_AFRL |=  (0x77U << 8);

    /* High speed optional */
    GPIOA_OSPEEDR |= (0xFU << 4);

    /* For default MSI clock = 4 MHz, baud = 9600 */
    USART2_BRR = 0x1A1;

    /* Enable TX, RX and USART */
    USART2_CR1 = 0;
    USART2_CR1 |= (1U << 3);   // TE
    USART2_CR1 |= (1U << 2);   // RE
    USART2_CR1 |= (1U << 0);   // UE
}

void USART2_SendChar(char c)
{
    while(!(USART2_ISR & (1U << 7)));   // TXE
    USART2_TDR = (uint32_t)c;
}

void USART2_SendString(char *str)
{
    while(*str)
    {
        USART2_SendChar(*str++);
    }
}
