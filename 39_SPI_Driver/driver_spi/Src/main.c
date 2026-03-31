#include <stdint.h>
#include "spi_driver.h"

volatile uint8_t received_data = 0;
volatile uint32_t sr_value = 0;

int main(void)
{
    SPI1_Init();

    while(1)
    {
        SPI1_Transmit(0x55);

        received_data = SPI1_Receive();

        sr_value = ((volatile uint32_t)0x40013008); // Read SR directly

        for(int i=0;i<500000;i++);
    }
}
