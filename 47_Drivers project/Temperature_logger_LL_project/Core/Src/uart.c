#include "stm32l476xx.h"
#include "uart.h"

void UART2_Init(void)
{
    USART2->BRR = 4000000/9600;
    USART2->CR1 |= USART_CR1_TE;
    USART2->CR1 |= USART_CR1_UE;
}

void UART2_Print(char *msg)
{

	while(*msg)
    {
        while(!(USART2->ISR & USART_ISR_TXE));
        USART2->TDR = *msg++;
    }
}

void UART2_Print_Char(char c)
{
    while(!(USART2->ISR & USART_ISR_TXE));
    USART2->TDR = c;
}

void UART2_Print_Int(int32_t num)
{
    char buf[12];
    int i = 0;

    if(num == 0)
    {
        UART2_Print("0");
        return;
    }

    if(num < 0)
    {
        UART2_Print("-");
        num = -num;
    }

    while(num > 0)
    {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }

    while(i > 0)
    {
        UART2_Print_Char(buf[--i]);
    }
}


void UART2_Print_Float(float num)
{
    int int_part = (int)num;
    int frac_part = (int)((num - int_part) * 100);

    if(frac_part < 0)
        frac_part = -frac_part;

    UART2_Print_Int(int_part);
    UART2_Print(".");

    if(frac_part < 10)
        UART2_Print("0");

    UART2_Print_Int(frac_part);
}

void UART_SendChar(char c)
{
    while(!(USART2->ISR & USART_ISR_TXE));
    USART2->TDR = c;
}

void UART_SendString(char *str)
{
    while(*str)
    {
        UART_SendChar(*str++);
    }
}
