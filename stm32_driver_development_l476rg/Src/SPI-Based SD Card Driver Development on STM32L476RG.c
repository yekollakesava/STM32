#include "spi.h"
#include "uart.h"
#include "sdcard.h"

int main(void)
{
    uint8_t ret;

    UART2_Init();
    UART2_SendString("\r\n-- SD CARD SPI PROJECT START ----\r\n");

    SPI1_GPIO_Init();
    UART2_SendString("SPI GPIO Init Done\r\n");

    SPI1_Init_Slow();
    UART2_SendString("SPI Init Slow Done\r\n");

    UART2_SendString("Calling SD_Init...\r\n");

    ret = SD_Init();

    if(ret == 0)
        UART2_SendString("SD init success\r\n");
    else
    {
        UART2_SendString("SD init failed code=");
        UART2_SendHex(ret);
        UART2_SendString("\r\n");
    }

    while(1);
}
