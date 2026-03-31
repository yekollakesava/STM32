#ifndef USART_HAL_DRIVER_H
#define USART_HAL_DRIVER_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

typedef struct
{
    UART_HandleTypeDef *huart;
} USART_Handle_t;

/* APIs */
HAL_StatusTypeDef USART_Driver_Init(USART_Handle_t *husart);
HAL_StatusTypeDef USART_Driver_Transmit(USART_Handle_t *husart, uint8_t *pData, uint16_t size, uint32_t timeout);
HAL_StatusTypeDef USART_Driver_Receive(USART_Handle_t *husart, uint8_t *pData, uint16_t size, uint32_t timeout);
HAL_StatusTypeDef USART_Driver_Transmit_IT(USART_Handle_t *husart, uint8_t *pData, uint16_t size);
HAL_StatusTypeDef USART_Driver_Receive_IT(USART_Handle_t *husart, uint8_t *pData, uint16_t size);

#endif
