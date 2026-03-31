#include "uart.h"
#include "stm32l47xx.h"
#include <stdint.h>

/* bit positions */
#define GPIOA_EN_BIT            0
#define USART2_EN_BIT           17

#define USART_CR1_UE_BIT        0
#define USART_CR1_RE_BIT        2
#define USART_CR1_TE_BIT        3
#define USART_CR1_PCE_BIT       10
#define USART_CR1_M0_BIT        12
#define USART_CR1_M1_BIT        28

#define USART_ISR_TXE_BIT       7

static void UART2_GPIO_Init(void)
{
    /* Enable GPIOA clock */
    RCC->AHB2ENR |= (1U << GPIOA_EN_BIT);

    /* PA2 -> TX, PA3 -> RX, AF7 */
    GPIOA->MODER &= ~(0xFU << 4);
    GPIOA->MODER |=  (0xAU << 4);

    GPIOA->AFR[0] &= ~(0xFFU << 8);
    GPIOA->AFR[0] |=  (0x77U << 8);

    GPIOA->OSPEEDR |= (0xFU << 4);
}

void UART2_Init(void)
{
    UART2_GPIO_Init();

    /* Enable USART2 clock */
    RCC->APB1ENR1 |= (1U << USART2_EN_BIT);

    USART2->CR1 = 0;

    /* 8-bit data */
    USART2->CR1 &= ~(1U << USART_CR1_M0_BIT);
    USART2->CR1 &= ~(1U << USART_CR1_M1_BIT);

    /* no parity */
    USART2->CR1 &= ~(1U << USART_CR1_PCE_BIT);

    /* 1 stop bit */
    USART2->CR2 &= ~(3U << 12);

    /* assuming 16 MHz clock */
  //  USART2->BRR = 16000000U / 115200U;
    //USART2->BRR = 80000000 / 115200;
    USART2->BRR = 4000000U / 9600U;

    USART2->CR1 |= (1U << USART_CR1_TE_BIT);
    USART2->CR1 |= (1U << USART_CR1_RE_BIT);
    USART2->CR1 |= (1U << USART_CR1_UE_BIT);
}

void UART2_SendChar(char c)
{
    while(!(USART2->ISR & (1U << USART_ISR_TXE_BIT)));
    USART2->TDR = (uint8_t)c;
}

void UART2_SendString(const char *str)
{
    while(*str)
    {
        UART2_SendChar(*str++);
    }
}

void UART2_SendHex(uint8_t data)
{
    const char hex[] = "0123456789ABCDEF";
    UART2_SendChar(hex[(data >> 4) & 0x0F]);
    UART2_SendChar(hex[data & 0x0F]);
}
