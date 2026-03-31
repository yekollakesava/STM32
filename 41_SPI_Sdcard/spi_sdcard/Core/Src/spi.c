#include "spi.h"

void SPI1_SetSpeedLow(void)
{
    SPI1->CR1 &= ~(7 << 3);
    SPI1->CR1 |=  (7 << 3);   // Slow speed (init)
}

void SPI1_SetSpeedHigh(void)
{
    SPI1->CR1 &= ~(7 << 3);
    SPI1->CR1 |=  (2 << 3);   // Faster after init
}

void SPI1_Init(void)
{
    // Enable clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // PA5 SCK, PA6 MISO, PA7 MOSI -> AF5
    GPIOA->MODER &= ~((3<<10)|(3<<12)|(3<<14));
    GPIOA->MODER |=  (2<<10)|(2<<12)|(2<<14);

    GPIOA->AFR[0] |= (5<<20)|(5<<24)|(5<<28);

    // PA4 as CS output
    GPIOA->MODER &= ~(3<<8);
    GPIOA->MODER |=  (1<<8);

    GPIOA->OSPEEDR |= (3<<8);

    GPIOA->BSRR = (1<<4); // CS High

    SPI1->CR1 = 0;
    SPI1->CR1 |= SPI_CR1_MSTR;      // Master
    SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;
    SPI1->CR1 |= SPI_CR1_SPE;

    SPI1_SetSpeedLow();
}

uint8_t SPI1_Transfer(uint8_t data)
{
    while(!(SPI1->SR & SPI_SR_TXE));
    *((__IO uint8_t*)&SPI1->DR) = data;

    while(!(SPI1->SR & SPI_SR_RXNE));
    return *((__IO uint8_t*)&SPI1->DR);
}
