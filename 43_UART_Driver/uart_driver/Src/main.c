#include "uart.h"

void delay()
{
    for(volatile int i=0;i<500000;i++);
}

int main()
{
    UART2_Init();

    while(1)
    {
        UART2_SendString("Hello from STM32 UART\r\n");
        delay();
    }
}
