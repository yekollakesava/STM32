/*
 * system_stm32l476xx.c
 *
 *  Created on: Mar 27, 2026
 *      Author: yekol
 */


#include "stm32l476xx.h"

void SystemInit(void)
{
    /* Optional: enable FPU */
    SCB->CPACR |= (0xF << 20);

    /* Optional: reset clock config (safe default) */
}
