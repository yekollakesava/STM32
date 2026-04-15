#include "ultrasonic.h"
#include "gpio.h"
#include "delay.h"

void ultrasonic_trigger()
{
    GPIO_WritePin(1,0);
    delay_us(2);

    GPIO_WritePin(1,1);
    delay_us(10);

    GPIO_WritePin(1,0);
}

uint32_t ultrasonic_read()
{
    uint32_t time=0;

    while(GPIO_ReadPin(2)==0);

    while(GPIO_ReadPin(2))
    {
        time++;
        delay_us(1);
    }

    return time;
}

float ultrasonic_distance()
{
    uint32_t time;

    ultrasonic_trigger();

    time = ultrasonic_read();

    return (float)time/58.0;
}
