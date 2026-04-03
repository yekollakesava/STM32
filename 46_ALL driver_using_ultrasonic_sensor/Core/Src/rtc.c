#include "stm32l476xx.h"
#include "rtc.h"
#include <stdio.h>

#define RTC_INIT_FLAG  0x32F2

static unsigned int to_bcd(unsigned int val)
{
    return ((val / 10) << 4) | (val % 10);
}

static unsigned int from_bcd(unsigned int val)
{
    return (((val >> 4) & 0xF) * 10) + (val & 0xF);
}

void RTC_SetDateTime(unsigned char hour, unsigned char min, unsigned char sec,
                     unsigned char date, unsigned char month, unsigned char year)
{
    /* Disable write protection */
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

    /* Enter init mode */
    RTC->ISR |= (1U << 7);
    while(!(RTC->ISR & (1U << 6)));

    /* 24-hour format */
    RTC->CR &= ~(1U << 6);

    /* Set time in BCD */
    RTC->TR = (to_bcd(hour)  << 16) |
              (to_bcd(min)   << 8)  |
              (to_bcd(sec)   << 0);

    /* Set date in BCD */
    RTC->DR = (to_bcd(year)  << 16) |
              (to_bcd(month) << 8)  |
              (to_bcd(date)  << 0);

    /* Exit init mode */
    RTC->ISR &= ~(1U << 7);

    /* Enable write protection */
    RTC->WPR = 0xFF;
}

void RTC_Init(void)
{
    /* Enable PWR clock */
    RCC->APB1ENR1 |= (1U << 28);

    /* Enable access to backup domain */
    PWR->CR1 |= (1U << 8);

    /* If RTC not initialized already */
    if(RTC->BKP0R != RTC_INIT_FLAG)
    {
        /* Reset backup domain */
        RCC->BDCR |= (1U << 16);
        RCC->BDCR &= ~(1U << 16);

        /* Enable LSE */
        RCC->BDCR |= (1U << 0);
        while(!(RCC->BDCR & (1U << 1)));

        /* Select LSE as RTC clock */
        RCC->BDCR &= ~(3U << 8);
        RCC->BDCR |=  (1U << 8);

        /* Enable RTC */
        RCC->BDCR |= (1U << 15);

        /* Disable write protection */
        RTC->WPR = 0xCA;
        RTC->WPR = 0x53;

        /* Enter init mode */
        RTC->ISR |= (1U << 7);
        while(!(RTC->ISR & (1U << 6)));

        /* 24-hour format */
        RTC->CR &= ~(1U << 6);

        /* Prescaler for 1 second with 32.768 kHz LSE */
        RTC->PRER = (127U << 16) | 255U;

        /* Exit init mode */
        RTC->ISR &= ~(1U << 7);

        /* Enable write protection */
        RTC->WPR = 0xFF;

        /* Set initial date/time only once */
        RTC_SetDateTime(12, 30, 0, 27, 3, 26);

        /* Save init flag */
        RTC->BKP0R = RTC_INIT_FLAG;
    }
}

void RTC_GetDateTime(char *timeStr, char *dateStr)
{
    unsigned int tr, dr;
    unsigned int hour, min, sec;
    unsigned int date, month, year;

    tr = RTC->TR;
    dr = RTC->DR;

    hour  = from_bcd((tr >> 16) & 0xFF);
    min   = from_bcd((tr >> 8)  & 0x7F);
    sec   = from_bcd((tr >> 0)  & 0x7F);

    year  = from_bcd((dr >> 16) & 0xFF);
    month = from_bcd((dr >> 8)  & 0x1F);
    date  = from_bcd((dr >> 0)  & 0x3F);

    sprintf(timeStr, "%02u:%02u:%02u", hour, min, sec);
    sprintf(dateStr, "%02u-%02u-20%02u", date, month, year);
}
