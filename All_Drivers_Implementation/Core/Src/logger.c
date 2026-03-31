#include "logger.h"
#include "ff.h"
#include "uart.h"
#include <stdio.h>
#include <string.h>

static FATFS fs;
static FIL file;

/* ================== INIT ================== */
void Logger_Init(void)
{
    if(f_mount(&fs, "", 1) != FR_OK)
    {
        UART2_Print("Mount Failed\r\n");
        while(1);
    }

    UART2_Print("Mount OK\r\n");
}

/* ================== LOG TEMPERATURE ================== */
void Logger_LogTemperature(float temp)
{
    static char buffer[64];
    UINT bw;

    sprintf(buffer, "Temp: %.2f C\r\n", temp);

    if(f_open(&file, "temperature.txt", FA_OPEN_APPEND | FA_WRITE) == FR_OK)
    {
        f_write(&file, buffer, strlen(buffer), &bw);
        f_close(&file);
    }
}

/* ================== PRINT FILE ================== */
void Logger_PrintFile(void)
{
    static char buffer[64];
    UINT br;

    if(f_open(&file, "temperature.txt", FA_READ) != FR_OK)
    {
        UART2_Print("Open Error\r\n");
        return;
    }

    UART2_Print("\r\n--- FILE DATA ---\r\n");

    while(1)
    {
        if(f_read(&file, buffer, sizeof(buffer)-1, &br) != FR_OK || br == 0)
            break;

        buffer[br] = '\0';
        UART2_Print(buffer);
    }

    UART2_Print("\r\n--- END ---\r\n");

    f_close(&file);
}
