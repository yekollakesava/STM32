#include "rtc.h"

static uint8_t bcd2dec(uint8_t bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

void RTC_Init(void)
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;
    PWR->CR1 |= PWR_CR1_DBP;

    /* Enable LSE */
    RCC->BDCR |= RCC_BDCR_LSEON;
    while(!(RCC->BDCR & RCC_BDCR_LSERDY));

    /* Select LSE */
    RCC->BDCR |= RCC_BDCR_RTCSEL_0;
    RCC->BDCR |= RCC_BDCR_RTCEN;

    /* If RTC not initialized */
    if(!(RTC->ISR & RTC_ISR_INITS))
    {
        RTC->WPR = 0xCA;
        RTC->WPR = 0x53;

        RTC->ISR |= RTC_ISR_INIT;
        while(!(RTC->ISR & RTC_ISR_INITF));

        RTC->PRER = (127 << 16) | 255;

        RTC->ISR &= ~RTC_ISR_INIT;
        RTC->WPR = 0xFF;
    }
}

void RTC_Get_Time(uint8_t *hour, uint8_t *min, uint8_t *sec)
{
    uint32_t tr = RTC->TR;

    *hour = bcd2dec((tr >> 16) & 0x3F);
    *min  = bcd2dec((tr >> 8) & 0x7F);
    *sec  = bcd2dec(tr & 0x7F);
}

void RTC_Get_Date(uint8_t *date, uint8_t *month, uint8_t *year)
{
    uint32_t dr = RTC->DR;

    *date  = bcd2dec(dr & 0x3F);
    *month = bcd2dec((dr >> 8) & 0x1F);
    *year  = bcd2dec((dr >> 16) & 0xFF);
}

static uint8_t dec2bcd(uint8_t val)
{
    return ((val/10)<<4) | (val%10);
}

void RTC_SetDateTime(uint8_t day, uint8_t month, uint8_t year,
                     uint8_t hour, uint8_t min, uint8_t sec)
{
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

    RTC->ISR |= RTC_ISR_INIT;
    while(!(RTC->ISR & RTC_ISR_INITF));

    RTC->TR = (dec2bcd(hour) << 16) |
              (dec2bcd(min) << 8)  |
               dec2bcd(sec);

    RTC->DR = (dec2bcd(year) << 16) |
              (dec2bcd(month) << 8) |
               dec2bcd(day);

    RTC->ISR &= ~RTC_ISR_INIT;
    RTC->WPR = 0xFF;
}
