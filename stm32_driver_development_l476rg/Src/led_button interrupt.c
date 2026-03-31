/*
 * led_button interrupt.c
 *
 *  Created on: Mar 9, 2026
 *      Author: yekol
 */
/* main.c */
#include "stm32l47xx_gpio_driver.h"

int main(void)
{
    GPIO_Handle_t led, btn;

    led.pGPIOx = GPIOA;
    led.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_5;
    led.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
    led.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    led.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    led.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    GPIO_PeriClockControl(GPIOA, ENABLE);
    GPIO_Init(&led);

    btn.pGPIOx = GPIOC;
    btn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
    btn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IT_FT;
    btn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    btn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
    GPIO_PeriClockControl(GPIOC, ENABLE);
    GPIO_Init(&btn);

    GPIO_IRQPriorityConfig(IRQ_NO_EXTI15_10, NVIC_IRQ_PRI15);
    GPIO_IRQInterruptConfig(IRQ_NO_EXTI15_10, ENABLE);

    while(1);
}

void EXTI15_10_IRQHandler(void)
{
    GPIO_IRQHandling(GPIO_PIN_NO_13);
    GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);
}

