#include "stm32l47xx_gpio_driver.h"

void delay(void)
{
	for(uint32_t i = 0; i < 200000; i++);
}

int main(void)
{
	GPIO_Handle_t GpioLed, GpioBtn;

	/* LED : PA5 */
	GpioLed.pGPIOx = GPIOA;
	GpioLed.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_5;
	GpioLed.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	GpioLed.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GpioLed.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	GpioLed.GPIO_PinConfig.GPIO_PinAltFunMode = 0;

	GPIO_PeriClockControl(GPIOA, ENABLE);
	GPIO_Init(&GpioLed);

	/* Button : PC13 */
	GpioBtn.pGPIOx = GPIOC;
	GpioBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
	GpioBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
	GpioBtn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GpioBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	GpioBtn.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	GpioBtn.GPIO_PinConfig.GPIO_PinAltFunMode = 0;

	GPIO_PeriClockControl(GPIOC, ENABLE);
	GPIO_Init(&GpioBtn);

	while(1)
	{
		/* Button pressed = 0 for PC13 on Nucleo board */
		if(GPIO_ReadFromInputPin(GPIOC, GPIO_PIN_NO_13) == 0)
		{
			delay(); /* debounce */
			while(GPIO_ReadFromInputPin(GPIOC, GPIO_PIN_NO_13) == 0); /* wait for release */
			delay(); /* debounce */

			GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);
		}
	}
}
