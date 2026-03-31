#include "ff.h"
#include "sdcard.h"
#include "uart.h"
#include <string.h>

extern FIL file;

int SD_Write_Log(const char *text)
{
    UINT bw;

    if(f_open(&file, "log.txt", FA_OPEN_ALWAYS | FA_WRITE) != FR_OK)
        return 0;

    if(f_lseek(&file, f_size(&file)) != FR_OK)
    {
        f_close(&file);
        return 0;
    }

    if(f_write(&file, text, strlen(text), &bw) != FR_OK)
    {
        f_close(&file);
        return 0;
    }

    f_close(&file);
    return 1;
}

int SD_Read_Log_To_UART(void)
{
    UINT br;
    char readBuf[64];

    if(f_open(&file, "log.txt", FA_READ) != FR_OK)
        return 0;

    UART2_Print("\r\n----- LOG FILE DATA -----\r\n");

    while(1)
    {
        if(f_read(&file, readBuf, sizeof(readBuf) - 1, &br) != FR_OK)
        {
            f_close(&file);
            return 0;
        }

        if(br == 0)
            break;

        readBuf[br] = '\0';
        UART2_Print(readBuf);
    }

    UART2_Print("\r\n----- END OF FILE -----\r\n");

    f_close(&file);
    return 1;
}
