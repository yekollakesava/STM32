#include "spi.h"
#include "stm32l476xx.h"

void SPI1_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    SPI1->CR1 = 0;
    SPI1->CR2 = 0;

    SPI1->CR1 |= SPI_CR1_MSTR;
    SPI1->CR1 |= SPI_CR1_SSM;
    SPI1->CR1 |= SPI_CR1_SSI;

    /* Slowest clock (fPCLK/256) */
    SPI1->CR1 |= SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0;

    /* 8-bit mode */
    SPI1->CR2 |= (7 << SPI_CR2_DS_Pos);

    SPI1->CR2 |= SPI_CR2_FRXTH;

    SPI1->CR1 |= SPI_CR1_SPE;

    /* Flush RX buffer */
    volatile uint8_t dummy;
    while(SPI1->SR & SPI_SR_RXNE)
        dummy = *((__IO uint8_t*)&SPI1->DR);
}

/* SPI transfer */
uint8_t SPI1_Transfer(uint8_t data)
{
    uint32_t timeout = 0xFFFF;

    while(!(SPI1->SR & SPI_SR_TXE))
        if(--timeout == 0) return 0xEE;

    *((__IO uint8_t*)&SPI1->DR) = data;

    timeout = 0xFFFF;

    while(!(SPI1->SR & SPI_SR_RXNE))
        if(--timeout == 0) return 0xDD;

    return *((__IO uint8_t*)&SPI1->DR);
}

/* ================= CS CONTROL ON PB8 ================= */

void SD_CS_LOW(void)
{
    GPIOB->ODR &= ~(1 << 8);
}

void SD_CS_HIGH(void)
{
    GPIOB->ODR |= (1 << 8);
}
