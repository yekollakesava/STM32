#include "stm32l47xx.h"
#include "stm32l47xx_gpio_driver.h"
#include <stdint.h>

void SD_CS_LOW(void)
{
    GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_4, GPIO_PIN_RESET);
}

void SD_CS_HIGH(void)
{
    GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_4, GPIO_PIN_SET);
}

uint8_t SPI1_TxRx_Byte(uint8_t data)
{
    while(!(SPI1->SR & (1U << 1)));   // TXE
    *((volatile uint8_t *)&SPI1->DR) = data;

    while(!(SPI1->SR & (1U << 0)));   // RXNE
    return *((volatile uint8_t *)&SPI1->DR);
}
