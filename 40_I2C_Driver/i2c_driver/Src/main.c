#include "i2c_driver.h"

volatile uint8_t detected_devices[128];
volatile uint8_t device_count = 0;

int main(void)
{
    I2C1_Init();

    while(1)
    {
        device_count = 0;

        for(uint8_t addr = 1; addr < 128; addr++)
        {
            if(I2C1_CheckAddress(addr))
            {
                detected_devices[device_count++] = addr;
            }
        }

        for(volatile int i=0;i<1000000;i++);
    }
}
