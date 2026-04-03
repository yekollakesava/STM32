/*
 * uart_dma.c
 *
 *  Created on: Apr 1, 2026
 *      Author: yekol
 */


#include "uart_dma.h"
#include <string.h>

void UART_DMA_DriverInit(UART_DMA_Driver_t *driver,
                         UART_HandleTypeDef *huart,
                         DMA_HandleTypeDef *hdma_tx,
                         DMA_HandleTypeDef *hdma_rx)
{
    driver->huart    = huart;
    driver->hdma_tx  = hdma_tx;
    driver->hdma_rx  = hdma_rx;
    driver->rxLength = 0;
    driver->txBusy   = 0;
    driver->rxReady  = 0;

    memset(driver->rxBuffer, 0, sizeof(driver->rxBuffer));

    /* Enable UART IDLE interrupt */
    __HAL_UART_ENABLE_IT(driver->huart, UART_IT_IDLE);
}

HAL_StatusTypeDef UART_DMA_Transmit(UART_DMA_Driver_t *driver, uint8_t *data, uint16_t len)
{
    if (driver->txBusy)
    {
        return HAL_BUSY;
    }

    driver->txBusy = 1;
    return HAL_UART_Transmit_DMA(driver->huart, data, len);
}

HAL_StatusTypeDef UART_DMA_StartReceive(UART_DMA_Driver_t *driver)
{
    driver->rxReady  = 0;
    driver->rxLength = 0;
    memset(driver->rxBuffer, 0, sizeof(driver->rxBuffer));

    return HAL_UART_Receive_DMA(driver->huart, driver->rxBuffer, UART_DMA_RX_BUFFER_SIZE);
}

void UART_DMA_IdleLineHandler(UART_DMA_Driver_t *driver)
{
    if (__HAL_UART_GET_FLAG(driver->huart, UART_FLAG_IDLE) != RESET)
    {
        __HAL_UART_CLEAR_IDLEFLAG(driver->huart);

        /* Stop DMA so we can know how many bytes are received */
        HAL_UART_DMAStop(driver->huart);

        driver->rxLength = UART_DMA_RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(driver->hdma_rx);
        driver->rxReady  = 1;

        UART_DMA_RxEventCallback(driver, driver->rxLength);
    }
}

uint8_t UART_DMA_IsTxBusy(UART_DMA_Driver_t *driver)
{
    return driver->txBusy;
}

uint8_t UART_DMA_IsRxReady(UART_DMA_Driver_t *driver)
{
    return driver->rxReady;
}

void UART_DMA_ClearRxReady(UART_DMA_Driver_t *driver)
{
    driver->rxReady = 0;
}

/* Weak callback style functions - can be edited if needed */
__weak void UART_DMA_TxCpltCallback(UART_DMA_Driver_t *driver)
{
    (void)driver;
}

__weak void UART_DMA_RxEventCallback(UART_DMA_Driver_t *driver, uint16_t len)
{
    (void)driver;
    (void)len;
}
