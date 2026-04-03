#include "stm32l476xx.h"

void UART2_Init(void);
void UART2_SendString_IT(char *str);

volatile char *txBuffer = 0;
volatile uint32_t txIndex = 0;

int main(void)
{
    UART2_Init();

    UART2_SendString_IT("UART Interrupt Working Successfully!\r\n");

    while (1)
    {
        // CPU free
    }
}

void UART2_Init(void)
{
    /* 1. Enable GPIOA clock */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    /* 2. Set PA2, PA3 to Alternate Function mode */
    GPIOA->MODER &= ~(0xF << (2 * 2));
    GPIOA->MODER |=  (0xA << (2 * 2));   // AF mode for PA2 & PA3

    /* 3. Select AF7 for USART2 */
    GPIOA->AFR[0] &= ~(0xFF << (4 * 2));
    GPIOA->AFR[0] |=  (0x77 << (4 * 2));  // AF7 for PA2 & PA3

    /* 4. Enable USART2 clock */
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;

    /* 5. Disable USART before configuration */
    USART2->CR1 &= ~USART_CR1_UE;

    /* 6. Set Baud Rate (Assuming 16 MHz clock) */
    USART2->BRR = 417;   // 16MHz / 9600 = 1666.6 ≈ 1667

    /* 7. Enable Transmitter */
    USART2->CR1 |= USART_CR1_TE;

    /* 8. Enable USART */
    USART2->CR1 |= USART_CR1_UE;

    /* 9. Enable NVIC Interrupt */
    NVIC_EnableIRQ(USART2_IRQn);
}

void UART2_SendString_IT(char *str)
{
    txBuffer = str;
    txIndex = 0;

    /* Enable TXE interrupt AFTER buffer loaded */
    USART2->CR1 |= USART_CR1_TXEIE;
}

void USART2_IRQHandler(void)
{
    if ((USART2->ISR & USART_ISR_TXE) &&
        (USART2->CR1 & USART_CR1_TXEIE))
    {
        if (txBuffer[txIndex] != '\0')
        {
            USART2->TDR = txBuffer[txIndex++];
        }
        else
        {
            /* Disable TXE interrupt after complete */
            USART2->CR1 &= ~USART_CR1_TXEIE;
        }
    }
}
