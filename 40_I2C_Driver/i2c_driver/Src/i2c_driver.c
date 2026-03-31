#include "i2c_driver.h"

/* Base Addresses */
#define RCC_BASE        0x40023800
#define GPIOB_BASE      0x40020400
#define I2C1_BASE       0x40005400

/* RCC */
#define RCC_AHB1ENR     (*(volatile uint32_t*)(RCC_BASE + 0x30))
#define RCC_APB1ENR     (*(volatile uint32_t*)(RCC_BASE + 0x40))

/* GPIOB */
#define GPIOB_MODER     (*(volatile uint32_t*)(GPIOB_BASE + 0x00))
#define GPIOB_OTYPER    (*(volatile uint32_t*)(GPIOB_BASE + 0x04))
#define GPIOB_PUPDR     (*(volatile uint32_t*)(GPIOB_BASE + 0x0C))
#define GPIOB_AFRL      (*(volatile uint32_t*)(GPIOB_BASE + 0x20))

/* I2C1 */
#define I2C1_CR1        (*(volatile uint32_t*)(I2C1_BASE + 0x00))
#define I2C1_CR2        (*(volatile uint32_t*)(I2C1_BASE + 0x04))
#define I2C1_SR1        (*(volatile uint32_t*)(I2C1_BASE + 0x14))
#define I2C1_SR2        (*(volatile uint32_t*)(I2C1_BASE + 0x18))
#define I2C1_DR         (*(volatile uint32_t*)(I2C1_BASE + 0x10))
#define I2C1_CCR        (*(volatile uint32_t*)(I2C1_BASE + 0x1C))
#define I2C1_TRISE      (*(volatile uint32_t*)(I2C1_BASE + 0x20))

void I2C1_Init(void)
{
    RCC_AHB1ENR |= (1 << 1);
    RCC_APB1ENR |= (1 << 21);

    GPIOB_MODER &= ~(0xF << 12);
    GPIOB_MODER |=  (0xA << 12);

    GPIOB_OTYPER |= (1 << 6) | (1 << 7);

    GPIOB_PUPDR &= ~(0xF << 12);
    GPIOB_PUPDR |=  (0x5 << 12);

    GPIOB_AFRL &= ~(0xFF << 24);
    GPIOB_AFRL |=  (0x44 << 24);

    I2C1_CR1 |= (1 << 15);
    I2C1_CR1 &= ~(1 << 15);

    I2C1_CR2 = 16;
    I2C1_CCR = 80;
    I2C1_TRISE = 17;

    I2C1_CR1 |= (1 << 0);
}

uint8_t I2C1_CheckAddress(uint8_t addr)
{
    uint8_t found = 0;

    I2C1_CR1 |= (1 << 8);                   // START
    while(!(I2C1_SR1 & (1 << 0)));          // SB

    I2C1_DR = addr << 1;                   // Send address (write)

    while(!(I2C1_SR1 & ((1 << 1) | (1 << 10))));
    // Wait for ADDR or AF

    if(I2C1_SR1 & (1 << 1))                // ADDR set → ACK received
    {
        (void)I2C1_SR2;                    // Clear ADDR
        found = 1;
    }

    if(I2C1_SR1 & (1 << 10))               // AF → no ACK
    {
        I2C1_SR1 &= ~(1 << 10);            // Clear AF
    }

    I2C1_CR1 |= (1 << 9);                  // STOP

    return found;
}
