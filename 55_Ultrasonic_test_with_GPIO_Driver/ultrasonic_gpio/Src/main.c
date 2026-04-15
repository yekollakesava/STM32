#include "gpio.h"
#include "ultrasonic.h"
#include "delay.h"

float distance;

int main()
{
    GPIO_Init();

    while(1)
    {
        distance = ultrasonic_distance();

        delay_ms(500);
    }
}
