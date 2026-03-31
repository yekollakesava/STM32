#include "stm32l47xx.h"
#include "stm32l47xx_usart_driver.h"
#include "delay.h"
#include <stdint.h>

USART_Handle_t USART2Handle;

void UART2_Init_UserDriver(void)
{
    USART2Handle.pUSARTx = USART2;
    USART2Handle.USART_Config.USART_Baud = USART_STD_BAUD_9600;
    USART2Handle.USART_Config.USART_HWFlowControl = USART_HW_FLOW_CTRL_NONE;
    USART2Handle.USART_Config.USART_Mode = USART_MODE_ONLY_TX;
    USART2Handle.USART_Config.USART_NoOfStopBits = USART_STOPBITS_1;
    USART2Handle.USART_Config.USART_ParityControl = USART_PARITY_DISABLE;
    USART2Handle.USART_Config.USART_WordLength = USART_WORDLEN_8BITS;

    USART_Init(&USART2Handle);
}

void UART2_SendString(char *msg)
{
    while(*msg)
    {
        USART_SendData(&USART2Handle, (uint8_t *)msg, 1);
        msg++;
    }
}

int main(void)
{
    UART2_Init_UserDriver();

    while(1)
    {
        UART2_SendString("USART DRIVER OK\r\n");
        delay_ms(1000);
    }
}
