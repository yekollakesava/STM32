#include "stm32l47xx.h"
#include "spi.h"

void SPI1_Init(void)
{
    /* Enable clocks */
    RCC->AHB2ENR |= (1U << 0);    // GPIOA clock
    RCC->APB2ENR |= (1U << 12);   // SPI1 clock

    /* PA5 = SCK, PA6 = MISO, PA7 = MOSI -> AF5 */
    GPIOA->MODER &= ~(0x3FU << 10);
    GPIOA->MODER |=  (0x2AU << 10);

    GPIOA->AFR[0] &= ~(0xFFFU << 20);
    GPIOA->AFR[0] |=  (0x555U << 20);

    GPIOA->OSPEEDR |= (0x3FU << 10);

    /* Disable SPI before config */
    SPI1->CR1 &= ~(1U << 6);

    SPI1->CR1 = 0;
    SPI1->CR2 = 0;

    /* Master, software NSS, internal NSS high */
    SPI1->CR1 |= (1U << 2);   // MSTR
    SPI1->CR1 |= (1U << 8);   // SSI
    SPI1->CR1 |= (1U << 9);   // SSM

    /* Slow baud rate for SD init */
    SPI1->CR1 |= (7U << 3);   // BR = fPCLK/256

    /* CPOL = 0, CPHA = 0 -> SPI mode 0 */

    /* 8-bit data size */
    SPI1->CR2 |= (7U << 8);   // DS = 0111 => 8-bit

    /* RXNE event when FIFO level >= 8 bits */
    SPI1->CR2 |= (1U << 12);  // FRXTH

    /* Enable SPI */
    SPI1->CR1 |= (1U << 6);   // SPE
}

uint8_t SPI1_Transfer(uint8_t data)
{
    /* Wait until TX buffer empty */
    while (!(SPI1->SR & (1U << 1)))
    {
    }

    /* Write 8-bit data */
    *((volatile uint8_t *)&SPI1->DR) = data;

    /* Wait until receive buffer not empty */
    while (!(SPI1->SR & (1U << 0)))
    {
    }

    /* Read 8-bit received data */
    return *((volatile uint8_t *)&SPI1->DR);
}
