#include "sdcard.h"
#include "spi.h"

#define CMD0   0
#define CMD8   8
#define CMD55  55
#define CMD41  41
#define CMD58  58
#define CMD17  17
#define CMD24  24
#define CMD16  16

uint8_t SDHC_FLAG = 0;

/* ======================= */
static uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg, uint8_t crc)
{
    uint8_t response;
    uint32_t timeout = 0xFFFF;

    SPI1_Transfer(0x40 | cmd);
    SPI1_Transfer(arg >> 24);
    SPI1_Transfer(arg >> 16);
    SPI1_Transfer(arg >> 8);
    SPI1_Transfer(arg);
    SPI1_Transfer(crc);

    do {
        response = SPI1_Transfer(0xFF);
        timeout--;
    } while((response & 0x80) && timeout);

    return response;
}

/* ======================= */
uint8_t SD_Init(void)
{
    uint8_t response;
    uint32_t timeout;

    SD_CS_HIGH();

    /* Send 80 clocks */
    for(int i=0;i<10;i++)
        SPI1_Transfer(0xFF);

    SD_CS_LOW();

    SPI1_Transfer(0xFF);   // 🔥 REQUIRED dummy

    /* CMD0 */
    response = SD_SendCommand(CMD0, 0, 0x95);
    if(response != 0x01)
    {
        SD_CS_HIGH();
        SPI1_Transfer(0xFF);
        return 1;
    }

    /* CMD8 */
    response = SD_SendCommand(CMD8, 0x1AA, 0x87);
    if(response != 0x01)
    {
        SD_CS_HIGH();
        SPI1_Transfer(0xFF);
        return 2;
    }

    timeout = 0xFFFF;

    /* ACMD41 loop */
    do {
        SD_SendCommand(CMD55, 0, 0xFF);
        response = SD_SendCommand(CMD41, 0x40000000, 0xFF);
        timeout--;
    } while(response != 0x00 && timeout);

    if(timeout == 0)
    {
        SD_CS_HIGH();
        SPI1_Transfer(0xFF);
        return 3;
    }

    SD_CS_HIGH();
    SPI1_Transfer(0xFF);

    return 0;
}

/* ======================= */
uint8_t SD_ReadBlock(uint32_t sector, uint8_t *buffer)
{
    uint8_t response;
    uint8_t token;
    uint32_t timeout;

    SD_CS_LOW();

    response = SD_SendCommand(CMD17, sector, 0xFF);
    if(response != 0x00)
    {
        SD_CS_HIGH();
        return 1;
    }

    timeout = 0xFFFF;
    do {
        token = SPI1_Transfer(0xFF);
        timeout--;
    } while(token != 0xFE && timeout);

    if(timeout == 0)
    {
        SD_CS_HIGH();
        return 2;
    }

    for(int i=0;i<512;i++)
        buffer[i] = SPI1_Transfer(0xFF);

    SPI1_Transfer(0xFF);
    SPI1_Transfer(0xFF);

    SD_CS_HIGH();
    SPI1_Transfer(0xFF);

    return 0;
}

/* ======================= */
uint8_t SD_WriteBlock(uint32_t sector, uint8_t *buffer)
{
    uint8_t response;
    uint32_t timeout;

    SD_CS_LOW();

    response = SD_SendCommand(CMD24, sector, 0xFF);
    if(response != 0x00)
    {
        SD_CS_HIGH();
        return 1;
    }

    SPI1_Transfer(0xFE);

    for(int i=0;i<512;i++)
        SPI1_Transfer(buffer[i]);

    SPI1_Transfer(0xFF);
    SPI1_Transfer(0xFF);

    response = SPI1_Transfer(0xFF);
    if((response & 0x1F) != 0x05)
    {
        SD_CS_HIGH();
        return 2;
    }

    timeout = 0xFFFF;
    while(SPI1_Transfer(0xFF) == 0x00 && timeout)
        timeout--;

    SD_CS_HIGH();
    SPI1_Transfer(0xFF);

    return 0;
}
