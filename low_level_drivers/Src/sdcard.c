#include "stm32l47xx.h"
#include "stm32l47xx_usart_driver.h"
#include "sdcard.h"
#include "spi.h"
#include <stdint.h>

extern USART_Handle_t USART2Handle;

static void UART2_SendString_Local(char *msg)
{
    while(*msg)
    {
        USART_SendData(&USART2Handle, (uint8_t *)msg, 1);
        msg++;
    }
}

static void UART2_SendChar_Local(char ch)
{
    USART_SendData(&USART2Handle, (uint8_t *)&ch, 1);
}

static void UART2_SendHex8_Local(uint8_t val)
{
    char hex[] = "0123456789ABCDEF";
    UART2_SendChar_Local(hex[(val >> 4) & 0x0F]);
    UART2_SendChar_Local(hex[val & 0x0F]);
}

/* SD CS on PA4 */
#define SD_CS_LOW()   (GPIOA->BSRR = (1U << (4 + 16)))
#define SD_CS_HIGH()  (GPIOA->BSRR = (1U << 4))

static void SD_CS_Init(void)
{
    RCC->AHB2ENR |= (1U << 0);   /* GPIOA clock */

    GPIOA->MODER &= ~(3U << (4 * 2));
    GPIOA->MODER |=  (1U << (4 * 2));   /* Output mode */

    GPIOA->OTYPER &= ~(1U << 4);
    GPIOA->OSPEEDR |= (3U << (4 * 2));

    SD_CS_HIGH();
}

static void SD_Select(void)
{
    SD_CS_LOW();
    SPI1_Transfer(0xFF);
}

static void SD_Deselect(void)
{
    SD_CS_HIGH();
    SPI1_Transfer(0xFF);
}

static uint8_t SD_WaitReady(void)
{
    uint32_t timeout = 50000;
    uint8_t resp;

    do
    {
        resp = SPI1_Transfer(0xFF);
        if(resp == 0xFF)
            return 1;
    } while(--timeout);

    return 0;
}

static uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg, uint8_t crc)
{
    uint8_t resp;
    uint8_t i;

    SPI1_Transfer(0xFF);

    SPI1_Transfer(cmd | 0x40);
    SPI1_Transfer((arg >> 24) & 0xFF);
    SPI1_Transfer((arg >> 16) & 0xFF);
    SPI1_Transfer((arg >> 8) & 0xFF);
    SPI1_Transfer(arg & 0xFF);
    SPI1_Transfer(crc);

    for(i = 0; i < 20; i++)
    {
        resp = SPI1_Transfer(0xFF);
        if((resp & 0x80) == 0)
            return resp;
    }

    return 0xFF;
}

uint8_t SD_Init(void)
{
    uint8_t r1;
    uint8_t i;
    uint8_t r7[4];
    uint32_t timeout;

    UART2_SendString_Local("SD_INIT: START\r\n");

    SPI1_Init();
    UART2_SendString_Local("SD_INIT: SPI1 OK\r\n");

    SD_CS_Init();
    UART2_SendString_Local("SD_INIT: CS GPIO OK\r\n");

    SD_CS_HIGH();
    for(i = 0; i < 10; i++)
    {
        SPI1_Transfer(0xFF);
    }
    UART2_SendString_Local("SD_INIT: DUMMY CLOCKS OK\r\n");

    SD_Select();
    UART2_SendString_Local("SD_INIT: CS LOW\r\n");

    r1 = SD_SendCommand(0, 0x00000000, 0x95);
    UART2_SendString_Local("SD_INIT: CMD0 R1 = 0x");
    UART2_SendHex8_Local(r1);
    UART2_SendString_Local("\r\n");

    if((r1 != 0x01) && (r1 != 0x00))
    {
        UART2_SendString_Local("SD_INIT: CMD0 FAILED\r\n");
        SD_Deselect();
        return 0;
    }

    r1 = SD_SendCommand(8, 0x000001AA, 0x87);
    UART2_SendString_Local("SD_INIT: CMD8 R1 = 0x");
    UART2_SendHex8_Local(r1);
    UART2_SendString_Local("\r\n");

    if(r1 != 0x01)
    {
        UART2_SendString_Local("SD_INIT: CMD8 FAILED\r\n");
        SD_Deselect();
        return 0;
    }

    for(i = 0; i < 4; i++)
        r7[i] = SPI1_Transfer(0xFF);

    UART2_SendString_Local("SD_INIT: CMD8 R7 = ");
    UART2_SendHex8_Local(r7[0]); UART2_SendChar_Local(' ');
    UART2_SendHex8_Local(r7[1]); UART2_SendChar_Local(' ');
    UART2_SendHex8_Local(r7[2]); UART2_SendChar_Local(' ');
    UART2_SendHex8_Local(r7[3]); UART2_SendString_Local("\r\n");

    timeout = 50000;
    do
    {
        SD_SendCommand(55, 0, 0x01);
        r1 = SD_SendCommand(41, 0x40000000, 0x01);
        timeout--;
    } while((r1 != 0x00) && timeout);

    UART2_SendString_Local("SD_INIT: ACMD41 R1 = 0x");
    UART2_SendHex8_Local(r1);
    UART2_SendString_Local("\r\n");

    if(timeout == 0)
    {
        UART2_SendString_Local("SD_INIT: ACMD41 TIMEOUT\r\n");
        SD_Deselect();
        return 0;
    }

    r1 = SD_SendCommand(58, 0, 0x01);
    UART2_SendString_Local("SD_INIT: CMD58 R1 = 0x");
    UART2_SendHex8_Local(r1);
    UART2_SendString_Local("\r\n");

    if(r1 != 0x00)
    {
        UART2_SendString_Local("SD_INIT: CMD58 FAILED\r\n");
        SD_Deselect();
        return 0;
    }

    for(i = 0; i < 4; i++)
        SPI1_Transfer(0xFF);

    SD_Deselect();

    UART2_SendString_Local("SD_INIT: SUCCESS\r\n");
    return 1;
}

uint8_t SD_ReadBlock(uint32_t block_addr, uint8_t *buffer)
{
    uint8_t token;
    uint8_t resp;
    uint16_t i;
    uint32_t timeout = 100000;

    SD_Select();

    resp = SD_SendCommand(17, block_addr, 0x01);
    if(resp != 0x00)
    {
        SD_Deselect();
        return 0;
    }

    do
    {
        token = SPI1_Transfer(0xFF);
    } while((token == 0xFF) && --timeout);

    if(token != 0xFE)
    {
        SD_Deselect();
        return 0;
    }

    for(i = 0; i < 512; i++)
    {
        buffer[i] = SPI1_Transfer(0xFF);
    }

    SPI1_Transfer(0xFF);   // CRC
    SPI1_Transfer(0xFF);

    SD_Deselect();
    return 1;
}
uint8_t SD_WriteBlock(uint32_t block_addr, const uint8_t *buffer)
{
    uint8_t resp;
    uint16_t i;
    uint32_t timeout = 100000;

    SD_Select();

    UART2_SendString_Local("SD_WRITE: CMD24 addr = 0x");
    UART2_SendHex8_Local((block_addr >> 24) & 0xFF);
    UART2_SendHex8_Local((block_addr >> 16) & 0xFF);
    UART2_SendHex8_Local((block_addr >> 8) & 0xFF);
    UART2_SendHex8_Local(block_addr & 0xFF);
    UART2_SendString_Local("\r\n");

    resp = SD_SendCommand(24, block_addr, 0x01);
    UART2_SendString_Local("SD_WRITE: CMD24 R1 = 0x");
    UART2_SendHex8_Local(resp);
    UART2_SendString_Local("\r\n");

    if(resp != 0x00)
    {
        SD_Deselect();
        return 0;
    }

    SPI1_Transfer(0xFF);
    SPI1_Transfer(0xFE);   // start token

    for(i = 0; i < 512; i++)
    {
        SPI1_Transfer(buffer[i]);
    }

    SPI1_Transfer(0xFF);   // dummy CRC
    SPI1_Transfer(0xFF);

    resp = SPI1_Transfer(0xFF);
    UART2_SendString_Local("SD_WRITE: DATA RESP = 0x");
    UART2_SendHex8_Local(resp);
    UART2_SendString_Local("\r\n");

    if((resp & 0x1F) != 0x05)
    {
        UART2_SendString_Local("SD_WRITE: DATA REJECTED\r\n");
        SD_Deselect();
        return 0;
    }

    while(timeout--)
    {
        resp = SPI1_Transfer(0xFF);
        if(resp == 0xFF)
            break;
    }

    if(timeout == 0)
    {
        UART2_SendString_Local("SD_WRITE: BUSY TIMEOUT\r\n");
        SD_Deselect();
        return 0;
    }

    UART2_SendString_Local("SD_WRITE: SUCCESS\r\n");
    SD_Deselect();
    return 1;
}
