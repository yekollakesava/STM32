#ifndef MY_STM32L476XX_H
#define MY_STM32L476XX_H

#include <stdint.h>

/* ---------------- Base addresses ---------------- */
#define RCC_BASE        0x40021000UL
#define GPIOA_BASE      0x48000000UL
#define USART2_BASE     0x40004400UL
#define RTC_BASE        0x40002800UL
#define PWR_BASE        0x40007000UL

/* ---------------- RCC registers ---------------- */
#define RCC_AHB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x4C))
#define RCC_APB1ENR1    (*(volatile uint32_t *)(RCC_BASE + 0x58))
#define RCC_BDCR        (*(volatile uint32_t *)(RCC_BASE + 0x90))

/* ---------------- GPIOA registers ---------------- */
#define GPIOA_MODER     (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_OSPEEDR   (*(volatile uint32_t *)(GPIOA_BASE + 0x08))
#define GPIOA_AFRL      (*(volatile uint32_t *)(GPIOA_BASE + 0x20))

/* ---------------- USART2 registers ---------------- */
#define USART2_CR1      (*(volatile uint32_t *)(USART2_BASE + 0x00))
#define USART2_BRR      (*(volatile uint32_t *)(USART2_BASE + 0x0C))
#define USART2_ISR      (*(volatile uint32_t *)(USART2_BASE + 0x1C))
#define USART2_TDR      (*(volatile uint32_t *)(USART2_BASE + 0x28))

/* ---------------- RTC registers ---------------- */
#define RTC_TR          (*(volatile uint32_t *)(RTC_BASE + 0x00))
#define RTC_DR          (*(volatile uint32_t *)(RTC_BASE + 0x04))
#define RTC_CR          (*(volatile uint32_t *)(RTC_BASE + 0x08))
#define RTC_ISR         (*(volatile uint32_t *)(RTC_BASE + 0x0C))
#define RTC_PRER        (*(volatile uint32_t *)(RTC_BASE + 0x10))
#define RTC_WPR         (*(volatile uint32_t *)(RTC_BASE + 0x24))

/* ---------------- PWR registers ---------------- */
#define PWR_CR1         (*(volatile uint32_t *)(PWR_BASE + 0x00))

/* ---------------- Bit macros ---------------- */
#define GPIOA_EN        (1U << 0)
#define USART2_EN       (1U << 17)
#define PWR_EN          (1U << 28)

#define USART_UE        (1U << 0)
#define USART_TE        (1U << 3)
#define USART_TXE       (1U << 7)

#define PWR_DBP         (1U << 8)

#define RTC_INIT        (1U << 7)
#define RTC_INITF       (1U << 6)

#endif
