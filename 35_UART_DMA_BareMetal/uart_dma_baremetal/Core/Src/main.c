#include "stm32l476xx.h"

void UART2_Init(void);
void DMA_Init(void);
void UART2_Send_DMA(char *data, uint32_t length);

char message[] = "UART DMA Working Successfully!\r\n";

/* ================= MAIN ================= */

int main(void)
{
    UART2_Init();
    DMA_Init();

    while (1)
    {
        UART2_Send_DMA(message, sizeof(message) - 1);

        for (volatile int i = 0; i < 8000000; i++);
    }
}

/* ================= UART INIT ================= */

void UART2_Init(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;

    /* PA2 TX */
    GPIOA->MODER &= ~(3 << (2 * 2));
    GPIOA->MODER |=  (2 << (2 * 2));

    GPIOA->AFR[0] &= ~(0xF << (2 * 4));
    GPIOA->AFR[0] |=  (7 << (2 * 4));

    USART2->CR1 &= ~USART_CR1_UE;
    USART2->BRR = 1667;   // 16 MHz
    USART2->CR1 |= USART_CR1_TE;
    USART2->CR1 |= USART_CR3_DMAT;   // Enable DMA for TX
    USART2->CR1 |= USART_CR1_UE;
}

/* ================= DMA INIT ================= */

void DMA_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

    DMA1_Channel7->CCR &= ~DMA_CCR_EN;

    DMA1_Channel7->CPAR = (uint32_t)&USART2->TDR;
}

/* ================= SEND USING DMA ================= */

void UART2_Send_DMA(char *data, uint32_t length)
{
    DMA1_Channel7->CCR &= ~DMA_CCR_EN;

    DMA1_Channel7->CMAR = (uint32_t)data;
    DMA1_Channel7->CNDTR = length;

    DMA1_Channel7->CCR =
          DMA_CCR_MINC      |   // Memory increment
          DMA_CCR_DIR       |   // Memory to peripheral
          DMA_CCR_TCIE;         // Transfer complete interrupt optional

    DMA1_Channel7->CCR |= DMA_CCR_EN;

    while (DMA1_Channel7->CNDTR);  // Wait until done
}
