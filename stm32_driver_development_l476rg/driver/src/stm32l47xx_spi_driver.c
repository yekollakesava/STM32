#include "stm32l47xx_spi_driver.h"

static void spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle);
static void spi_rxne_interrupt_handle(SPI_Handle_t *pSPIHandle);
static void spi_ovr_err_interrupt_handle(SPI_Handle_t *pSPIHandle);

void SPI_PeriClockControl(SPI_RegDef_t *pSPIx, uint8_t EnOrDi)
{
    if(EnOrDi == ENABLE)
    {
        if(pSPIx == SPI1)
        {
            SPI1_PCLK_EN();
        }
        else if(pSPIx == SPI2)
        {
            SPI2_PCLK_EN();
        }
        else if(pSPIx == SPI3)
        {
            SPI3_PCLK_EN();
        }
    }
    else
    {
        if(pSPIx == SPI1)
        {
            SPI1_PCLK_DI();
        }
        else if(pSPIx == SPI2)
        {
            SPI2_PCLK_DI();
        }
        else if(pSPIx == SPI3)
        {
            SPI3_PCLK_DI();
        }
    }
}

void SPI_Init(SPI_Handle_t *pSPIHandle)
{
    uint32_t tempreg = 0;

    SPI_PeriClockControl(pSPIHandle->pSPIx, ENABLE);

    /* Device mode */
    tempreg |= (pSPIHandle->SPIConfig.SPI_DeviceMode << SPI_CR1_MSTR);

    /* Bus config */
    if(pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_FD)
    {
        tempreg &= ~(1U << SPI_CR1_BIDIMODE);
        tempreg &= ~(1U << SPI_CR1_RXONLY);
    }
    else if(pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_HD)
    {
        tempreg |= (1U << SPI_CR1_BIDIMODE);
        tempreg &= ~(1U << SPI_CR1_RXONLY);
    }
    else if(pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_SIMPLEX_RX)
    {
        tempreg &= ~(1U << SPI_CR1_BIDIMODE);
        tempreg |=  (1U << SPI_CR1_RXONLY);
    }

    /* Serial clock speed */
    tempreg |= (pSPIHandle->SPIConfig.SPI_SclkSpeed << SPI_CR1_BR);

    /* CPOL */
    tempreg |= (pSPIHandle->SPIConfig.SPI_CPOL << SPI_CR1_CPOL);

    /* CPHA */
    tempreg |= (pSPIHandle->SPIConfig.SPI_CPHA << SPI_CR1_CPHA);

    /* SSM */
    tempreg |= (pSPIHandle->SPIConfig.SPI_SSM << SPI_CR1_SSM);

    pSPIHandle->pSPIx->CR1 = tempreg;

    /* STM32L4 uses DS[3:0] in CR2 instead of DFF in CR1 */
    pSPIHandle->pSPIx->CR2 &= ~(0xFU << SPI_CR2_DS);
    pSPIHandle->pSPIx->CR2 |=  ((uint32_t)pSPIHandle->SPIConfig.SPI_DataSize << SPI_CR2_DS);

    /* For 8-bit reception, set FIFO threshold */
    if(pSPIHandle->SPIConfig.SPI_DataSize == SPI_DATASIZE_8BIT)
    {
        pSPIHandle->pSPIx->CR2 |= (1U << SPI_CR2_FRXTH);
    }
    else
    {
        pSPIHandle->pSPIx->CR2 &= ~(1U << SPI_CR2_FRXTH);
    }

    pSPIHandle->TxState = SPI_READY;
    pSPIHandle->RxState = SPI_READY;
    pSPIHandle->pTxBuffer = 0;
    pSPIHandle->pRxBuffer = 0;
    pSPIHandle->TxLen = 0;
    pSPIHandle->RxLen = 0;
}

void SPI_DeInit(SPI_RegDef_t *pSPIx)
{
    if(pSPIx == SPI1)
    {
        SPI1_REG_RESET();
    }
    else if(pSPIx == SPI2)
    {
        SPI2_REG_RESET();
    }
    else if(pSPIx == SPI3)
    {
        SPI3_REG_RESET();
    }
}

uint8_t SPI_GetFlagStatus(SPI_RegDef_t *pSPIx, uint32_t FlagName)
{
    if(pSPIx->SR & FlagName)
    {
        return FLAG_SET;
    }
    return FLAG_RESET;
}

void SPI_PeripheralControl(SPI_RegDef_t *pSPIx, uint8_t EnOrDi)
{
    if(EnOrDi == ENABLE)
    {
        pSPIx->CR1 |= (1U << SPI_CR1_SPE);
    }
    else
    {
        pSPIx->CR1 &= ~(1U << SPI_CR1_SPE);
    }
}

void SPI_SSIConfig(SPI_RegDef_t *pSPIx, uint8_t EnOrDi)
{
    if(EnOrDi == ENABLE)
    {
        pSPIx->CR1 |= (1U << SPI_CR1_SSI);
    }
    else
    {
        pSPIx->CR1 &= ~(1U << SPI_CR1_SSI);
    }
}

void SPI_SSOEConfig(SPI_RegDef_t *pSPIx, uint8_t EnOrDi)
{
    if(EnOrDi == ENABLE)
    {
        pSPIx->CR2 |= (1U << SPI_CR2_SSOE);
    }
    else
    {
        pSPIx->CR2 &= ~(1U << SPI_CR2_SSOE);
    }
}

void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint32_t Len)
{
    while(Len > 0)
    {
        while(SPI_GetFlagStatus(pSPIx, SPI_TXE_FLAG) == FLAG_RESET);

        if(((pSPIx->CR2 >> SPI_CR2_DS) & 0xFU) == SPI_DATASIZE_16BIT)
        {
            pSPIx->DR = *((uint16_t*)pTxBuffer);
            Len -= 2U;
            pTxBuffer += 2;
        }
        else
        {
            *((__vo uint8_t*)&pSPIx->DR) = *pTxBuffer;
            Len--;
            pTxBuffer++;
        }
    }

    while(SPI_GetFlagStatus(pSPIx, SPI_BSY_FLAG) == FLAG_SET);
}

void SPI_ReceiveData(SPI_RegDef_t *pSPIx, uint8_t *pRxBuffer, uint32_t Len)
{
    while(Len > 0)
    {
        while(SPI_GetFlagStatus(pSPIx, SPI_RXNE_FLAG) == FLAG_RESET);

        if(((pSPIx->CR2 >> SPI_CR2_DS) & 0xFU) == SPI_DATASIZE_16BIT)
        {
            *((uint16_t*)pRxBuffer) = (uint16_t)pSPIx->DR;
            Len -= 2U;
            pRxBuffer += 2;
        }
        else
        {
            *pRxBuffer = *((__vo uint8_t*)&pSPIx->DR);
            Len--;
            pRxBuffer++;
        }
    }
}

uint8_t SPI_SendDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pTxBuffer, uint32_t Len)
{
    uint8_t state = pSPIHandle->TxState;

    if(state != SPI_BUSY_IN_TX)
    {
        pSPIHandle->pTxBuffer = pTxBuffer;
        pSPIHandle->TxLen = Len;
        pSPIHandle->TxState = SPI_BUSY_IN_TX;

        pSPIHandle->pSPIx->CR2 |= (1U << SPI_CR2_TXEIE);
    }

    return state;
}

uint8_t SPI_ReceiveDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pRxBuffer, uint32_t Len)
{
    uint8_t state = pSPIHandle->RxState;

    if(state != SPI_BUSY_IN_RX)
    {
        pSPIHandle->pRxBuffer = pRxBuffer;
        pSPIHandle->RxLen = Len;
        pSPIHandle->RxState = SPI_BUSY_IN_RX;

        pSPIHandle->pSPIx->CR2 |= (1U << SPI_CR2_RXNEIE);
    }

    return state;
}

void SPI_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi)
{
    if(EnOrDi == ENABLE)
    {
        if(IRQNumber <= 31U)
        {
            *NVIC_ISER0 |= (1U << IRQNumber);
        }
        else if(IRQNumber < 64U)
        {
            *NVIC_ISER1 |= (1U << (IRQNumber % 32U));
        }
        else if(IRQNumber < 96U)
        {
            *NVIC_ISER2 |= (1U << (IRQNumber % 64U));
        }
    }
    else
    {
        if(IRQNumber <= 31U)
        {
            *NVIC_ICER0 |= (1U << IRQNumber);
        }
        else if(IRQNumber < 64U)
        {
            *NVIC_ICER1 |= (1U << (IRQNumber % 32U));
        }
        else if(IRQNumber < 96U)
        {
            *NVIC_ICER2 |= (1U << (IRQNumber % 64U));
        }
    }
}

void SPI_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority)
{
    uint8_t iprx = IRQNumber / 4U;
    uint8_t iprx_section = IRQNumber % 4U;
    uint8_t shift_amount = (uint8_t)((8U * iprx_section) + (8U - NO_PR_BITS_IMPLEMENTED));

    *(NVIC_PR_BASEADDR + iprx) |= (IRQPriority << shift_amount);
}

void SPI_IRQHandling(SPI_Handle_t *pHandle)
{
    uint8_t temp1, temp2;

    /* TXE */
    temp1 = (uint8_t)(pHandle->pSPIx->SR & (1U << SPI_SR_TXE));
    temp2 = (uint8_t)(pHandle->pSPIx->CR2 & (1U << SPI_CR2_TXEIE));
    if(temp1 && temp2)
    {
        spi_txe_interrupt_handle(pHandle);
    }

    /* RXNE */
    temp1 = (uint8_t)(pHandle->pSPIx->SR & (1U << SPI_SR_RXNE));
    temp2 = (uint8_t)(pHandle->pSPIx->CR2 & (1U << SPI_CR2_RXNEIE));
    if(temp1 && temp2)
    {
        spi_rxne_interrupt_handle(pHandle);
    }

    /* OVR */
    temp1 = (uint8_t)(pHandle->pSPIx->SR & (1U << SPI_SR_OVR));
    temp2 = (uint8_t)(pHandle->pSPIx->CR2 & (1U << SPI_CR2_ERRIE));
    if(temp1 && temp2)
    {
        spi_ovr_err_interrupt_handle(pHandle);
    }
}

void SPI_ClearOVRFlag(SPI_RegDef_t *pSPIx)
{
    volatile uint8_t temp;

    temp = *((__vo uint8_t*)&pSPIx->DR);
    temp = *((__vo uint8_t*)&pSPIx->SR);
    (void)temp;
}

void SPI_CloseTransmission(SPI_Handle_t *pSPIHandle)
{
    pSPIHandle->pSPIx->CR2 &= ~(1U << SPI_CR2_TXEIE);
    pSPIHandle->pTxBuffer = 0;
    pSPIHandle->TxLen = 0;
    pSPIHandle->TxState = SPI_READY;
}

void SPI_CloseReception(SPI_Handle_t *pSPIHandle)
{
    pSPIHandle->pSPIx->CR2 &= ~(1U << SPI_CR2_RXNEIE);
    pSPIHandle->pRxBuffer = 0;
    pSPIHandle->RxLen = 0;
    pSPIHandle->RxState = SPI_READY;
}

static void spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
    if(((pSPIHandle->pSPIx->CR2 >> SPI_CR2_DS) & 0xFU) == SPI_DATASIZE_16BIT)
    {
        pSPIHandle->pSPIx->DR = *((uint16_t*)pSPIHandle->pTxBuffer);
        pSPIHandle->TxLen -= 2U;
        pSPIHandle->pTxBuffer += 2;
    }
    else
    {
        *((__vo uint8_t*)&pSPIHandle->pSPIx->DR) = *(pSPIHandle->pTxBuffer);
        pSPIHandle->TxLen--;
        pSPIHandle->pTxBuffer++;
    }

    if(pSPIHandle->TxLen == 0U)
    {
        while(SPI_GetFlagStatus(pSPIHandle->pSPIx, SPI_BSY_FLAG) == FLAG_SET);
        SPI_CloseTransmission(pSPIHandle);
        SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_TX_CMPLT);
    }
}

static void spi_rxne_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
    if(((pSPIHandle->pSPIx->CR2 >> SPI_CR2_DS) & 0xFU) == SPI_DATASIZE_16BIT)
    {
        *((uint16_t*)pSPIHandle->pRxBuffer) = (uint16_t)pSPIHandle->pSPIx->DR;
        pSPIHandle->RxLen -= 2U;
        pSPIHandle->pRxBuffer += 2;
    }
    else
    {
        *(pSPIHandle->pRxBuffer) = *((__vo uint8_t*)&pSPIHandle->pSPIx->DR);
        pSPIHandle->RxLen--;
        pSPIHandle->pRxBuffer++;
    }

    if(pSPIHandle->RxLen == 0U)
    {
        SPI_CloseReception(pSPIHandle);
        SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_RX_CMPLT);
    }
}

static void spi_ovr_err_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
    if(pSPIHandle->TxState != SPI_BUSY_IN_TX)
    {
        SPI_ClearOVRFlag(pSPIHandle->pSPIx);
    }

    SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_OVR_ERR);
}

__attribute__((weak)) void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle, uint8_t AppEv)
{
    (void)pSPIHandle;
    (void)AppEv;
}
