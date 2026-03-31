#include "spi.h"
#include "sdcard.h"
#include <string.h>

int main(void)
{
    uint8_t write_buf[512] = {0};
    uint8_t read_buf[512]  = {0};

    // Put message into buffer
    const char *msg = "Hello from STM32";

    strcpy((char*)write_buf, msg);

    SPI1_Init();

    if(SD_Init() == 0)
    {
        if(SD_WriteBlock(2048, write_buf) == 0)
        {
            if(SD_ReadBlock(2048, read_buf) == 0)
            {
                while(1); // SUCCESS
            }
        }
    }

    while(1); // ERROR
}
