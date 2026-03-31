#include "spi.h"

/* RCC bits */
#define GPIOA_EN_BIT            0
#define SPI1_EN_BIT             12

/* SPI bits */
#define SPI_CR1_CPHA_BIT        0
#define SPI_CR1_CPOL_BIT        1
#define SPI_CR1_MSTR_BIT        2
#define SPI_CR1_BR_POS          3
#define SPI_CR1_SPE_BIT         6
#define SPI_CR1_SSI_BIT         8
#define SPI_CR1_SSM_BIT         9

/* SPI SR bits */
#define SPI_SR_RXNE_BIT         0
#define SPI_SR_TXE_BIT          1

void SPI1_GPIO_Init(void)
{
    /* Enable GPIOA clock */
    RCC->AHB2ENR |= (1U << GPIOA_EN_BIT);

    /*
     * PA5 -> SPI1_SCK  AF5
     * PA6 -> SPI1_MISO AF5
     * PA7 -> SPI1_MOSI AF5
     */
    GPIOA->MODER &= ~(0x3FU << 10);
    GPIOA->MODER |=  (0x2AU << 10);

    GPIOA->AFR[0] &= ~(0xFFFU << 20);
    GPIOA->AFR[0] |=  (0x555U << 20);

    GPIOA->OSPEEDR |= (0x3FU << 10);

    /* PA4 as GPIO output for CS */
    GPIOA->MODER &= ~(0x3U << 8);
    GPIOA->MODER |=  (0x1U << 8);

    GPIOA->OTYPER &= ~(1U << 4);
    GPIOA->OSPEEDR |= (0x3U << 8);

    SD_CS_HIGH();
}

void SPI1_Init_Slow(void)
{
    /* Enable SPI1 clock */
    RCC->APB2ENR |= (1U << SPI1_EN_BIT);

    SPI1->CR1 = 0;

    /* Master mode */
    SPI1->CR1 |= (1U << SPI_CR1_MSTR_BIT);

    /* Software slave management */
    SPI1->CR1 |= (1U << SPI_CR1_SSM_BIT);
    SPI1->CR1 |= (1U << SPI_CR1_SSI_BIT);

    /* Baud rate = fPCLK / 256 => BR[2:0] = 111 */
    SPI1->CR1 &= ~(7U << SPI_CR1_BR_POS);
    SPI1->CR1 |=  (7U << SPI_CR1_BR_POS);

    /* SPI mode 0 */
    SPI1->CR1 &= ~(1U << SPI_CR1_CPOL_BIT);
    SPI1->CR1 &= ~(1U << SPI_CR1_CPHA_BIT);

    /* Enable SPI */
    SPI1->CR1 |= (1U << SPI_CR1_SPE_BIT);
}

void SPI1_SetFast(void)
{
    /* Disable SPI before changing BR */
    SPI1->CR1 &= ~(1U << SPI_CR1_SPE_BIT);

    /* Baud rate = fPCLK / 8 => BR[2:0] = 010 */
    SPI1->CR1 &= ~(7U << SPI_CR1_BR_POS);
    SPI1->CR1 |=  (2U << SPI_CR1_BR_POS);

    /* Enable SPI */
    SPI1->CR1 |= (1U << SPI_CR1_SPE_BIT);
}

uint8_t SPI1_Transfer(uint8_t data)
{
    while(!(SPI1->SR & (1U << SPI_SR_TXE_BIT)));
    *((volatile uint8_t*)&SPI1->DR) = data;

    while(!(SPI1->SR & (1U << SPI_SR_RXNE_BIT)));
    return *((volatile uint8_t*)&SPI1->DR);
}

void delay_ms(uint32_t ms)
{
    for(uint32_t i = 0; i < ms; i++)
    {
        for(volatile uint32_t j = 0; j < 4000; j++);
    }
}
