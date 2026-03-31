#ifndef INC_STM32L47XX_SPI_DRIVER_H_
#define INC_STM32L47XX_SPI_DRIVER_H_

#include "stm32l47xx.h"
#include <stdint.h>

/*
 * SPI application states
 */
#define SPI_READY               0
#define SPI_BUSY_IN_RX          1
#define SPI_BUSY_IN_TX          2

/*
 * SPI possible application events
 */
#define SPI_EVENT_TX_CMPLT      1
#define SPI_EVENT_RX_CMPLT      2
#define SPI_EVENT_OVR_ERR       3

/*
 * SPI configuration structure
 */
typedef struct
{
    uint8_t SPI_DeviceMode;
    uint8_t SPI_BusConfig;
    uint8_t SPI_SclkSpeed;
    uint8_t SPI_DataSize;
    uint8_t SPI_CPOL;
    uint8_t SPI_CPHA;
    uint8_t SPI_SSM;
} SPI_Config_t;

/*
 * SPI handle structure
 */
typedef struct
{
    SPI_RegDef_t *pSPIx;
    SPI_Config_t SPIConfig;

    uint8_t  *pTxBuffer;
    uint8_t  *pRxBuffer;
    uint32_t TxLen;
    uint32_t RxLen;
    uint8_t  TxState;
    uint8_t  RxState;
} SPI_Handle_t;

/*
 * @SPI_DeviceMode
 */
#define SPI_DEVICE_MODE_SLAVE       0
#define SPI_DEVICE_MODE_MASTER      1

/*
 * @SPI_BusConfig
 */
#define SPI_BUS_CONFIG_FD           1
#define SPI_BUS_CONFIG_HD           2
#define SPI_BUS_CONFIG_SIMPLEX_RX   3

/*
 * @SPI_SclkSpeed
 */
#define SPI_SCLK_SPEED_DIV2         0
#define SPI_SCLK_SPEED_DIV4         1
#define SPI_SCLK_SPEED_DIV8         2
#define SPI_SCLK_SPEED_DIV16        3
#define SPI_SCLK_SPEED_DIV32        4
#define SPI_SCLK_SPEED_DIV64        5
#define SPI_SCLK_SPEED_DIV128       6
#define SPI_SCLK_SPEED_DIV256       7

/*
 * @SPI_DataSize (STM32L4 uses CR2.DS field)
 */
#define SPI_DATASIZE_4BIT           3U
#define SPI_DATASIZE_5BIT           4U
#define SPI_DATASIZE_6BIT           5U
#define SPI_DATASIZE_7BIT           6U
#define SPI_DATASIZE_8BIT           7U
#define SPI_DATASIZE_9BIT           8U
#define SPI_DATASIZE_10BIT          9U
#define SPI_DATASIZE_11BIT          10U
#define SPI_DATASIZE_12BIT          11U
#define SPI_DATASIZE_13BIT          12U
#define SPI_DATASIZE_14BIT          13U
#define SPI_DATASIZE_15BIT          14U
#define SPI_DATASIZE_16BIT          15U

/*
 * @SPI_CPOL
 */
#define SPI_CPOL_LOW                0
#define SPI_CPOL_HIGH               1

/*
 * @SPI_CPHA
 */
#define SPI_CPHA_LOW                0
#define SPI_CPHA_HIGH               1

/*
 * @SPI_SSM
 */
#define SPI_SSM_DI                  0
#define SPI_SSM_EN                  1

/*
 * SPI APIs
 */
void SPI_PeriClockControl(SPI_RegDef_t *pSPIx, uint8_t EnOrDi);
void SPI_Init(SPI_Handle_t *pSPIHandle);
void SPI_DeInit(SPI_RegDef_t *pSPIx);

void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint32_t Len);
void SPI_ReceiveData(SPI_RegDef_t *pSPIx, uint8_t *pRxBuffer, uint32_t Len);

uint8_t SPI_SendDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pTxBuffer, uint32_t Len);
uint8_t SPI_ReceiveDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pRxBuffer, uint32_t Len);

void SPI_PeripheralControl(SPI_RegDef_t *pSPIx, uint8_t EnOrDi);
void SPI_SSIConfig(SPI_RegDef_t *pSPIx, uint8_t EnOrDi);
void SPI_SSOEConfig(SPI_RegDef_t *pSPIx, uint8_t EnOrDi);
uint8_t SPI_GetFlagStatus(SPI_RegDef_t *pSPIx, uint32_t FlagName);

void SPI_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi);
void SPI_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority);
void SPI_IRQHandling(SPI_Handle_t *pHandle);

void SPI_ClearOVRFlag(SPI_RegDef_t *pSPIx);
void SPI_CloseTransmission(SPI_Handle_t *pSPIHandle);
void SPI_CloseReception(SPI_Handle_t *pSPIHandle);

/*
 * Application callback
 */
void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle, uint8_t AppEv);

#endif
