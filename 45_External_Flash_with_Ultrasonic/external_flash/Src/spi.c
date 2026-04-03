#include "spi.h"

#define RCC_APB2ENR (*(volatile uint32_t*)0x40021060)
#define SPI1_CR1 (*(volatile uint32_t*)0x40013000)
#define SPI1_SR  (*(volatile uint32_t*)0x40013008)
#define SPI1_DR  (*(volatile uint32_t*)0x4001300C)

void SPI_Init()
{
    RCC_APB2ENR |= (1<<12);

    SPI1_CR1 = (1<<2) | (1<<6) | (1<<9);
}

uint8_t SPI_Transfer(uint8_t data)
{
    SPI1_DR = data;

    while(!(SPI1_SR & (1<<1)));

    while(!(SPI1_SR & (1<<0)));

    return SPI1_DR;
}
