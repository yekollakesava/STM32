/*
 * gpio_hal_driver.c
 *
 *  Created on: Mar 30, 2026
 *      Author: yekol
 */
#include "gpio_hal_driver.h"

void GPIO_Driver_Init(GPIO_Handle_t *pGPIOHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin   = pGPIOHandle->GPIO_Pin;
    GPIO_InitStruct.Mode  = pGPIOHandle->GPIO_Mode;
    GPIO_InitStruct.Pull  = pGPIOHandle->GPIO_Pull;
    GPIO_InitStruct.Speed = pGPIOHandle->GPIO_Speed;

    HAL_GPIO_Init(pGPIOHandle->GPIOx, &GPIO_InitStruct);
}

void GPIO_Driver_Write(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, PinState);
}

GPIO_PinState GPIO_Driver_Read(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    return HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
}

void GPIO_Driver_Toggle(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    HAL_GPIO_TogglePin(GPIOx, GPIO_Pin);
}

