/*
 * button.c
 *
 *  Created on: Mar 27, 2026
 *      Author: yekol
 */


#include "stm32l476xx.h"
#include "button.h"

void Button_Init(void)
{
    RCC->AHB2ENR |= (1U << 2);   /* GPIOC clock enable */

    /* PC13 as input */
    GPIOC->MODER &= ~(3U << 26);

    /* Pull-up */
    GPIOC->PUPDR &= ~(3U << 26);
    GPIOC->PUPDR |=  (1U << 26);
}

int Button_Pressed(void)
{
    /* Active low button */
    if((GPIOC->IDR & (1U << 13)) == 0)
        return 1;
    else
        return 0;
}
