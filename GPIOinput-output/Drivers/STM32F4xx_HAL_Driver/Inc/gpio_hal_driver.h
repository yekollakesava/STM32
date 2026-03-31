#ifndef GPIO_HAL_DRIVER_H
#define GPIO_HAL_DRIVER_H

#include "stm32f4xx_hal.h"

/* GPIO configuration structure */
typedef struct
{
    GPIO_TypeDef *GPIOx;
    uint16_t GPIO_Pin;
    uint32_t GPIO_Mode;
    uint32_t GPIO_Pull;
    uint32_t GPIO_Speed;
} GPIO_Handle_t;

/* Driver APIs */
void GPIO_Driver_Init(GPIO_Handle_t *pGPIOHandle);
void GPIO_Driver_Write(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
GPIO_PinState GPIO_Driver_Read(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void GPIO_Driver_Toggle(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

#endif
