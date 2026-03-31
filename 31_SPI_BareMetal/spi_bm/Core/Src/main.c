#include "stm32l476xx.h"

void SPI1_Init(void);
uint8_t SPI1_Transfer(uint8_t data);
void delay(void);

volatile uint8_t received;

int main(void)
{
    SPI1_Init();

    while(1)
    {
        received = SPI1_Transfer(0x55);  // Send 0x55
        delay();
    }
}

void delay(void)
{
    for(volatile int i=0;i<300000;i++);
}

/* ---------------- SPI1 INIT ---------------- */
void SPI1_Init(void)
{
    /* Enable Clocks */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;   // GPIOA clock
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;    // SPI1 clock

    /* PA5=SCK, PA6=MISO, PA7=MOSI -> AF Mode */
    GPIOA->MODER &= ~(3<<(5*2));
    GPIOA->MODER |=  (2<<(5*2));

    GPIOA->MODER &= ~(3<<(6*2));
    GPIOA->MODER |=  (2<<(6*2));

    GPIOA->MODER &= ~(3<<(7*2));
    GPIOA->MODER |=  (2<<(7*2));

    /* AF5 for SPI1 */
    GPIOA->AFR[0] &= ~(0xF<<(5*4));
    GPIOA->AFR[0] |=  (5<<(5*4));

    GPIOA->AFR[0] &= ~(0xF<<(6*4));
    GPIOA->AFR[0] |=  (5<<(6*4));

    GPIOA->AFR[0] &= ~(0xF<<(7*4));
    GPIOA->AFR[0] |=  (5<<(7*4));

    /* Reset SPI */
    SPI1->CR1 = 0;
    SPI1->CR2 = 0;

    /* Master mode */
    SPI1->CR1 |= SPI_CR1_MSTR;

    /* Software NSS management */
    SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;

    /* Baud rate = fPCLK / 16 (safe slow speed) */
    SPI1->CR1 |= SPI_CR1_BR_1 | SPI_CR1_BR_0;

    /* CPOL=0, CPHA=0 (Mode 0) */

    /* 8-bit data size */
    SPI1->CR2 &= ~SPI_CR2_DS;
    SPI1->CR2 |= (7 << SPI_CR2_DS_Pos);

    /* Set RX threshold to 8-bit */
    SPI1->CR2 |= SPI_CR2_FRXTH;

    /* Enable SPI */
    SPI1->CR1 |= SPI_CR1_SPE;
}

/* ---------------- FULL DUPLEX TRANSFER ---------------- */
uint8_t SPI1_Transfer(uint8_t data)
{
    /* Wait until TX empty */
    while(!(SPI1->SR & SPI_SR_TXE));

    *((__IO uint8_t*)&SPI1->DR) = data;

    /* Wait until RX not empty */
    while(!(SPI1->SR & SPI_SR_RXNE));

    return *((__IO uint8_t*)&SPI1->DR);
}
