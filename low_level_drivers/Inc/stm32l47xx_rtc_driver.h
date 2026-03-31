#ifndef INC_STM32L47XX_RTC_DRIVER_H_
#define INC_STM32L47XX_RTC_DRIVER_H_

#include "stm32l47xx.h"
#include <stdint.h>

typedef struct
{
    uint8_t date;
    uint8_t month;
    uint8_t year;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} RTC_DateTime_t;

/* RTC driver APIs */
void RTC_Init(void);
void RTC_SetDateTime(uint8_t date, uint8_t month, uint8_t year,
                     uint8_t hours, uint8_t minutes, uint8_t seconds);
void RTC_GetDateTime(RTC_DateTime_t *dt);

#endif /* INC_STM32L47XX_RTC_DRIVER_H_ */
