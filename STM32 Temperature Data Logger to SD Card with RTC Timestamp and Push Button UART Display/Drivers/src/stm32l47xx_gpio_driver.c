/*
 * stm32l47xx_gpio_driver.c
 *
 *  Created on: Mar 25, 2026
 *      Author: yekol
 */


#include "stm32l47xx_gpio_driver.h"

/*********************************************************************
 * @fn      		  - GPIO_PeriClockControl
 *
 * @brief             - This function enables or disables peripheral clock for GPIO port
 *
 * @param[in]         - base address of the GPIO peripheral
 * @param[in]         - ENABLE or DISABLE macros
 *
 * @return            - none
 *
 * @Note              - none
 */
void GPIO_PeriClockControl(GPIO_RegDef_t *pGPIOx, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
	{
		if(pGPIOx == GPIOA)
		{
			GPIOA_PCLK_EN();
		}
		else if(pGPIOx == GPIOB)
		{
			GPIOB_PCLK_EN();
		}
		else if(pGPIOx == GPIOC)
		{
			GPIOC_PCLK_EN();
		}
		else if(pGPIOx == GPIOD)
		{
			GPIOD_PCLK_EN();
		}
		else if(pGPIOx == GPIOE)
		{
			GPIOE_PCLK_EN();
		}
		else if(pGPIOx == GPIOF)
		{
			GPIOF_PCLK_EN();
		}
		else if(pGPIOx == GPIOG)
		{
			GPIOG_PCLK_EN();
		}
	}
	else
	{
		if(pGPIOx == GPIOA)
		{
			GPIOA_PCLK_DI();
		}
		else if(pGPIOx == GPIOB)
		{
			GPIOB_PCLK_DI();
		}
		else if(pGPIOx == GPIOC)
		{
			GPIOC_PCLK_DI();
		}
		else if(pGPIOx == GPIOD)
		{
			GPIOD_PCLK_DI();
		}
		else if(pGPIOx == GPIOE)
		{
			GPIOE_PCLK_DI();
		}
		else if(pGPIOx == GPIOF)
		{
			GPIOF_PCLK_DI();
		}
		else if(pGPIOx == GPIOG)
		{
			GPIOG_PCLK_DI();
		}
	}
}


/*********************************************************************
 * @fn      		  - GPIO_Init
 *
 * @brief             - This function initializes GPIO pin
 *
 * @param[in]         - pointer to GPIO handle structure
 *
 * @return            - none
 *
 * @Note              - none
 */
void GPIO_Init(GPIO_Handle_t *pGPIOHandle)
{
	uint32_t temp = 0;

	/*********************** 1. Configure the mode ***********************/
	if(pGPIOHandle->GPIO_PinConfig.GPIO_PinMode <= GPIO_MODE_ANALOG)
	{
		temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode <<
				(2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));

		pGPIOHandle->pGPIOx->MODER &= ~(0x3U <<
				(2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
		pGPIOHandle->pGPIOx->MODER |= temp;
	}
	else
	{
		/* Interrupt mode */

		/* Configure MODER as input for interrupt pin */
		pGPIOHandle->pGPIOx->MODER &= ~(0x3U <<
				(2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));

		/* Clear existing trigger settings */
		EXTI->FTSR1 &= ~(1U << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
		EXTI->RTSR1 &= ~(1U << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);

		if(pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_FT)
		{
			EXTI->FTSR1 |= (1U << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
		}
		else if(pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_RT)
		{
			EXTI->RTSR1 |= (1U << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
		}
		else if(pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_RFT)
		{
			EXTI->FTSR1 |= (1U << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
			EXTI->RTSR1 |= (1U << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
		}

		/* Configure the GPIO port selection in SYSCFG_EXTICR */
		uint8_t temp1 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber / 4;
		uint8_t temp2 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber % 4;
		uint8_t portcode = GPIO_BASEADDR_TO_CODE(pGPIOHandle->pGPIOx);

		SYSCFG_PCLK_EN();

		SYSCFG->EXTICR[temp1] &= ~(0xFU << (temp2 * 4));
		SYSCFG->EXTICR[temp1] |=  (portcode << (temp2 * 4));

		/* Unmask EXTI line */
		EXTI->IMR1 |= (1U << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
	}

	/*********************** 2. Configure the speed ***********************/
	temp = 0;
	temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinSpeed <<
			(2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));

	pGPIOHandle->pGPIOx->OSPEEDR &= ~(0x3U <<
			(2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
	pGPIOHandle->pGPIOx->OSPEEDR |= temp;


	/*********************** 3. Configure the pull-up/pull-down ***********************/
	temp = 0;
	temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinPuPdControl <<
			(2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));

	pGPIOHandle->pGPIOx->PUPDR &= ~(0x3U <<
			(2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
	pGPIOHandle->pGPIOx->PUPDR |= temp;


	/*********************** 4. Configure the output type ***********************/
	temp = 0;
	temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinOPType <<
			pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);

	pGPIOHandle->pGPIOx->OTYPER &= ~(0x1U <<
			pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
	pGPIOHandle->pGPIOx->OTYPER |= temp;


	/*********************** 5. Configure the alternate functionality ***********************/
	if(pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_ALTFN)
	{
		uint8_t temp1, temp2;

		temp1 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber / 8;
		temp2 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber % 8;

		pGPIOHandle->pGPIOx->AFR[temp1] &= ~(0xFU << (4 * temp2));
		pGPIOHandle->pGPIOx->AFR[temp1] |=
				(pGPIOHandle->GPIO_PinConfig.GPIO_PinAltFunMode << (4 * temp2));
	}
}


/*********************************************************************
 * @fn      		  - GPIO_DeInit
 *
 * @brief             - This function resets GPIO peripheral registers
 *
 * @param[in]         - base address of the GPIO peripheral
 *
 * @return            - none
 *
 * @Note              - none
 */
void GPIO_DeInit(GPIO_RegDef_t *pGPIOx)
{
	if(pGPIOx == GPIOA)
	{
		GPIOA_REG_RESET();
	}
	else if(pGPIOx == GPIOB)
	{
		GPIOB_REG_RESET();
	}
	else if(pGPIOx == GPIOC)
	{
		GPIOC_REG_RESET();
	}
	else if(pGPIOx == GPIOD)
	{
		GPIOD_REG_RESET();
	}
	else if(pGPIOx == GPIOE)
	{
		GPIOE_REG_RESET();
	}
	else if(pGPIOx == GPIOF)
	{
		GPIOF_REG_RESET();
	}
	else if(pGPIOx == GPIOG)
	{
		GPIOG_REG_RESET();
	}
}


/*********************************************************************
 * @fn      		  - GPIO_ReadFromInputPin
 *
 * @brief             - This function reads value from a GPIO input pin
 *
 * @param[in]         - base address of GPIO peripheral
 * @param[in]         - pin number
 *
 * @return            - 0 or 1
 *
 * @Note              - none
 */
uint8_t GPIO_ReadFromInputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber)
{
	uint8_t value;
	value = (uint8_t)((pGPIOx->IDR >> PinNumber) & 0x00000001U);
	return value;
}


/*********************************************************************
 * @fn      		  - GPIO_ReadFromInputPort
 *
 * @brief             - This function reads value from GPIO input port
 *
 * @param[in]         - base address of GPIO peripheral
 *
 * @return            - 16-bit port value
 *
 * @Note              - none
 */
uint16_t GPIO_ReadFromInputPort(GPIO_RegDef_t *pGPIOx)
{
	uint16_t value;
	value = (uint16_t)(pGPIOx->IDR);
	return value;
}


/*********************************************************************
 * @fn      		  - GPIO_WriteToOutputPin
 *
 * @brief             - This function writes value to GPIO output pin
 *
 * @param[in]         - base address of GPIO peripheral
 * @param[in]         - pin number
 * @param[in]         - GPIO_PIN_SET or GPIO_PIN_RESET
 *
 * @return            - none
 *
 * @Note              - none
 */
void GPIO_WriteToOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber, uint8_t Value)
{
	if(Value == GPIO_PIN_SET)
	{
		pGPIOx->ODR |= (1U << PinNumber);
	}
	else
	{
		pGPIOx->ODR &= ~(1U << PinNumber);
	}
}


/*********************************************************************
 * @fn      		  - GPIO_WriteToOutputPort
 *
 * @brief             - This function writes value to entire GPIO output port
 *
 * @param[in]         - base address of GPIO peripheral
 * @param[in]         - 16-bit value
 *
 * @return            - none
 *
 * @Note              - none
 */
void GPIO_WriteToOutputPort(GPIO_RegDef_t *pGPIOx, uint16_t Value)
{
	pGPIOx->ODR = Value;
}


/*********************************************************************
 * @fn      		  - GPIO_ToggleOutputPin
 *
 * @brief             - This function toggles GPIO output pin
 *
 * @param[in]         - base address of GPIO peripheral
 * @param[in]         - pin number
 *
 * @return            - none
 *
 * @Note              - none
 */
void GPIO_ToggleOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber)
{
	pGPIOx->ODR ^= (1U << PinNumber);
}


/*********************************************************************
 * @fn      		  - GPIO_IRQInterruptConfig
 *
 * @brief             - This function enables or disables IRQ number in NVIC
 *
 * @param[in]         - IRQ number
 * @param[in]         - ENABLE or DISABLE
 *
 * @return            - none
 *
 * @Note              - none
 */
void GPIO_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
	{
		if(IRQNumber <= 31)
		{
			*NVIC_ISER0 |= (1U << IRQNumber);
		}
		else if((IRQNumber > 31) && (IRQNumber < 64))
		{
			*NVIC_ISER1 |= (1U << (IRQNumber % 32));
		}
		else if((IRQNumber >= 64) && (IRQNumber < 96))
		{
			*NVIC_ISER2 |= (1U << (IRQNumber % 64));
		}
	}
	else
	{
		if(IRQNumber <= 31)
		{
			*NVIC_ICER0 |= (1U << IRQNumber);
		}
		else if((IRQNumber > 31) && (IRQNumber < 64))
		{
			*NVIC_ICER1 |= (1U << (IRQNumber % 32));
		}
		else if((IRQNumber >= 64) && (IRQNumber < 96))
		{
			*NVIC_ICER2 |= (1U << (IRQNumber % 64));
		}
	}
}


/*********************************************************************
 * @fn      		  - GPIO_IRQPriorityConfig
 *
 * @brief             - This function configures IRQ priority
 *
 * @param[in]         - IRQ number
 * @param[in]         - IRQ priority
 *
 * @return            - none
 *
 * @Note              - none
 */
void GPIO_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority)
{
	uint8_t iprx = IRQNumber / 4;
	uint8_t iprx_section = IRQNumber % 4;

	uint8_t shift_amount = (8 * iprx_section) + (8 - NO_PR_BITS_IMPLEMENTED);

	*(NVIC_PR_BASE_ADDR + iprx) &= ~(0xFFU << (8 * iprx_section));
	*(NVIC_PR_BASE_ADDR + iprx) |=  (IRQPriority << shift_amount);
}


/*********************************************************************
 * @fn      		  - GPIO_IRQHandling
 *
 * @brief             - This function clears EXTI pending bit
 *
 * @param[in]         - pin number
 *
 * @return            - none
 *
 * @Note              - Call this inside actual ISR
 */
void GPIO_IRQHandling(uint16_t PinNumber)
{
	if(EXTI->PR1 & (1U << PinNumber))
	{
		/* write 1 to clear pending bit */
		EXTI->PR1 |= (1U << PinNumber);
	}
}
