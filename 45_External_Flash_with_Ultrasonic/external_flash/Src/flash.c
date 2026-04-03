#include "flash.h"
#include "spi.h"
#include "gpio.h"

#define WRITE_ENABLE 0x06
#define PAGE_PROGRAM 0x02

void flash_write_enable()
{
    GPIO_Write(4,0);

    SPI_Transfer(WRITE_ENABLE);

    GPIO_Write(4,1);
}

void flash_write(uint32_t addr,uint8_t data)
{
    flash_write_enable();

    GPIO_Write(4,0);

    SPI_Transfer(PAGE_PROGRAM);

    SPI_Transfer(addr>>16);
    SPI_Transfer(addr>>8);
    SPI_Transfer(addr);

    SPI_Transfer(data);

    GPIO_Write(4,1);
}
