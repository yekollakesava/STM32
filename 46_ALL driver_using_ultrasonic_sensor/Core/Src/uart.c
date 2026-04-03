#include "stm32l476xx.h"
#include "uart.h"

void UART2_Init(void)
{
    /* Enable clocks */
    RCC->AHB2ENR  |= (1U << 0);    // GPIOA clock
    RCC->APB1ENR1 |= (1U << 17);   // USART2 clock

    /* PA2 = USART2_TX, PA3 = USART2_RX */
    GPIOA->MODER &= ~(0xFU << 4);
    GPIOA->MODER |=  (0xAU << 4);   // alternate function mode for PA2, PA3

    /* AF7 for USART2 on PA2 and PA3 */
    GPIOA->AFR[0] &= ~(0xFFU << 8);
    GPIOA->AFR[0] |=  (0x77U << 8);

    /* Optional: high speed */
    GPIOA->OSPEEDR |= (0xFU << 4);

    /* Same BRR value as your yesterday working code */
    USART2->BRR = 0x01A1;

    USART2->CR1 |= (1U << 3);   // TE
    USART2->CR1 |= (1U << 2);   // RE
    USART2->CR1 |= (1U << 0);   // UE
}

void UART2_SendChar(char ch)
{
    while (!(USART2->ISR & (1U << 7)));   // TXE
    USART2->TDR = ch;
}

void UART2_Print(char *str)
{
    while(*str)
    {
        UART2_SendChar(*str++);
    }
}
