#include "uart.h"
#include "stm32l476xx.h"

#define USART_TXE (1<<7)
#define USART_RXNE (1<<5)

void UART2_Init()
{
	RCC_AHB2ENR |= (1<<0);      // Enable GPIOA clock
	    RCC_APB1ENR1 |= (1<<17);    // Enable USART2 clock

	    GPIOA_MODER &= ~(0xF<<4);
	    GPIOA_MODER |=  (0xA<<4);   // PA2 PA3 alternate function

	    GPIOA_AFRL &= ~(0xFF<<8);
	    GPIOA_AFRL |=  (0x77<<8);   // AF7 for USART2

	    USART2_CR1 &= ~(1<<0);      // Disable UART

	    USART2_BRR = 416;           // FIXED BAUD RATE

	    USART2_CR1 |= (1<<3);       // TX enable
	    USART2_CR1 |= (1<<2);       // RX enable

	    USART2_CR1 |= (1<<0);       // Enable UART
}

void UART2_SendChar(char ch)
{
    while(!(USART2_ISR & USART_TXE));
    USART2_TDR = ch;
}

void UART2_SendString(char *str)
{
    while(*str)
    {
        UART2_SendChar(*str++);
    }
}

char UART2_ReadChar()
{
    while(!(USART2_ISR & USART_RXNE));
    return USART2_RDR;
}
