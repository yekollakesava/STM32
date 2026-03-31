#include "stm32l47xx_i2c_driver.h"

/* CR1 bits */
#define I2C_CR1_PE                0
#define I2C_CR1_TXIE              1
#define I2C_CR1_RXIE              2
#define I2C_CR1_ADDRIE            3
#define I2C_CR1_NACKIE            4
#define I2C_CR1_STOPIE            5
#define I2C_CR1_TCIE              6
#define I2C_CR1_ERRIE             7

/* CR2 bits */
#define I2C_CR2_RD_WRN            10
#define I2C_CR2_START             13
#define I2C_CR2_STOP              14
#define I2C_CR2_NBYTES            16
#define I2C_CR2_RELOAD            24
#define I2C_CR2_AUTOEND           25

/* ISR bits */
#define I2C_ISR_TXIS              1
#define I2C_ISR_RXNE              2
#define I2C_ISR_ADDR              3
#define I2C_ISR_NACKF             4
#define I2C_ISR_STOPF             5
#define I2C_ISR_TC                6
#define I2C_ISR_TCR               7
#define I2C_ISR_BERR              8
#define I2C_ISR_ARLO              9
#define I2C_ISR_OVR               10
#define I2C_ISR_BUSY              15

/* ICR bits */
#define I2C_ICR_ADDRCF            3
#define I2C_ICR_NACKCF            4
#define I2C_ICR_STOPCF            5
#define I2C_ICR_BERRCF            8
#define I2C_ICR_ARLOCF            9
#define I2C_ICR_OVRCF             10

static void I2C_ClearStopFlag(I2C_RegDef_t *pI2Cx)
{
    pI2Cx->ICR |= (1U << I2C_ICR_STOPCF);
}

static void I2C_ClearNackFlag(I2C_RegDef_t *pI2Cx)
{
    pI2Cx->ICR |= (1U << I2C_ICR_NACKCF);
}

static I2C_Status_t I2C_WaitUntilBusFree(I2C_RegDef_t *pI2Cx)
{
    uint32_t timeout = 1000000U;

    while(pI2Cx->ISR & (1U << I2C_ISR_BUSY))
    {
        if(timeout-- == 0U)
        {
            return I2C_TIMEOUT;
        }
    }
    return I2C_READY;
}

static I2C_Status_t I2C_WaitForTXIS(I2C_RegDef_t *pI2Cx)
{
    uint32_t timeout = 1000000U;

    while(!(pI2Cx->ISR & (1U << I2C_ISR_TXIS)))
    {
        if(pI2Cx->ISR & (1U << I2C_ISR_NACKF))
        {
            I2C_ClearNackFlag(pI2Cx);
            return I2C_NACK_ERROR;
        }

        if(timeout-- == 0U)
        {
            return I2C_TIMEOUT;
        }
    }
    return I2C_READY;
}

static I2C_Status_t I2C_WaitForRXNE(I2C_RegDef_t *pI2Cx)
{
    uint32_t timeout = 1000000U;

    while(!(pI2Cx->ISR & (1U << I2C_ISR_RXNE)))
    {
        if(pI2Cx->ISR & (1U << I2C_ISR_NACKF))
        {
            I2C_ClearNackFlag(pI2Cx);
            return I2C_NACK_ERROR;
        }

        if(timeout-- == 0U)
        {
            return I2C_TIMEOUT;
        }
    }
    return I2C_READY;
}

static I2C_Status_t I2C_WaitForTC(I2C_RegDef_t *pI2Cx)
{
    uint32_t timeout = 1000000U;

    while(!(pI2Cx->ISR & (1U << I2C_ISR_TC)))
    {
        if(pI2Cx->ISR & (1U << I2C_ISR_NACKF))
        {
            I2C_ClearNackFlag(pI2Cx);
            return I2C_NACK_ERROR;
        }

        if(timeout-- == 0U)
        {
            return I2C_TIMEOUT;
        }
    }
    return I2C_READY;
}

static I2C_Status_t I2C_WaitForSTOP(I2C_RegDef_t *pI2Cx)
{
    uint32_t timeout = 1000000U;

    while(!(pI2Cx->ISR & (1U << I2C_ISR_STOPF)))
    {
        if(pI2Cx->ISR & (1U << I2C_ISR_NACKF))
        {
            I2C_ClearNackFlag(pI2Cx);
            return I2C_NACK_ERROR;
        }

        if(timeout-- == 0U)
        {
            return I2C_TIMEOUT;
        }
    }

    I2C_ClearStopFlag(pI2Cx);
    return I2C_READY;
}

void I2C_PeriClockControl(I2C_RegDef_t *pI2Cx, uint8_t EnorDi)
{
    if(EnorDi == ENABLE)
    {
        if(pI2Cx == I2C1)      I2C1_PCLK_EN();
        else if(pI2Cx == I2C2) I2C2_PCLK_EN();
        else if(pI2Cx == I2C3) I2C3_PCLK_EN();
    }
    else
    {
        if(pI2Cx == I2C1)      I2C1_PCLK_DI();
        else if(pI2Cx == I2C2) I2C2_PCLK_DI();
        else if(pI2Cx == I2C3) I2C3_PCLK_DI();
    }
}

void I2C_GPIO_Init(void)
{
    GPIOB_PCLK_EN();

    GPIOB->MODER &= ~((3U << (8 * 2)) | (3U << (9 * 2)));
    GPIOB->MODER |=  ((2U << (8 * 2)) | (2U << (9 * 2)));

    GPIOB->OTYPER |= (1U << 8);
    GPIOB->OTYPER |= (1U << 9);

    GPIOB->PUPDR &= ~((3U << (8 * 2)) | (3U << (9 * 2)));
    GPIOB->PUPDR |=  ((1U << (8 * 2)) | (1U << (9 * 2)));

    GPIOB->OSPEEDR &= ~((3U << (8 * 2)) | (3U << (9 * 2)));
    GPIOB->OSPEEDR |=  ((3U << (8 * 2)) | (3U << (9 * 2)));

    GPIOB->AFR[1] &= ~((0xFU << 0) | (0xFU << 4));
    GPIOB->AFR[1] |=  ((4U << 0) | (4U << 4));
}

void I2C_Init(I2C_Handle_t *pI2CHandle)
{
    I2C_PeriClockControl(pI2CHandle->pI2Cx, ENABLE);

    pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_PE);

    pI2CHandle->pI2Cx->TIMINGR = pI2CHandle->Timing;

    pI2CHandle->pI2Cx->OAR1 = 0U;
    if(pI2CHandle->OwnAddress1 != 0U)
    {
        pI2CHandle->pI2Cx->OAR1 = (1U << 15) | (pI2CHandle->OwnAddress1 << 1);
    }

    pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_PE);

    pI2CHandle->TxRxState = I2C_READY_STATE;
    pI2CHandle->pTxBuffer = 0;
    pI2CHandle->pRxBuffer = 0;
    pI2CHandle->TxLen = 0;
    pI2CHandle->RxLen = 0;
    pI2CHandle->RxSize = 0;
}

I2C_Status_t I2C_MasterSendData(I2C_RegDef_t *pI2Cx,
                                uint8_t *pTxBuffer,
                                uint32_t Len,
                                uint8_t SlaveAddr)
{
    uint32_t i;
    I2C_Status_t status;

    if(Len == 0U) return I2C_ERROR;

    status = I2C_WaitUntilBusFree(pI2Cx);
    if(status != I2C_READY) return status;

    pI2Cx->CR2 = 0U;
    pI2Cx->CR2 |= ((uint32_t)(SlaveAddr << 1));
    pI2Cx->CR2 |= (Len << I2C_CR2_NBYTES);
    pI2Cx->CR2 |= (1U << I2C_CR2_AUTOEND);
    pI2Cx->CR2 |= (1U << I2C_CR2_START);

    for(i = 0; i < Len; i++)
    {
        status = I2C_WaitForTXIS(pI2Cx);
        if(status != I2C_READY) return status;
        pI2Cx->TXDR = pTxBuffer[i];
    }

    return I2C_WaitForSTOP(pI2Cx);
}

I2C_Status_t I2C_MasterReceiveData(I2C_RegDef_t *pI2Cx,
                                   uint8_t *pRxBuffer,
                                   uint32_t Len,
                                   uint8_t SlaveAddr)
{
    uint32_t i;
    I2C_Status_t status;

    if(Len == 0U) return I2C_ERROR;

    status = I2C_WaitUntilBusFree(pI2Cx);
    if(status != I2C_READY) return status;

    pI2Cx->CR2 = 0U;
    pI2Cx->CR2 |= ((uint32_t)(SlaveAddr << 1));
    pI2Cx->CR2 |= (1U << I2C_CR2_RD_WRN);
    pI2Cx->CR2 |= (Len << I2C_CR2_NBYTES);
    pI2Cx->CR2 |= (1U << I2C_CR2_AUTOEND);
    pI2Cx->CR2 |= (1U << I2C_CR2_START);

    for(i = 0; i < Len; i++)
    {
        status = I2C_WaitForRXNE(pI2Cx);
        if(status != I2C_READY) return status;
        pRxBuffer[i] = (uint8_t)pI2Cx->RXDR;
    }

    return I2C_WaitForSTOP(pI2Cx);
}

I2C_Status_t I2C_Mem_Write(I2C_RegDef_t *pI2Cx,
                           uint8_t SlaveAddr,
                           uint16_t MemAddr,
                           uint8_t MemAddrSize,
                           uint8_t *pData,
                           uint32_t Len)
{
    I2C_Status_t status;
    uint32_t i;
    uint32_t totalBytes;

    if((MemAddrSize != I2C_MEM_ADDR_8BIT) && (MemAddrSize != I2C_MEM_ADDR_16BIT))
        return I2C_ERROR;

    totalBytes = Len + MemAddrSize;

    status = I2C_WaitUntilBusFree(pI2Cx);
    if(status != I2C_READY) return status;

    pI2Cx->CR2 = 0U;
    pI2Cx->CR2 |= ((uint32_t)(SlaveAddr << 1));
    pI2Cx->CR2 |= (totalBytes << I2C_CR2_NBYTES);
    pI2Cx->CR2 |= (1U << I2C_CR2_AUTOEND);
    pI2Cx->CR2 |= (1U << I2C_CR2_START);

    status = I2C_WaitForTXIS(pI2Cx);
    if(status != I2C_READY) return status;

    if(MemAddrSize == I2C_MEM_ADDR_16BIT)
    {
        pI2Cx->TXDR = (uint8_t)(MemAddr >> 8);
        status = I2C_WaitForTXIS(pI2Cx);
        if(status != I2C_READY) return status;
    }

    pI2Cx->TXDR = (uint8_t)(MemAddr & 0xFFU);

    for(i = 0; i < Len; i++)
    {
        status = I2C_WaitForTXIS(pI2Cx);
        if(status != I2C_READY) return status;
        pI2Cx->TXDR = pData[i];
    }

    return I2C_WaitForSTOP(pI2Cx);
}

I2C_Status_t I2C_Mem_Read(I2C_RegDef_t *pI2Cx,
                          uint8_t SlaveAddr,
                          uint16_t MemAddr,
                          uint8_t MemAddrSize,
                          uint8_t *pData,
                          uint32_t Len)
{
    I2C_Status_t status;
    uint32_t i;

    if((MemAddrSize != I2C_MEM_ADDR_8BIT) && (MemAddrSize != I2C_MEM_ADDR_16BIT))
        return I2C_ERROR;

    if(Len == 0U)
        return I2C_ERROR;

    status = I2C_WaitUntilBusFree(pI2Cx);
    if(status != I2C_READY) return status;

    pI2Cx->CR2 = 0U;
    pI2Cx->CR2 |= ((uint32_t)(SlaveAddr << 1));
    pI2Cx->CR2 |= ((uint32_t)MemAddrSize << I2C_CR2_NBYTES);
    pI2Cx->CR2 |= (1U << I2C_CR2_START);

    status = I2C_WaitForTXIS(pI2Cx);
    if(status != I2C_READY) return status;

    if(MemAddrSize == I2C_MEM_ADDR_16BIT)
    {
        pI2Cx->TXDR = (uint8_t)(MemAddr >> 8);
        status = I2C_WaitForTXIS(pI2Cx);
        if(status != I2C_READY) return status;
    }

    pI2Cx->TXDR = (uint8_t)(MemAddr & 0xFFU);

    status = I2C_WaitForTC(pI2Cx);
    if(status != I2C_READY) return status;

    pI2Cx->CR2 = 0U;
    pI2Cx->CR2 |= ((uint32_t)(SlaveAddr << 1));
    pI2Cx->CR2 |= (1U << I2C_CR2_RD_WRN);
    pI2Cx->CR2 |= ((uint32_t)Len << I2C_CR2_NBYTES);
    pI2Cx->CR2 |= (1U << I2C_CR2_AUTOEND);
    pI2Cx->CR2 |= (1U << I2C_CR2_START);

    for(i = 0; i < Len; i++)
    {
        status = I2C_WaitForRXNE(pI2Cx);
        if(status != I2C_READY) return status;
        pData[i] = (uint8_t)pI2Cx->RXDR;
    }

    return I2C_WaitForSTOP(pI2Cx);
}

/* ---------------- INTERRUPT APIs ---------------- */

I2C_Status_t I2C_MasterSendDataIT(I2C_Handle_t *pI2CHandle,
                                  uint8_t *pTxBuffer,
                                  uint32_t Len,
                                  uint8_t SlaveAddr)
{
    I2C_Status_t state = I2C_READY;

    if(pI2CHandle->TxRxState != I2C_READY_STATE)
    {
        return I2C_BUSY_IN_TX;
    }

    state = I2C_WaitUntilBusFree(pI2CHandle->pI2Cx);
    if(state != I2C_READY)
    {
        return state;
    }

    pI2CHandle->pTxBuffer = pTxBuffer;
    pI2CHandle->TxLen = Len;
    pI2CHandle->DevAddr = SlaveAddr;
    pI2CHandle->TxRxState = I2C_BUSY_IN_TX_STATE;

    pI2CHandle->pI2Cx->CR2 = 0U;
    pI2CHandle->pI2Cx->CR2 |= ((uint32_t)(SlaveAddr << 1));
    pI2CHandle->pI2Cx->CR2 |= (Len << I2C_CR2_NBYTES);
    pI2CHandle->pI2Cx->CR2 |= (1U << I2C_CR2_AUTOEND);
    pI2CHandle->pI2Cx->CR2 |= (1U << I2C_CR2_START);

    pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_TXIE);
    pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_STOPIE);
    pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_NACKIE);
    pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_ERRIE);

    return I2C_READY;
}

I2C_Status_t I2C_MasterReceiveDataIT(I2C_Handle_t *pI2CHandle,
                                     uint8_t *pRxBuffer,
                                     uint32_t Len,
                                     uint8_t SlaveAddr)
{
    I2C_Status_t state = I2C_READY;

    if(pI2CHandle->TxRxState != I2C_READY_STATE)
    {
        return I2C_BUSY_IN_RX;
    }

    state = I2C_WaitUntilBusFree(pI2CHandle->pI2Cx);
    if(state != I2C_READY)
    {
        return state;
    }

    pI2CHandle->pRxBuffer = pRxBuffer;
    pI2CHandle->RxLen = Len;
    pI2CHandle->RxSize = Len;
    pI2CHandle->DevAddr = SlaveAddr;
    pI2CHandle->TxRxState = I2C_BUSY_IN_RX_STATE;

    pI2CHandle->pI2Cx->CR2 = 0U;
    pI2CHandle->pI2Cx->CR2 |= ((uint32_t)(SlaveAddr << 1));
    pI2CHandle->pI2Cx->CR2 |= (1U << I2C_CR2_RD_WRN);
    pI2CHandle->pI2Cx->CR2 |= (Len << I2C_CR2_NBYTES);
    pI2CHandle->pI2Cx->CR2 |= (1U << I2C_CR2_AUTOEND);
    pI2CHandle->pI2Cx->CR2 |= (1U << I2C_CR2_START);

    pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_RXIE);
    pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_STOPIE);
    pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_NACKIE);
    pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_ERRIE);

    return I2C_READY;
}

void I2C_CloseSendData(I2C_Handle_t *pI2CHandle)
{
    pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_TXIE);
    pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_STOPIE);
    pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_NACKIE);
    pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_ERRIE);

    pI2CHandle->pTxBuffer = 0;
    pI2CHandle->TxLen = 0;
    pI2CHandle->TxRxState = I2C_READY_STATE;
}

void I2C_CloseReceiveData(I2C_Handle_t *pI2CHandle)
{
    pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_RXIE);
    pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_STOPIE);
    pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_NACKIE);
    pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_ERRIE);

    pI2CHandle->pRxBuffer = 0;
    pI2CHandle->RxLen = 0;
    pI2CHandle->RxSize = 0;
    pI2CHandle->TxRxState = I2C_READY_STATE;
}

void I2C_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi)
{
    if(EnorDi == ENABLE)
    {
        if(IRQNumber <= 31)
        {
            *NVIC_ISER0 |= (1U << IRQNumber);
        }
        else if(IRQNumber < 64)
        {
            *NVIC_ISER1 |= (1U << (IRQNumber % 32));
        }
        else if(IRQNumber < 96)
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
        else if(IRQNumber < 64)
        {
            *NVIC_ICER1 |= (1U << (IRQNumber % 32));
        }
        else if(IRQNumber < 96)
        {
            *NVIC_ICER2 |= (1U << (IRQNumber % 64));
        }
    }
}

void I2C_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority)
{
    uint8_t iprx = IRQNumber / 4U;
    uint8_t iprx_section = IRQNumber % 4U;
    uint8_t shift_amount = (8U * iprx_section) + (8U - NO_PR_BITS_IMPLEMENTED);

    *(NVIC_PR_BASEADDR + iprx) |= (IRQPriority << shift_amount);
}

void I2C_EV_IRQHandling(I2C_Handle_t *pI2CHandle)
{
    uint32_t temp1, temp2;
    I2C_RegDef_t *pI2Cx = pI2CHandle->pI2Cx;

    temp1 = pI2Cx->ISR & (1U << I2C_ISR_TXIS);
    temp2 = pI2Cx->CR1 & (1U << I2C_CR1_TXIE);

    if(temp1 && temp2)
    {
        if((pI2CHandle->TxRxState == I2C_BUSY_IN_TX_STATE) && (pI2CHandle->TxLen > 0U))
        {
            pI2Cx->TXDR = *(pI2CHandle->pTxBuffer);
            pI2CHandle->pTxBuffer++;
            pI2CHandle->TxLen--;
        }
    }

    temp1 = pI2Cx->ISR & (1U << I2C_ISR_RXNE);
    temp2 = pI2Cx->CR1 & (1U << I2C_CR1_RXIE);

    if(temp1 && temp2)
    {
        if((pI2CHandle->TxRxState == I2C_BUSY_IN_RX_STATE) && (pI2CHandle->RxLen > 0U))
        {
            *(pI2CHandle->pRxBuffer) = (uint8_t)pI2Cx->RXDR;
            pI2CHandle->pRxBuffer++;
            pI2CHandle->RxLen--;
        }
    }

    temp1 = pI2Cx->ISR & (1U << I2C_ISR_STOPF);
    temp2 = pI2Cx->CR1 & (1U << I2C_CR1_STOPIE);

    if(temp1 && temp2)
    {
        I2C_ClearStopFlag(pI2Cx);

        if(pI2CHandle->TxRxState == I2C_BUSY_IN_TX_STATE)
        {
            I2C_CloseSendData(pI2CHandle);
            I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_TX_CMPLT);
        }
        else if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX_STATE)
        {
            I2C_CloseReceiveData(pI2CHandle);
            I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_RX_CMPLT);
        }
    }

    temp1 = pI2Cx->ISR & (1U << I2C_ISR_NACKF);
    temp2 = pI2Cx->CR1 & (1U << I2C_CR1_NACKIE);

    if(temp1 && temp2)
    {
        I2C_ClearNackFlag(pI2Cx);
        I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_NACKF);
    }
}

void I2C_ER_IRQHandling(I2C_Handle_t *pI2CHandle)
{
    I2C_RegDef_t *pI2Cx = pI2CHandle->pI2Cx;

    if(pI2Cx->ISR & (1U << I2C_ISR_BERR))
    {
        pI2Cx->ICR |= (1U << I2C_ICR_BERRCF);
        I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_BERR);
    }

    if(pI2Cx->ISR & (1U << I2C_ISR_ARLO))
    {
        pI2Cx->ICR |= (1U << I2C_ICR_ARLOCF);
        I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_ARLO);
    }

    if(pI2Cx->ISR & (1U << I2C_ISR_OVR))
    {
        pI2Cx->ICR |= (1U << I2C_ICR_OVRCF);
        I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_OVR);
    }
}

__attribute__((weak)) void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle, uint8_t AppEv)
{
    (void)pI2CHandle;
    (void)AppEv;
}
