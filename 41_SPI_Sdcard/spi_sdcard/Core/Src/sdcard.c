#include "spi.h"
#include "sdcard.h"

#define CS_LOW()   (GPIOA->BSRR = (1<<4)<<16)
#define CS_HIGH()  (GPIOA->BSRR = (1<<4))

static uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg, uint8_t crc)
{
    uint8_t response;

    SPI1_Transfer(0x40 | cmd);
    SPI1_Transfer(arg >> 24);
    SPI1_Transfer(arg >> 16);
    SPI1_Transfer(arg >> 8);
    SPI1_Transfer(arg);
    SPI1_Transfer(crc);

    for(int i=0;i<10;i++)
    {
        response = SPI1_Transfer(0xFF);
        if(response != 0xFF)
            return response;
    }

    return 0xFF;
}

uint8_t SD_Init(void)
{
    CS_HIGH();

    for(int i=0;i<10;i++)
        SPI1_Transfer(0xFF);

    CS_LOW();

    // CMD0
    if(SD_SendCommand(0,0,0x95) != 0x01)
        return 1;

    CS_HIGH();
    SPI1_Transfer(0xFF);

    // CMD8
    CS_LOW();
    if(SD_SendCommand(8,0x1AA,0x87) != 0x01)
        return 2;

    for(int i=0;i<4;i++)
        SPI1_Transfer(0xFF);

    CS_HIGH();
    SPI1_Transfer(0xFF);

    // ACMD41
    do{
        CS_LOW();
        SD_SendCommand(55,0,0x01);
        CS_HIGH();
        SPI1_Transfer(0xFF);

        CS_LOW();
    }while(SD_SendCommand(41,0x40000000,0x01) != 0x00);

    CS_HIGH();
    SPI1_Transfer(0xFF);

    SPI1_SetSpeedHigh();

    return 0;
}

uint8_t SD_WriteBlock(uint32_t block, uint8_t *data)
{
    CS_LOW();

    if(SD_SendCommand(24, block, 0xFF) != 0x00)
    {
        CS_HIGH();
        return 1;
    }

    SPI1_Transfer(0xFE);

    for(int i=0;i<512;i++)
        SPI1_Transfer(data[i]);

    SPI1_Transfer(0xFF);
    SPI1_Transfer(0xFF);

    uint8_t resp = SPI1_Transfer(0xFF);
    if((resp & 0x1F) != 0x05)
    {
        CS_HIGH();
        return 2;
    }

    while(SPI1_Transfer(0xFF) == 0);

    CS_HIGH();
    SPI1_Transfer(0xFF);

    return 0;
}

uint8_t SD_ReadBlock(uint32_t block, uint8_t *data)
{
    CS_LOW();

    if(SD_SendCommand(17, block, 0xFF) != 0x00)
    {
        CS_HIGH();
        return 1;
    }

    while(SPI1_Transfer(0xFF) != 0xFE);

    for(int i=0;i<512;i++)
        data[i] = SPI1_Transfer(0xFF);

    SPI1_Transfer(0xFF);
    SPI1_Transfer(0xFF);

    CS_HIGH();
    SPI1_Transfer(0xFF);

    return 0;
}
