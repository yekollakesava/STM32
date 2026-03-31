/*
 * 001_LED_Togglr.c
 *
 *  Created on: Mar 6, 2026
 *      Author: yekol
 */
/*
 * main.c
 *
 * Toggle LED using custom GPIO driver
 */

#include "stm32l47xx_gpio_driver.h"

void delay(void)
{
	for(uint32_t i = 0; i < 500000/2; i++);
}

int main(void)
{
	GPIO_Handle_t GpioLed;

	/* Select GPIO port */
	GpioLed.pGPIOx = GPIOA;

	/* Configure GPIO pin */
	GpioLed.GPIO_PinConfig.GPIO_PinNumber      = GPIO_PIN_NO_5;
	GpioLed.GPIO_PinConfig.GPIO_PinMode        = GPIO_MODE_OUT;
	GpioLed.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_SPEED_FAST;
	GpioLed.GPIO_PinConfig.GPIO_PinOPType      = GPIO_OP_TYPE_PP;
	GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	GpioLed.GPIO_PinConfig.GPIO_PinAltFunMode  = 0;

	/* Enable peripheral clock */
	GPIO_PeriClockControl(GPIOA, ENABLE);

	/* Initialize GPIO pin */
	GPIO_Init(&GpioLed);

	/* Toggle LED forever */
	while(1)
	{
		GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);
		delay();
	}
}
