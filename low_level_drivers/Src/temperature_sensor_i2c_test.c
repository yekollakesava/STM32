/*
 * temperature_sensor_i2c_test.c
 *
 *  Created on: Mar 25, 2026
 *      Author: yekol
 */


#include "stm32l47xx.h"
#include <stdint.h>
#include <stdio.h>

#define SENSOR_ADDR   0x77

char msg[120];

/* UART */
void UART2_Init(void);
void UART2_SendChar(char ch);
void UART2_SendString(const char *str);

/* delay */
void delay_ms(uint32_t ms);

/* I2C */
void I2C1_GPIO_Init(void);
void I2C1_Init(void);
uint8_t I2C1_Write(uint8_t addr, uint8_t *data, uint8_t len);
uint8_t I2C1_Read(uint8_t addr, uint8_t *data, uint8_t len);
uint8_t I2C1_WriteRead(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len);

/* BMP/BME280 */
uint8_t  BMP280_Read8(uint8_t reg);
uint16_t BMP280_Read16_LE(uint8_t reg);
int16_t  BMP280_ReadS16_LE(uint8_t reg);
void     BMP280_Write8(uint8_t reg, uint8_t value);
uint8_t  BMP280_Init(void);
int32_t  BMP280_ReadRawTemp(void);
float    BMP280_ReadTemperature(void);

/* calibration */
static uint16_t dig_T1;
static int16_t  dig_T2, dig_T3;
static int32_t  t_fine;

/* -------------------------------------------------- */
int main(void)
{
    uint8_t id;
    float temp;
    int t_int, t_frac;

    UART2_Init();
    I2C1_GPIO_Init();
    I2C1_Init();

    UART2_SendString("\r\n0x77 Temperature Sensor Test Start\r\n");

    id = BMP280_Read8(0xD0);
    sprintf(msg, "Chip ID = 0x%02X\r\n", id);
    UART2_SendString(msg);

    if ((id != 0x58) && (id != 0x60))
    {
        UART2_SendString("0x77 device is not BMP280/BME280.\r\n");
        UART2_SendString("But I2C is working. Need exact sensor type.\r\n");

        while (1)
        {
            delay_ms(1000);
        }
    }

    if (!BMP280_Init())
    {
        UART2_SendString("BMP/BME280 init failed.\r\n");
        while (1)
        {
            delay_ms(1000);
        }
    }

    UART2_SendString("BMP/BME280 init success.\r\n");

    while (1)
    {
        temp = BMP280_ReadTemperature();

        t_int  = (int)temp;
        t_frac = (int)((temp - t_int) * 100.0f);
        if (t_frac < 0) t_frac = -t_frac;

        sprintf(msg, "Temperature = %d.%02d C\r\n", t_int, t_frac);
        UART2_SendString(msg);

        delay_ms(1000);
    }
}

/* -------------------------------------------------- */
/* UART2 : PA2 TX, PA3 RX */
/* -------------------------------------------------- */
void UART2_Init(void)
{
    RCC->AHB2ENR  |= (1U << 0);
    RCC->APB1ENR1 |= (1U << 17);

    GPIOA->MODER &= ~(0xFU << 4);
    GPIOA->MODER |=  (0xAU << 4);

    GPIOA->AFR[0] &= ~(0xFFU << 8);
    GPIOA->AFR[0] |=  (0x77U << 8);

    GPIOA->OSPEEDR |= (0xFU << 4);
    GPIOA->PUPDR   &= ~(0xFU << 4);
    GPIOA->PUPDR   |=  (0x5U << 4);

    USART2->CR1 = 0;
    USART2->BRR = 0x1A1;   /* 9600 baud @ 4 MHz */
    USART2->CR1 |= (1U << 3);
    USART2->CR1 |= (1U << 2);
    USART2->CR1 |= (1U << 0);
}

void UART2_SendChar(char ch)
{
    while (!(USART2->ISR & (1U << 7)));
    USART2->TDR = ch;
}

void UART2_SendString(const char *str)
{
    while (*str)
    {
        UART2_SendChar(*str++);
    }
}

/* -------------------------------------------------- */
/* Use existing delay.c implementation */
/* -------------------------------------------------- */
void delay_ms(uint32_t ms);

/* -------------------------------------------------- */
/* I2C1 : PB6 SCL, PB7 SDA */
/* -------------------------------------------------- */
void I2C1_GPIO_Init(void)
{
    RCC->AHB2ENR |= (1U << 1);

    GPIOB->MODER &= ~(0xFU << 12);
    GPIOB->MODER |=  (0xAU << 12);

    GPIOB->OTYPER |= (1U << 6) | (1U << 7);

    GPIOB->PUPDR &= ~(0xFU << 12);
    GPIOB->PUPDR |=  (0x5U << 12);

    GPIOB->OSPEEDR |= (0xFU << 12);

    GPIOB->AFR[0] &= ~(0xFFU << 24);
    GPIOB->AFR[0] |=  (0x44U << 24);
}

void I2C1_Init(void)
{
    RCC->APB1ENR1 |= (1U << 21);

    I2C1->CR1 &= ~(1U << 0);
    I2C1->TIMINGR = 0x00303D5B;   /* ~100 kHz */
    I2C1->CR1 |= (1U << 0);
}

/* write only */
uint8_t I2C1_Write(uint8_t addr, uint8_t *data, uint8_t len)
{
    uint8_t i;

    while (I2C1->ISR & (1U << 15));   /* BUSY */

    I2C1->ICR = 0xFFFFFFFF;
    I2C1->CR2 = 0;
    I2C1->CR2 |= ((uint32_t)addr << 1);
    I2C1->CR2 |= ((uint32_t)len << 16);
    I2C1->CR2 |= (1U << 25);          /* AUTOEND */
    I2C1->CR2 |= (1U << 13);          /* START */

    for (i = 0; i < len; i++)
    {
        while (!(I2C1->ISR & ((1U << 1) | (1U << 4))));  /* TXIS or NACK */
        if (I2C1->ISR & (1U << 4))
        {
            I2C1->ICR |= (1U << 4);
            return 0;
        }
        I2C1->TXDR = data[i];
    }

    while (!(I2C1->ISR & (1U << 5)));   /* STOPF */
    I2C1->ICR |= (1U << 5);             /* STOPCF */

    return 1;
}

/* read only */
uint8_t I2C1_Read(uint8_t addr, uint8_t *data, uint8_t len)
{
    uint8_t i;

    while (I2C1->ISR & (1U << 15));   /* BUSY */

    I2C1->ICR = 0xFFFFFFFF;
    I2C1->CR2 = 0;
    I2C1->CR2 |= ((uint32_t)addr << 1);
    I2C1->CR2 |= (1U << 10);          /* RD_WRN */
    I2C1->CR2 |= ((uint32_t)len << 16);
    I2C1->CR2 |= (1U << 25);          /* AUTOEND */
    I2C1->CR2 |= (1U << 13);          /* START */

    for (i = 0; i < len; i++)
    {
        while (!(I2C1->ISR & ((1U << 2) | (1U << 4))));  /* RXNE or NACK */
        if (I2C1->ISR & (1U << 4))
        {
            I2C1->ICR |= (1U << 4);
            return 0;
        }
        data[i] = (uint8_t)I2C1->RXDR;
    }

    while (!(I2C1->ISR & (1U << 5)));   /* STOPF */
    I2C1->ICR |= (1U << 5);             /* STOPCF */

    return 1;
}

/* register write then register read */
uint8_t I2C1_WriteRead(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len)
{
    if (!I2C1_Write(addr, &reg, 1))
        return 0;

    delay_ms(1);

    if (!I2C1_Read(addr, data, len))
        return 0;

    return 1;
}

/* -------------------------------------------------- */
/* BMP280/BME280 helpers */
/* -------------------------------------------------- */
uint8_t BMP280_Read8(uint8_t reg)
{
    uint8_t val = 0;
    I2C1_WriteRead(SENSOR_ADDR, reg, &val, 1);
    return val;
}

uint16_t BMP280_Read16_LE(uint8_t reg)
{
    uint8_t buf[2] = {0};
    I2C1_WriteRead(SENSOR_ADDR, reg, buf, 2);
    return (uint16_t)(buf[0] | (buf[1] << 8));
}

int16_t BMP280_ReadS16_LE(uint8_t reg)
{
    return (int16_t)BMP280_Read16_LE(reg);
}

void BMP280_Write8(uint8_t reg, uint8_t value)
{
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = value;
    I2C1_Write(SENSOR_ADDR, buf, 2);
}

uint8_t BMP280_Init(void)
{
    uint8_t id = BMP280_Read8(0xD0);

    if ((id != 0x58) && (id != 0x60))
        return 0;

    /* read temp calibration */
    dig_T1 = BMP280_Read16_LE(0x88);
    dig_T2 = BMP280_ReadS16_LE(0x8A);
    dig_T3 = BMP280_ReadS16_LE(0x8C);

    /* ctrl_meas: temp x1, pressure x1, normal mode */
    BMP280_Write8(0xF4, 0x27);

    /* config: standby/filter off basic test */
    BMP280_Write8(0xF5, 0x00);

    delay_ms(100);

    return 1;
}

int32_t BMP280_ReadRawTemp(void)
{
    uint8_t buf[3] = {0};
    int32_t adc_T;

    I2C1_WriteRead(SENSOR_ADDR, 0xFA, buf, 3);

    adc_T = ((int32_t)buf[0] << 12) |
            ((int32_t)buf[1] << 4)  |
            ((int32_t)buf[2] >> 4);

    return adc_T;
}

float BMP280_ReadTemperature(void)
{
    int32_t adc_T;
    int32_t var1, var2, T;

    adc_T = BMP280_ReadRawTemp();

    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) *
              ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) *
            ((int32_t)dig_T3)) >> 14;

    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;

    return T / 100.0f;
}
