#include "spi_driver.h"

/* Base Addresses */
#define RCC_BASE        0x40023800
#define GPIOA_BASE      0x40020000
#define SPI1_BASE       0x40013000

/* RCC Registers */
#define RCC_AHB1ENR   (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_APB2ENR   (*(volatile uint32_t *)(RCC_BASE + 0x44))

/* GPIOA Registers */
#define GPIOA_MODER   (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_AFRL    (*(volatile uint32_t *)(GPIOA_BASE + 0x20))

/* SPI1 Registers */
#define SPI1_CR1      (*(volatile uint32_t *)(SPI1_BASE + 0x00))
#define SPI1_SR       (*(volatile uint32_t *)(SPI1_BASE + 0x08))
#define SPI1_DR       (*(volatile uint32_t *)(SPI1_BASE + 0x0C))

void SPI1_Init(void)
{
    /* 1. Enable Clocks */
    RCC_AHB1ENR |= (1 << 0);   // GPIOA clock
    RCC_APB2ENR |= (1 << 12);  // SPI1 clock

    /* 2. Set PA5, PA6, PA7 to Alternate Function */
    GPIOA_MODER &= ~(0x3F << 10);  // Clear bits for PA5,6,7
    GPIOA_MODER |=  (0x2A << 10);  // Set AF mode (10)

    /* 3. Select AF5 for SPI1 */
    GPIOA_AFRL &= ~(0xFFF << 20);
    GPIOA_AFRL |=  (0x555 << 20);  // AF5 = 0101

    /* 4. SPI Configuration */
    SPI1_CR1 = 0;

    SPI1_CR1 |= (1 << 2);   // Master mode
    SPI1_CR1 |= (3 << 3);   // Baud rate (div16)
    SPI1_CR1 |= (1 << 9);   // SSM
    SPI1_CR1 |= (1 << 8);   // SSI
    SPI1_CR1 |= (1 << 6);   // SPI Enable
}

void SPI1_Transmit(uint8_t data)
{
    while(!(SPI1_SR & (1 << 1)));   // Wait TXE
    SPI1_DR = data;
}

uint8_t SPI1_Receive(void)
{
    while(!(SPI1_SR & (1 << 0)));   // Wait RXNE
    return SPI1_DR;
}



