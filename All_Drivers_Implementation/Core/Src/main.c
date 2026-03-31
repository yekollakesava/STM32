#include "stm32l476xx.h"
#include "rcc.h"
#include "gpio.h"
#include "uart.h"
#include "spi.h"
#include "i2c.h"
#include "sdcard.h"
#include "ff.h"
#include "bme280.h"
#include "rtc.h"
#include <stdio.h>
#include <string.h>

#define SAMPLE_COUNT 10

FATFS fs;
FIL file;
UINT bw, br;

char read_buffer[128];

void delay_ms(uint32_t ms)
{
    for(uint32_t i=0;i<ms*4000;i++);
}

int main(void)
{
    RCC_Init();
    GPIO_Init();
    UART2_Init();
    SPI1_Init();
    I2C1_Init();
    RTC_Init();
    BME280_Init();

    UART2_Print("System Ready\r\n");

    /* Mount SD */
    if(f_mount(&fs, "", 1) != FR_OK)
    {
        UART2_Print("Mount Failed\r\n");
        while(1);
    }

    UART2_Print("Mount OK\r\n");

    float temp_buffer[SAMPLE_COUNT];
    uint8_t index = 0;

    while(1)
    {
        /* ===== Read Temperature ===== */
        temp_buffer[index++] = BME280_ReadTemperature();
        delay_ms(300);

        /* ===== When 10 samples collected ===== */
        if(index >= SAMPLE_COUNT)
        {
            if(f_open(&file, "T-Log1.TXT",
                      FA_OPEN_ALWAYS | FA_WRITE) == FR_OK)
            {
                /* Move to end for append */
                f_lseek(&file, f_size(&file));

                for(uint8_t i = 0; i < SAMPLE_COUNT; i++)
                {
                    char line[32];
                    sprintf(line, "Temp: %.2f C\r\n", temp_buffer[i]);
                    f_write(&file, line, strlen(line), &bw);
                }

                f_close(&file);
                UART2_Print("10 Logs Stored\r\n");
            }
            else
            {
                UART2_Print("File Open Error\r\n");
            }

            index = 0;
        }

        /* ===== Button Press -> Print File ===== */
        if(Button_Pressed())
        {
            delay_ms(200);   // debounce

            if(Button_Pressed())
            {
                UART2_Print("\r\n--- TEMP LOGS ---\r\n");

                if(f_open(&file, "T-Log1.TXT", FA_READ) == FR_OK)
                {
                    while(1)
                    {
                        if(f_read(&file, read_buffer,
                                  sizeof(read_buffer)-1, &br) != FR_OK || br == 0)
                            break;

                        read_buffer[br] = 0;
                        UART2_Print(read_buffer);
                    }

                    f_close(&file);
                }
                else
                {
                    UART2_Print("Read Error\r\n");
                }

                UART2_Print("\r\n-----------------\r\n");

                while(Button_Pressed());  // wait for release
            }
        }
    }
}
