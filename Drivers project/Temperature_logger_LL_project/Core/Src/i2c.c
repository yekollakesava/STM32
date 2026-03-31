#include "stm32l476xx.h"
#include "i2c.h"

#define TIMEOUT 100000

void I2C1_Init(void)
{
    /* Enable I2C1 clock */
    RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN;

    /* Disable I2C before config */
    I2C1->CR1 &= ~I2C_CR1_PE;

    /* For 16 MHz system clock, 100 kHz */
    I2C1->TIMINGR = 0x00303D5B;

    /* Enable I2C */
    I2C1->CR1 |= I2C_CR1_PE;
}

static int WaitFlag(uint32_t flag)
{
    uint32_t timeout = TIMEOUT;
    while(!(I2C1->ISR & flag))
    {
        if(--timeout == 0)
            return 0;
    }
    return 1;
}

uint8_t I2C_ReadReg(uint8_t dev, uint8_t reg)
{
    uint8_t data = 0;

    while(I2C1->ISR & I2C_ISR_BUSY);

    /* Send register address */
    I2C1->CR2 = (dev<<1) | (1<<16) | I2C_CR2_START;

    if(!WaitFlag(I2C_ISR_TXIS)) return 0;
    I2C1->TXDR = reg;

    if(!WaitFlag(I2C_ISR_TC)) return 0;

    /* Read */
    I2C1->CR2 = (dev<<1) | (1<<16) |
                I2C_CR2_RD_WRN |
                I2C_CR2_START |
                I2C_CR2_AUTOEND;

    if(!WaitFlag(I2C_ISR_RXNE)) return 0;
    data = I2C1->RXDR;

    return data;
}

void I2C_WriteReg(uint8_t dev, uint8_t reg, uint8_t data)
{
    while(I2C1->ISR & I2C_ISR_BUSY);

    I2C1->CR2 = (dev<<1) |
                (2<<16) |
                I2C_CR2_START |
                I2C_CR2_AUTOEND;

    if(!WaitFlag(I2C_ISR_TXIS)) return;
    I2C1->TXDR = reg;

    if(!WaitFlag(I2C_ISR_TXIS)) return;
    I2C1->TXDR = data;

    WaitFlag(I2C_ISR_STOPF);
    I2C1->ICR |= I2C_ICR_STOPCF;
}
