#include "main.h"
#include "ultrasonic.h"
#include <stdint.h>

/* Pin selection
   TRIG = PA0
   ECHO = PA1
*/

#define TRIG_PORT   GPIOA
#define ECHO_PORT   GPIOA
#define TRIG_PIN    0
#define ECHO_PIN    1

static void delay_us_soft(uint32_t us)
{
    volatile uint32_t i;
    while(us--)
    {
        for(i = 0; i < 12; i++)
        {
            __asm("nop");
        }
    }
}

void Ultrasonic_Init(void)
{
    /* Enable GPIOA clock */
    RCC->AHB2ENR |= (1U << 0);

    /* PA0 = output (TRIG) */
    TRIG_PORT->MODER &= ~(3U << (TRIG_PIN * 2));
    TRIG_PORT->MODER |=  (1U << (TRIG_PIN * 2));

    /* PA1 = input (ECHO) */
    ECHO_PORT->MODER &= ~(3U << (ECHO_PIN * 2));

    /* Push-pull, no pull */
    TRIG_PORT->OTYPER &= ~(1U << TRIG_PIN);
    TRIG_PORT->PUPDR  &= ~(3U << (TRIG_PIN * 2));
    ECHO_PORT->PUPDR  &= ~(3U << (ECHO_PIN * 2));

    /* Start with TRIG low */
    TRIG_PORT->BSRR = (1U << (TRIG_PIN + 16));
}

float Ultrasonic_ReadDistance(void)
{
    uint32_t timeout;
    uint32_t pulse_width = 0;

    /* Ensure TRIG low */
    TRIG_PORT->BSRR = (1U << (TRIG_PIN + 16));
    delay_us_soft(2);

    /* 10us trigger pulse */
    TRIG_PORT->BSRR = (1U << TRIG_PIN);
    delay_us_soft(10);
    TRIG_PORT->BSRR = (1U << (TRIG_PIN + 16));

    /* Wait for ECHO to go high */
    timeout = 30000;
    while(((ECHO_PORT->IDR & (1U << ECHO_PIN)) == 0U) && timeout)
    {
        delay_us_soft(1);
        timeout--;
    }

    if(timeout == 0)
    {
        return -1.0f;   /* echo start timeout */
    }

    /* Measure high time */
    timeout = 30000;
    while((ECHO_PORT->IDR & (1U << ECHO_PIN)) && timeout)
    {
        delay_us_soft(1);
        pulse_width++;
        timeout--;
    }

    if(timeout == 0)
    {
        return -2.0f;   /* echo end timeout */
    }

    /* distance in cm */
    return (float)pulse_width / 58.0f;
}
