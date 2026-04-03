#ifndef UART_DMA_H_
#define UART_DMA_H_

#include "stm32l4xx_hal.h"
#include <stdint.h>

#define UART_DMA_RX_BUFFER_SIZE   128

typedef struct
{
    UART_HandleTypeDef *huart;
    DMA_HandleTypeDef  *hdma_tx;
    DMA_HandleTypeDef  *hdma_rx;

    uint8_t rxBuffer[UART_DMA_RX_BUFFER_SIZE];
    volatile uint16_t rxLength;

    volatile uint8_t txBusy;
    volatile uint8_t rxReady;
} UART_DMA_Driver_t;

void UART_DMA_DriverInit(UART_DMA_Driver_t *driver,
                         UART_HandleTypeDef *huart,
                         DMA_HandleTypeDef *hdma_tx,
                         DMA_HandleTypeDef *hdma_rx);

HAL_StatusTypeDef UART_DMA_Transmit(UART_DMA_Driver_t *driver, uint8_t *data, uint16_t len);
HAL_StatusTypeDef UART_DMA_StartReceive(UART_DMA_Driver_t *driver);
void UART_DMA_IdleLineHandler(UART_DMA_Driver_t *driver);

uint8_t UART_DMA_IsTxBusy(UART_DMA_Driver_t *driver);
uint8_t UART_DMA_IsRxReady(UART_DMA_Driver_t *driver);
void UART_DMA_ClearRxReady(UART_DMA_Driver_t *driver);

void UART_DMA_TxCpltCallback(UART_DMA_Driver_t *driver);
void UART_DMA_RxEventCallback(UART_DMA_Driver_t *driver, uint16_t len);

#endif
