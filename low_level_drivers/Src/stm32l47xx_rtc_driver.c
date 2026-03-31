/*
 * stm32l47xx_rtc_driver.c
 *
 *  Created on: Mar 25, 2026
 *      Author: yekol
 */


#include "stm32l47xx_rtc_driver.h"

static uint8_t RTC_BcdToDec(uint8_t val)
{
    return (uint8_t)(((val >> 4U) * 10U) + (val & 0x0FU));
}

static uint8_t RTC_DecToBcd(uint8_t val)
{
    return (uint8_t)(((val / 10U) << 4U) | (val % 10U));
}

void RTC_Init(void)
{
    /* Enable PWR clock */
    RCC->APB1ENR1 |= (1U << 28);

    /* Enable access to backup domain */
    PWR->CR1 |= (1U << 8);

    /* Enable LSE */
    RCC->BDCR |= (1U << 0);
    while (!(RCC->BDCR & (1U << 1)));

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
    while (!(RTC->ISR & (1U << 6)));

    /* 32.768 kHz / (127 + 1) / (255 + 1) = 1 Hz */
    RTC->PRER = (127U << 16) | 255U;

    /* Exit init mode */
    RTC->ISR &= ~(1U << 7);

    /* Enable write protection */
    RTC->WPR = 0xFF;
}

void RTC_SetDateTime(uint8_t date, uint8_t month, uint8_t year,
                     uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    /* Disable write protection */
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

    /* Enter init mode */
    RTC->ISR |= (1U << 7);
    while (!(RTC->ISR & (1U << 6)));

    /* Time register */
    RTC->TR =
        ((RTC_DecToBcd(hours)   & 0x3FU) << 16) |
        ((RTC_DecToBcd(minutes) & 0x7FU) << 8)  |
        ((RTC_DecToBcd(seconds) & 0x7FU) << 0);

    /* Date register
       Weekday kept as 1 */
    RTC->DR =
        ((RTC_DecToBcd(year)  & 0xFFU) << 16) |
        ((RTC_DecToBcd(month) & 0x1FU) << 8)  |
        ((RTC_DecToBcd(date)  & 0x3FU) << 0)  |
        (1U << 13);

    /* Exit init mode */
    RTC->ISR &= ~(1U << 7);

    /* Enable write protection */
    RTC->WPR = 0xFF;
}

void RTC_GetDateTime(RTC_DateTime_t *dt)
{
    uint32_t tr;
    uint32_t dr;

    if (dt == 0)
        return;

    tr = RTC->TR;
    dr = RTC->DR;

    dt->seconds = RTC_BcdToDec((uint8_t)(tr & 0x7FU));
    dt->minutes = RTC_BcdToDec((uint8_t)((tr >> 8) & 0x7FU));
    dt->hours   = RTC_BcdToDec((uint8_t)((tr >> 16) & 0x3FU));

    dt->date    = RTC_BcdToDec((uint8_t)(dr & 0x3FU));
    dt->month   = RTC_BcdToDec((uint8_t)((dr >> 8) & 0x1FU));
    dt->year    = RTC_BcdToDec((uint8_t)((dr >> 16) & 0xFFU));
}
