#include "gpio.h"
#include "spi.h"
#include "flash.h"
#include "ultrasonic.h"
#include "delay.h"

uint8_t distance;
uint32_t addr = 0;

int main(void)
{
    GPIO_Init();        // Initialize GPIO pins
    SPI_Init();         // Initialize SPI for Flash
    ultrasonic_init();  // Initialize ultrasonic pins
    flash_init();       // Optional flash setup

    while(1)
    {
        distance = ultrasonic_distance();   // Measure distance

        flash_write(addr, distance);        // Store in flash

        addr++;                             // Next address

        if(addr > 0x0FFF)                   // Prevent overflow
            addr = 0;

        delay_ms(1000);
    }
}
