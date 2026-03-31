/*
 * usart_hal_driver.c
 *
 *  Created on: Mar 30, 2026
 *      Author: yekol
 */


#include "usart_hal_driver.h"

HAL_StatusTypeDef USART_Driver_Init(USART_Handle_t *husart)
{
    if (husart == NULL || husart->huart == NULL)
        return HAL_ERROR;

    return HAL_UART_Init(husart->huart);
}

HAL_StatusTypeDef USART_Driver_Transmit(USART_Handle_t *husart, uint8_t *pData, uint16_t size, uint32_t timeout)
{
    if (husart == NULL || husart->huart == NULL || pData == NULL)
        return HAL_ERROR;

    return HAL_UART_Transmit(husart->huart, pData, size, timeout);
}

HAL_StatusTypeDef USART_Driver_Receive(USART_Handle_t *husart, uint8_t *pData, uint16_t size, uint32_t timeout)
{
    if (husart == NULL || husart->huart == NULL || pData == NULL)
        return HAL_ERROR;

    return HAL_UART_Receive(husart->huart, pData, size, timeout);
}

HAL_StatusTypeDef USART_Driver_Transmit_IT(USART_Handle_t *husart, uint8_t *pData, uint16_t size)
{
    if (husart == NULL || husart->huart == NULL || pData == NULL)
        return HAL_ERROR;

    return HAL_UART_Transmit_IT(husart->huart, pData, size);
}

HAL_StatusTypeDef USART_Driver_Receive_IT(USART_Handle_t *husart, uint8_t *pData, uint16_t size)
{
    if (husart == NULL || husart->huart == NULL || pData == NULL)
        return HAL_ERROR;

    return HAL_UART_Receive_IT(husart->huart, pData, size);
}
