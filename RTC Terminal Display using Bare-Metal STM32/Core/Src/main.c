#include "usart.h"
#include "rtc.h"
#include <stdio.h>

static void delay(void)
{
    for(volatile unsigned int i = 0; i < 800000; i++);
}

int main(void)
{
    RTC_TimeDate_t rtc;
    char msg[100];

    USART2_Init();
    RTC_Init();

    USART2_SendString("RTC Terminal Display Start\r\n");

    while(1)
    {
        RTC_Read(&rtc);

        sprintf(msg,
                "Time: %02d:%02d:%02d  Date: %02d-%02d-20%02d\r\n",
                rtc.hours,
                rtc.minutes,
                rtc.seconds,
                rtc.date,
                rtc.month,
                rtc.year);

        USART2_SendString(msg);

        delay();
    }
}
