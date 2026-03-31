#include "stm32l47xx.h"
#include "ff.h"
#include <string.h>
#include <stdint.h>

extern void SPI1_Init(void);

FATFS fs;
FIL fil;
FRESULT fres;
UINT bw;

static void UART2_Init(void);
static void UART2_SendChar(char c);
static void UART2_SendString(const char *str);
static void delay_ms(uint32_t ms);

static void UART2_Init(void)
{
    RCC->AHB2ENR |= (1U << 0);
    RCC->APB1ENR1 |= (1U << 17);

    GPIOA->MODER &= ~(0xFU << 4);
    GPIOA->MODER |=  (0xAU << 4);

    GPIOA->AFR[0] &= ~(0xFFU << 8);
    GPIOA->AFR[0] |=  (0x77U << 8);

    GPIOA->OSPEEDR |= (0xFU << 4);

    USART2->BRR = 0x1A1;
    USART2->CR1 |= (1U << 3);
    USART2->CR1 |= (1U << 0);
}

static void UART2_SendChar(char c)
{
    while (!(USART2->ISR & (1U << 7)));
    USART2->TDR = c;
}

static void UART2_SendString(const char *str)
{
    while (*str)
    {
        UART2_SendChar(*str++);
    }
}

static void delay_ms(uint32_t ms)
{
    volatile uint32_t i, j;
    for (i = 0; i < ms; i++)
    {
        for (j = 0; j < 4000; j++)
        {
            __asm__("nop");
        }
    }
}

int main(void)
{
    const char data[] = "Hello Kesava\r\nSTM32 SD working\r\n";

    UART2_Init();
    SPI1_Init();

    delay_ms(200);

    UART2_SendString("SD FILE CREATE TEST\r\n");

    fres = f_mount(&fs, "", 1);
    if (fres != FR_OK)
    {
        UART2_SendString("Mount FAILED\r\n");
        while (1);
    }
    UART2_SendString("Mount OK\r\n");

    fres = f_open(&fil, "test.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (fres != FR_OK)
    {
        UART2_SendString("File Create FAILED\r\n");
        while (1);
    }
    UART2_SendString("File Created\r\n");

    bw = 0;
    fres = f_write(&fil, data, strlen(data), &bw);
    if (fres != FR_OK)
    {
        UART2_SendString("Write FAILED\r\n");
        while (1);
    }

    if (bw != strlen(data))
    {
        UART2_SendString("Write size mismatch\r\n");
        while (1);
    }
    UART2_SendString("Write OK\r\n");

    fres = f_sync(&fil);
    if (fres != FR_OK)
    {
        UART2_SendString("Sync FAILED\r\n");
        while (1);
    }
    UART2_SendString("Sync OK\r\n");

    fres = f_close(&fil);
    if (fres != FR_OK)
    {
        UART2_SendString("Close FAILED\r\n");
        while (1);
    }
    UART2_SendString("File Closed\r\n");

    delay_ms(1000);

    while (1)
    {
    }
}
