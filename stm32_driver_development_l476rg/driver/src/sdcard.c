#include "sdcard.h"
#include "spi.h"
#include <stdint.h>

#define CMD0    0
#define CMD8    8
#define CMD16   16
#define CMD17   17
#define CMD24   24
#define CMD55   55
#define CMD41   41
#define CMD58   58

uint8_t SDHC_FLAG = 0;

static void SD_Deselect(void)
{
    SD_CS_HIGH();
    SPI1_Transfer(0xFF);
}

static void SD_Select(void)
{
    SD_CS_LOW();
    SPI1_Transfer(0xFF);
}

static uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg, uint8_t crc)
{
    uint8_t response;
    uint32_t timeout = 0xFFFF;

    SPI1_Transfer(0x40 | cmd);
    SPI1_Transfer((arg >> 24) & 0xFF);
    SPI1_Transfer((arg >> 16) & 0xFF);
    SPI1_Transfer((arg >> 8) & 0xFF);
    SPI1_Transfer(arg & 0xFF);
    SPI1_Transfer(crc);

    do
    {
        response = SPI1_Transfer(0xFF);
        timeout--;
    }
    while((response & 0x80) && timeout);

    return response;
}

uint8_t SD_Init(void)
{
    uint8_t response;
    uint8_t r7[4];
    uint8_t ocr[4];
    uint32_t timeout;

    SDHC_FLAG = 0;

    SD_Deselect();

    /* send 80 dummy clocks */
    for(int i = 0; i < 10; i++)
        SPI1_Transfer(0xFF);

    /* CMD0 */
    SD_Select();
    response = SD_SendCommand(CMD0, 0x00000000, 0x95);
    SD_Deselect();

    if(response != 0x01)
        return 1;

    /* CMD8 */
    SD_Select();
    response = SD_SendCommand(CMD8, 0x000001AA, 0x87);

    if(response == 0x01)
    {
        for(int i = 0; i < 4; i++)
            r7[i] = SPI1_Transfer(0xFF);

        SD_Deselect();

        if(r7[2] != 0x01 || r7[3] != 0xAA)
            return 2;

        timeout = 100000;
        do
        {
            SD_Select();
            response = SD_SendCommand(CMD55, 0, 0xFF);
            SD_Deselect();

            SD_Select();
            response = SD_SendCommand(CMD41, 0x40000000, 0xFF);
            SD_Deselect();

            timeout--;
        }
        while(response != 0x00 && timeout);

        if(timeout == 0)
            return 3;

        SD_Select();
        response = SD_SendCommand(CMD58, 0, 0xFF);

        if(response != 0x00)
        {
            SD_Deselect();
            return 4;
        }

        for(int i = 0; i < 4; i++)
            ocr[i] = SPI1_Transfer(0xFF);

        SD_Deselect();

        if(ocr[0] & 0x40)
            SDHC_FLAG = 1;
        else
            SDHC_FLAG = 0;
    }
    else
    {
        SD_Deselect();

        timeout = 100000;
        do
        {
            SD_Select();
            response = SD_SendCommand(CMD55, 0, 0xFF);
            SD_Deselect();

            SD_Select();
            response = SD_SendCommand(CMD41, 0, 0xFF);
            SD_Deselect();

            timeout--;
        }
        while(response != 0x00 && timeout);

        if(timeout == 0)
            return 5;

        SD_Select();
        response = SD_SendCommand(CMD16, 512, 0xFF);
        SD_Deselect();

        if(response != 0x00)
            return 6;

        SDHC_FLAG = 0;
    }

    return 0;
}

uint8_t SD_ReadBlock(uint32_t sector, uint8_t *buffer)
{
    uint8_t response;
    uint8_t token;
    uint32_t timeout;
    uint32_t address;

    address = (SDHC_FLAG) ? sector : (sector * 512UL);

    SD_Select();

    response = SD_SendCommand(CMD17, address, 0xFF);
    if(response != 0x00)
    {
        SD_Deselect();
        return 1;
    }

    timeout = 100000;
    do
    {
        token = SPI1_Transfer(0xFF);
        timeout--;
    }
    while(token != 0xFE && timeout);

    if(timeout == 0)
    {
        SD_Deselect();
        return 2;
    }

    for(int i = 0; i < 512; i++)
        buffer[i] = SPI1_Transfer(0xFF);

    SPI1_Transfer(0xFF);
    SPI1_Transfer(0xFF);

    SD_Deselect();
    return 0;
}

uint8_t SD_WriteBlock(uint32_t sector, uint8_t *buffer)
{
    uint8_t response;
    uint32_t timeout;
    uint32_t address;

    address = (SDHC_FLAG) ? sector : (sector * 512UL);

    SD_Select();

    response = SD_SendCommand(CMD24, address, 0xFF);
    if(response != 0x00)
    {
        SD_Deselect();
        return 1;
    }

    SPI1_Transfer(0xFF);
    SPI1_Transfer(0xFE);

    for(int i = 0; i < 512; i++)
        SPI1_Transfer(buffer[i]);

    SPI1_Transfer(0xFF);
    SPI1_Transfer(0xFF);

    response = SPI1_Transfer(0xFF);
    if((response & 0x1F) != 0x05)
    {
        SD_Deselect();
        return 2;
    }

    timeout = 100000;
    while((SPI1_Transfer(0xFF) == 0x00) && timeout)
        timeout--;

    if(timeout == 0)
    {
        SD_Deselect();
        return 3;
    }

    SD_Deselect();
    return 0;
}
