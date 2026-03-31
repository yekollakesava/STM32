#ifndef STM32L47XX_I2C_DRIVER_H_
#define STM32L47XX_I2C_DRIVER_H_

#include "stm32l47xx.h"
#include <stdint.h>

typedef enum
{
    I2C_READY = 0,
    I2C_BUSY_IN_RX,
    I2C_BUSY_IN_TX,
    I2C_ERROR,
    I2C_TIMEOUT,
    I2C_NACK_ERROR
} I2C_Status_t;

typedef struct
{
    I2C_RegDef_t *pI2Cx;
    uint32_t Timing;
    uint8_t OwnAddress1;

    uint8_t *pTxBuffer;
    uint8_t *pRxBuffer;
    uint32_t TxLen;
    uint32_t RxLen;
    uint32_t TxRxState;
    uint8_t DevAddr;
    uint32_t RxSize;
} I2C_Handle_t;

/* memory address size */
#define I2C_MEM_ADDR_8BIT         1U
#define I2C_MEM_ADDR_16BIT        2U

/* application states */
#define I2C_READY_STATE           0U
#define I2C_BUSY_IN_RX_STATE      1U
#define I2C_BUSY_IN_TX_STATE      2U

/* callback events */
#define I2C_EV_TX_CMPLT           1U
#define I2C_EV_RX_CMPLT           2U
#define I2C_ERROR_BERR            3U
#define I2C_ERROR_ARLO            4U
#define I2C_ERROR_OVR             5U
#define I2C_ERROR_NACKF           6U

/* APIs */
void I2C_PeriClockControl(I2C_RegDef_t *pI2Cx, uint8_t EnorDi);
void I2C_GPIO_Init(void);
void I2C_Init(I2C_Handle_t *pI2CHandle);

I2C_Status_t I2C_MasterSendData(I2C_RegDef_t *pI2Cx,
                                uint8_t *pTxBuffer,
                                uint32_t Len,
                                uint8_t SlaveAddr);

I2C_Status_t I2C_MasterReceiveData(I2C_RegDef_t *pI2Cx,
                                   uint8_t *pRxBuffer,
                                   uint32_t Len,
                                   uint8_t SlaveAddr);

I2C_Status_t I2C_Mem_Write(I2C_RegDef_t *pI2Cx,
                           uint8_t SlaveAddr,
                           uint16_t MemAddr,
                           uint8_t MemAddrSize,
                           uint8_t *pData,
                           uint32_t Len);

I2C_Status_t I2C_Mem_Read(I2C_RegDef_t *pI2Cx,
                          uint8_t SlaveAddr,
                          uint16_t MemAddr,
                          uint8_t MemAddrSize,
                          uint8_t *pData,
                          uint32_t Len);

/* interrupt APIs */
I2C_Status_t I2C_MasterSendDataIT(I2C_Handle_t *pI2CHandle,
                                  uint8_t *pTxBuffer,
                                  uint32_t Len,
                                  uint8_t SlaveAddr);

I2C_Status_t I2C_MasterReceiveDataIT(I2C_Handle_t *pI2CHandle,
                                     uint8_t *pRxBuffer,
                                     uint32_t Len,
                                     uint8_t SlaveAddr);

void I2C_CloseSendData(I2C_Handle_t *pI2CHandle);
void I2C_CloseReceiveData(I2C_Handle_t *pI2CHandle);

void I2C_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi);
void I2C_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority);

void I2C_EV_IRQHandling(I2C_Handle_t *pI2CHandle);
void I2C_ER_IRQHandling(I2C_Handle_t *pI2CHandle);

/* weak callback */
void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle, uint8_t AppEv);

#endif
