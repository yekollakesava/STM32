#include "ultrasonic.h"
#include "gpio.h"
#include "delay.h"

void ultrasonic_trigger(void)
{
    GPIO_WritePin(1, 0);
    delay_us(2);

    GPIO_WritePin(1, 1);
    delay_us(10);

    GPIO_WritePin(1, 0);
}

uint32_t ultrasonic_read(void)
{
    uint32_t t = 0;

    /* Wait until echo goes HIGH */
    while(GPIO_ReadPin(2) == 0);

    /* Measure HIGH pulse width */
    while(GPIO_ReadPin(2))
    {
        t++;
        delay_us(1);
    }

    return t;
}

uint8_t ultrasonic_distance(void)
{
    uint32_t t;

    ultrasonic_trigger();

    t = ultrasonic_read();

    /* Distance in cm */
    return (uint8_t)(t / 58);
}
