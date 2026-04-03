#ifndef RTC_H_
#define RTC_H_

#include "stm32l476xx.h"

void RTC_Init(void);
void RTC_Get_Time(uint8_t *hour, uint8_t *min, uint8_t *sec);
void RTC_Get_Date(uint8_t *date, uint8_t *month, uint8_t *year);

#endif
