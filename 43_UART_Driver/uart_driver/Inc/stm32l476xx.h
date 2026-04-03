#ifndef STM32L476XX_H
#define STM32L476XX_H

#include <stdint.h>

/* Base addresses */
#define RCC_BASE        0x40021000
#define GPIOA_BASE      0x48000000
#define USART2_BASE     0x40004400

/* RCC registers */
#define RCC_AHB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x4C))
#define RCC_APB1ENR1    (*(volatile uint32_t *)(RCC_BASE + 0x58))

/* GPIOA registers */
#define GPIOA_MODER     (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_AFRL      (*(volatile uint32_t *)(GPIOA_BASE + 0x20))

/* USART2 registers */
#define USART2_CR1      (*(volatile uint32_t *)(USART2_BASE + 0x00))
#define USART2_BRR      (*(volatile uint32_t *)(USART2_BASE + 0x0C))
#define USART2_ISR      (*(volatile uint32_t *)(USART2_BASE + 0x1C))
#define USART2_RDR      (*(volatile uint32_t *)(USART2_BASE + 0x24))
#define USART2_TDR      (*(volatile uint32_t *)(USART2_BASE + 0x28))

#endif
