#ifndef RTC_H
#define RTC_H

void RTC_Init(void);
void RTC_SetDateTime(unsigned char hour, unsigned char min, unsigned char sec,
                     unsigned char date, unsigned char month, unsigned char year);
void RTC_GetDateTime(char *timeStr, char *dateStr);

#endif
