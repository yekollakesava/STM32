#include "stm32l47xx.h"

void USART2_Init(void)
{
    RCC->AHB2ENR  |= (1U << 0);
    RCC->APB1ENR1 |= (1U << 17);

    GPIOA->MODER &= ~(0xFU << 4);
    GPIOA->MODER |=  (0xAU << 4);

    GPIOA->AFR[0] &= ~(0xFFU << 8);
    GPIOA->AFR[0] |=  (0x77U << 8);

    USART2->BRR = 417;   // OR 1667 (use the one that worked for TX)

    USART2->CR1 = 0;
    USART2->CR1 |= (1U << 3);
    USART2->CR1 |= (1U << 2);
    USART2->CR1 |= (1U << 0);
}

void USART2_SendChar(char ch)
{
    while(!(USART2->ISR & (1U << 7)));
    USART2->TDR = ch;
}

void USART2_SendString(char *str)
{
    while(*str)
    {
        USART2_SendChar(*str++);
    }
}

void delay(void)
{
    for(volatile int i=0;i<300000;i++);
}

int main(void)
{
    USART2_Init();

    USART2_SendString("UART READY\r\n");

    while(1)
    {
        if(USART2->ISR & (1U << 5))   // RXNE
        {
            char ch = (char)USART2->RDR;

            USART2_SendString("RX: ");
            USART2_SendChar(ch);
            USART2_SendString("\r\n");
        }
        else
        {
            USART2_SendString("No RX\r\n");
            delay();
        }
    }
}
