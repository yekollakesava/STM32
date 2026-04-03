#include "main.h"
#include "uart.h"
#include "ultrasonic.h"
#include "rtc.h"
#include "button.h"
#include "sdcard.h"
#include "ff.h"
#include <stdio.h>
#include <string.h>

/* FatFs objects */
FATFS fs;
FIL file;
char read_buffer[128];

/* Thresholds */
#define ENTRY_THRESHOLD_CM   20
#define EXIT_THRESHOLD_CM    25
#define CONFIRM_COUNT        3
#define LOG_FILE_NAME        "entrylog.txt"

static void delay_ms(unsigned int ms)
{
    volatile unsigned int i, j;
    for(i = 0; i < ms; i++)
    {
        for(j = 0; j < 1000; j++);
    }
}

static void log_to_sd(const char *msg)
{
    UINT bw;

    if(f_open(&file, LOG_FILE_NAME, FA_OPEN_APPEND | FA_WRITE) == FR_OK)
    {
        f_write(&file, msg, strlen(msg), &bw);
        f_close(&file);
    }
}

static void display_log_file(void)
{
    UINT br;

    UART2_Print("\r\n===== EVENT LOGS =====\r\n");

    if(f_open(&file, LOG_FILE_NAME, FA_READ) == FR_OK)
    {
        do
        {
            memset(read_buffer, 0, sizeof(read_buffer));
            f_read(&file, read_buffer, sizeof(read_buffer) - 1, &br);

            if(br > 0)
            {
                read_buffer[br] = '\0';
                UART2_Print(read_buffer);
            }

        } while(br > 0);

        f_close(&file);
    }
    else
    {
        UART2_Print("NO EVENTS STORED\r\n");
    }

    UART2_Print("======================\r\n");
}

int main(void)
{
    char buffer[160];
    char timeStr[20];
    char dateStr[20];

    unsigned int raw;
    unsigned int distance;

    int current_state = 0;   /* 0 = OUTSIDE, 1 = INSIDE */
    int entry_detect_count = 0;
    int exit_detect_count  = 0;

    UART2_Init();
    Ultrasonic_Init();
    RTC_Init();
    Button_Init();

    UART2_Print("\r\n==== Ultrasonic Entry Detection with SD Log ====\r\n");

    if(f_mount(&fs, "", 1) != FR_OK)
    {
        UART2_Print("SD Mount Failed\r\n");
    }
    else
    {
        UART2_Print("SD Mount OK\r\n");
    }

    UART2_Print("System Ready\r\n");

    while(1)
    {
        /* Button press -> display stored log file */
        if(Button_Pressed())
        {
            delay_ms(50);

            if(Button_Pressed())
            {
                display_log_file();
                while(Button_Pressed());
                delay_ms(100);
            }
        }

        raw = Ultrasonic_ReadDistance();

        /* If Ultrasonic_ReadDistance() already returns cm */
        distance = raw;

        /* If it returns microseconds instead, use this instead:
           distance = raw / 58;
        */

        if(distance == 0 || distance > 400)
        {
            delay_ms(100);
            continue;
        }

        if(current_state == 0)
        {
            if(distance < ENTRY_THRESHOLD_CM)
                entry_detect_count++;
            else
                entry_detect_count = 0;

            if(entry_detect_count >= CONFIRM_COUNT)
            {
                RTC_GetDateTime(dateStr, timeStr);

                sprintf(buffer,
                        "ENTERED -> Date: %s Time: %s Distance: %u cm\r\n",
                        dateStr, timeStr, distance);

                UART2_Print(buffer);
                log_to_sd(buffer);

                current_state = 1;
                entry_detect_count = 0;
                exit_detect_count = 0;

                delay_ms(500);
            }
        }
        else
        {
            if(distance > EXIT_THRESHOLD_CM)
                exit_detect_count++;
            else
                exit_detect_count = 0;

            if(exit_detect_count >= CONFIRM_COUNT)
            {
                RTC_GetDateTime(dateStr, timeStr);

                sprintf(buffer,
                        "LEFT    -> Date: %s Time: %s\r\n",
                        dateStr, timeStr);

                UART2_Print(buffer);
                log_to_sd(buffer);

                current_state = 0;
                exit_detect_count = 0;
                entry_detect_count = 0;

                delay_ms(500);
            }
        }

        delay_ms(100);
    }
}
