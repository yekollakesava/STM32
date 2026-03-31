#include "stm32l47xx.h"
#include "rtc.h"

static uint8_t to_bcd(uint8_t val)
{
    return ((val / 10) << 4) | (val % 10);
}

static uint8_t from_bcd(uint8_t val)
{
    return (((val >> 4) & 0x0F) * 10) + (val & 0x0F);
}

void RTC_Init(void)
{
    RCC_APB1ENR1 |= PWR_EN;
    PWR_CR1 |= PWR_DBP;

    /* LSE ON */
    RCC_BDCR |= (1U << 0);
    while(!(RCC_BDCR & (1U << 1)));

    /* RTC clock select = LSE */
    RCC_BDCR &= ~(3U << 8);
    RCC_BDCR |=  (1U << 8);

    /* RTC enable */
    RCC_BDCR |= (1U << 15);

    /* Disable write protection */
    RTC_WPR = 0xCA;
    RTC_WPR = 0x53;

    /* Enter init mode */
    RTC_ISR |= RTC_INIT;
    while(!(RTC_ISR & RTC_INITF));

    /* 32.768 kHz -> 1 Hz */
    RTC_PRER = (127U << 16) | 255U;

    /* 24-hour format */
    RTC_CR &= ~(1U << 6);

    /* Initial time = 17:30:00 */
    RTC_TR = ((uint32_t)to_bcd(17) << 16) |
             ((uint32_t)to_bcd(30) << 8)  |
             ((uint32_t)to_bcd(0));

    /* Initial date = 24-03-26 */
    RTC_DR = ((uint32_t)to_bcd(26) << 16) |
             ((uint32_t)to_bcd(3)  << 8)  |
             ((uint32_t)to_bcd(24));

    /* Exit init mode */
    RTC_ISR &= ~RTC_INIT;

    /* Enable write protection */
    RTC_WPR = 0xFF;
}

void RTC_Read(RTC_TimeDate_t *rtc)
{
    uint32_t tr = RTC_TR;
    uint32_t dr = RTC_DR;

    rtc->seconds = from_bcd((uint8_t)(tr & 0x7F));
    rtc->minutes = from_bcd((uint8_t)((tr >> 8) & 0x7F));
    rtc->hours   = from_bcd((uint8_t)((tr >> 16) & 0x3F));

    rtc->date    = from_bcd((uint8_t)(dr & 0x3F));
    rtc->month   = from_bcd((uint8_t)((dr >> 8) & 0x1F));
    rtc->year    = from_bcd((uint8_t)((dr >> 16) & 0xFF));
}
