/*
 * i2c.c
 *
 *  Created on: Mar 25, 2026
 *      Author: yekol
 */


#include "i2c.h"
#include "stm32l47xx.h"

static void I2C1_WaitTXIS(void)
{
    while (!(I2C1->ISR & (1U << 1))) { }
}

static void I2C1_WaitRXNE(void)
{
    while (!(I2C1->ISR & (1U << 2))) { }
}

static void I2C1_WaitTC(void)
{
    while (!(I2C1->ISR & (1U << 6))) { }
}

static void I2C1_WaitSTOP(void)
{
    while (!(I2C1->ISR & (1U << 5))) { }
    I2C1->ICR |= (1U << 5);
}

void I2C1_GPIO_Init(void)
{
    RCC->AHB2ENR |= (1U << 1);   /* GPIOB clock */

    /* PB6 = SCL, PB7 = SDA */
    GPIOB->MODER &= ~(0xFU << 12);
    GPIOB->MODER |=  (0xAU << 12);

    GPIOB->AFR[0] &= ~(0xFFU << 24);
    GPIOB->AFR[0] |=  (0x44U << 24);   /* AF4 */

    GPIOB->OTYPER |= (1U << 6) | (1U << 7); /* open-drain */

    GPIOB->PUPDR &= ~(0xFU << 12);
    GPIOB->PUPDR |=  (0x5U << 12);     /* pull-up */
}

void I2C1_Init(void)
{
    RCC->APB1ENR1 |= (1U << 21);  /* I2C1 clock enable */

    I2C1->CR1 &= ~(1U << 0);      /* disable I2C */
    I2C1->TIMINGR = 0x00303D5B;   /* standard-mode timing */
    I2C1->CR1 |= 1U;              /* enable I2C */
}

uint8_t I2C_ReadReg(uint8_t devAddr, uint8_t regAddr)
{
    uint8_t data;

    /* Write register address */
    I2C1->CR2 = 0;
    I2C1->CR2 |= ((uint32_t)devAddr << 1);
    I2C1->CR2 |= (1U << 16);      /* NBYTES = 1 */
    I2C1->CR2 &= ~(1U << 10);     /* write */
    I2C1->CR2 |= (1U << 13);      /* START */

    I2C1_WaitTXIS();
    I2C1->TXDR = regAddr;

    I2C1_WaitTC();

    /* Repeated start for read */
    I2C1->CR2 = 0;
    I2C1->CR2 |= ((uint32_t)devAddr << 1);
    I2C1->CR2 |= (1U << 16);      /* NBYTES = 1 */
    I2C1->CR2 |= (1U << 10);      /* read */
    I2C1->CR2 |= (1U << 25);      /* AUTOEND */
    I2C1->CR2 |= (1U << 13);      /* START */

    I2C1_WaitRXNE();
    data = (uint8_t)I2C1->RXDR;

    I2C1_WaitSTOP();

    return data;
}

void I2C_WriteReg(uint8_t devAddr, uint8_t regAddr, uint8_t data)
{
    I2C1->CR2 = 0;
    I2C1->CR2 |= ((uint32_t)devAddr << 1);
    I2C1->CR2 |= (2U << 16);      /* NBYTES = 2 */
    I2C1->CR2 &= ~(1U << 10);     /* write */
    I2C1->CR2 |= (1U << 25);      /* AUTOEND */
    I2C1->CR2 |= (1U << 13);      /* START */

    I2C1_WaitTXIS();
    I2C1->TXDR = regAddr;

    I2C1_WaitTXIS();
    I2C1->TXDR = data;

    I2C1_WaitSTOP();
}
