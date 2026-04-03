#include "delay.h"

void delay_us(uint32_t us)
{
    volatile int i;
    while(us--)
    {
        for(i=0;i<16;i++);
    }
}

void delay_ms(uint32_t ms)
{
    while(ms--)
    delay_us(1000);
}
