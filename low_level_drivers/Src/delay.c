/*
 * delay.c
 *
 *  Created on: Mar 25, 2026
 *      Author: yekol
 */
#include "delay.h"

void delay_ms(uint32_t ms)
{
    volatile uint32_t i, j;
    for(i = 0; i < ms; i++)
    {
        for(j = 0; j < 4000; j++)
        {
            __asm__("nop");
        }
    }
}
