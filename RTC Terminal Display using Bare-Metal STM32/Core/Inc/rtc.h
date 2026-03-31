#ifndef RTC_H
#define RTC_H

#include <stdint.h>

typedef struct
{
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint8_t date;
    uint8_t month;
    uint8_t year;
} RTC_TimeDate_t;

void RTC_Init(void);
void RTC_Read(RTC_TimeDate_t *rtc);

#endif
