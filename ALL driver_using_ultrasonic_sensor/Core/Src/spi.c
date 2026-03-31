#include "spi.h"

/*
 * SPI1
 * PA4 -> CS
 * PA5 -> SCK
 * PA6 -> MISO
 * PA7 -> MOSI
 */

void SPI1_Init(void)
{
    /* Enable GPIOA and SPI1 clocks */
    RCC->AHB2ENR |= (1U << 0);    /* GPIOAEN */
    RCC->APB2ENR |= (1U << 12);   /* SPI1EN */

    /* PA5, PA6, PA7 alternate function */
    GPIOA->MODER &= ~(0x3FU << 10);
    GPIOA->MODER |=  (0x2AU << 10);

    /* AF5 for SPI1 on PA5/PA6/PA7 */
    GPIOA->AFR[0] &= ~(0xFFFU << 20);
    GPIOA->AFR[0] |=  (0x555U << 20);

    GPIOA->OSPEEDR |= (0x3FU << 10);

    /* PA4 as GPIO output for CS */
    GPIOA->MODER &= ~(3U << (4U * 2U));
    GPIOA->MODER |=  (1U << (4U * 2U));
    GPIOA->OTYPER &= ~(1U << 4U);
    GPIOA->OSPEEDR |= (3U << (4U * 2U));

    SD_CS_High();

    /* SPI1 master, software NSS, slow baud */
    SPI1->CR1 = 0;
    SPI1->CR1 |= (1U << 2);              /* MSTR */
    SPI1->CR1 |= (1U << 9) | (1U << 8);  /* SSM + SSI */
    SPI1->CR1 |= (7U << 3);              /* BR */
    SPI1->CR1 |= (1U << 6);              /* SPE */
}

uint8_t SPI1_Transfer(uint8_t data)
{
    while ((SPI1->SR & (1U << 1)) == 0U)
    {
    }

    *((volatile uint8_t *)&SPI1->DR) = data;

    while ((SPI1->SR & (1U << 0)) == 0U)
    {
    }

    return *((volatile uint8_t *)&SPI1->DR);
}

void SD_CS_High(void)
{
    GPIOA->ODR |= (1U << 4U);
}

void SD_CS_Low(void)
{
    GPIOA->ODR &= ~(1U << 4U);
}
