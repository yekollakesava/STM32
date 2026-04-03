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

#define SAMPLE_COUNT        10
#define LOG_FILE_NAME       "T-Log3.TXT"
#define READ_BUFFER_SIZE    128
#define DEBOUNCE_DELAY_MS   200
#define SAMPLE_DELAY_MS     300

FATFS fs;
FIL file;
UINT bw, br;

char read_buffer[READ_BUFFER_SIZE];

/* Simple software delay */
void delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < (ms * 4000U); i++);
}

/* -------------------- Helper Functions -------------------- */

/* Store 10 temperature samples into SD card */
static void Store_Temperature_Logs(float *temp_buffer, uint8_t count)
{
    if (f_open(&file, LOG_FILE_NAME, FA_OPEN_ALWAYS | FA_WRITE) == FR_OK)
    {
        char line[32];

        /* Move file pointer to end for appending */
        f_lseek(&file, f_size(&file));

        for (uint8_t i = 0; i < count; i++)
        {
            sprintf(line, "Temp: %.2f C\r\n", temp_buffer[i]);
            f_write(&file, line, strlen(line), &bw);
        }

        f_close(&file);
        UART2_Print("10 logs stored\r\n");
    }
    else
    {
        UART2_Print("File Open Error\r\n");
    }
}

/* Read and print stored logs on UART */
static void Print_Temperature_Logs(void)
{
    UART2_Print("\r\n--- TEMPERATURE LOGS ---\r\n");

    if (f_open(&file, LOG_FILE_NAME, FA_READ) == FR_OK)
    {
        while (1)
        {
            if ((f_read(&file, read_buffer, READ_BUFFER_SIZE - 1, &br) != FR_OK) || (br == 0))
            {
                break;
            }

            read_buffer[br] = '\0';
            UART2_Print(read_buffer);
        }

        f_close(&file);
    }
    else
    {
        UART2_Print("Read Error\r\n");
    }

    UART2_Print("\r\n-----------------\r\n");
}

/* Check button press with simple debounce */
static uint8_t Is_Button_Pressed_Stable(void)
{
    if (Button_Pressed())
    {
        delay_ms(DEBOUNCE_DELAY_MS);

        if (Button_Pressed())
        {
            return 1;
        }
    }

    return 0;
}

/* -------------------- Main Function -------------------- */

int main(void)
{
    float temp_buffer[SAMPLE_COUNT];
    uint8_t sample_index = 0;

    /* Peripheral initialization */
    RCC_Init();
    GPIO_Init();
    UART2_Init();
    SPI1_Init();
    I2C1_Init();
    RTC_Init();
    BME280_Init();

    UART2_Print("System Ready\r\n");

    /* Mount SD card */
    if (f_mount(&fs, "", 1) != FR_OK)
    {
        UART2_Print("Mount Failed\r\n");
        while (1);
    }

    UART2_Print("Mount OK\r\n");

    while (1)
    {
        /* Read one temperature sample */
        temp_buffer[sample_index] = BME280_ReadTemperature();
        sample_index++;

        delay_ms(SAMPLE_DELAY_MS);

        /* Once 10 samples are collected, store them */
        if (sample_index >= SAMPLE_COUNT)
        {
            Store_Temperature_Logs(temp_buffer, SAMPLE_COUNT);
            sample_index = 0;
        }

        /* If button is pressed, print stored logs */
        if (Is_Button_Pressed_Stable())
        {
            Print_Temperature_Logs();

            /* Wait until button is released */
            while (Button_Pressed());
        }
    }
}
