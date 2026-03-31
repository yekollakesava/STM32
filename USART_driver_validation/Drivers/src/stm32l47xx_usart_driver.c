#include "stm32l47xx_usart_driver.h"
#include <stddef.h>

static uint32_t RCC_GetPCLK1Value(void);
static void USART_HandleTXEInterrupt(USART_Handle_t *pUSARTHandle);
static void USART_HandleRXNEInterrupt(USART_Handle_t *pUSARTHandle);
static void USART_EndTx(USART_Handle_t *pUSARTHandle);
static void USART_EndRx(USART_Handle_t *pUSARTHandle);

/**************************************
 * Peripheral clock control
 **************************************/
void USART_PeriClockControl(USART_RegDef_t *pUSARTx, uint8_t EnorDi)
{
    if(EnorDi == ENABLE)
    {
        if(pUSARTx == USART2)
        {
            USART2_PCLK_EN();
        }
    }
    else
    {
        if(pUSARTx == USART2)
        {
            USART2_PCLK_DI();
        }
    }
}

/**************************************
 * Enable / Disable USART peripheral
 **************************************/
void USART_PeripheralControl(USART_RegDef_t *pUSARTx, uint8_t EnorDi)
{
    if(EnorDi == ENABLE)
    {
        pUSARTx->CR1 |= (1U << USART_CR1_UE);
    }
    else
    {
        pUSARTx->CR1 &= ~(1U << USART_CR1_UE);
    }
}

/**************************************
 * Get flag status
 **************************************/
uint8_t USART_GetFlagStatus(USART_RegDef_t *pUSARTx, uint32_t FlagName)
{
    if(pUSARTx->ISR & FlagName)
    {
        return FLAG_SET;
    }
    return FLAG_RESET;
}

/**************************************
 * Baud rate configuration
 * Assumes PCLK1 = 16 MHz
 **************************************/
void USART_SetBaudRate(USART_RegDef_t *pUSARTx, uint32_t BaudRate)
{
    uint32_t pclk;
    uint32_t usartdiv;
    uint32_t mantissa;
    uint32_t fraction;
    uint32_t tempreg = 0;

    (void)pUSARTx;

    pclk = RCC_GetPCLK1Value();

    usartdiv = ((25U * pclk) / (4U * BaudRate));
    mantissa = usartdiv / 100U;
    tempreg |= (mantissa << 4);

    fraction = usartdiv - (mantissa * 100U);
    fraction = (((fraction * 16U) + 50U) / 100U) & 0xFU;

    tempreg |= fraction;

    pUSARTx->BRR = tempreg;
}

/**************************************
 * USART Init
 **************************************/
void USART_Init(USART_Handle_t *pUSARTHandle)
{
    uint32_t tempreg = 0;

    USART_PeriClockControl(pUSARTHandle->pUSARTx, ENABLE);

    /************** CR1 **************/
    tempreg = 0;

    if(pUSARTHandle->USART_Config.USART_Mode == USART_MODE_ONLY_RX)
    {
        tempreg |= (1U << USART_CR1_RE);
    }
    else if(pUSARTHandle->USART_Config.USART_Mode == USART_MODE_ONLY_TX)
    {
        tempreg |= (1U << USART_CR1_TE);
    }
    else if(pUSARTHandle->USART_Config.USART_Mode == USART_MODE_TXRX)
    {
        tempreg |= (1U << USART_CR1_RE);
        tempreg |= (1U << USART_CR1_TE);
    }

    if(pUSARTHandle->USART_Config.USART_WordLength == USART_WORDLEN_9BITS)
    {
        tempreg |= (1U << USART_CR1_M0);
        tempreg &= ~(1U << USART_CR1_M1);
    }
    else
    {
        tempreg &= ~(1U << USART_CR1_M0);
        tempreg &= ~(1U << USART_CR1_M1);
    }

    if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_EN_EVEN)
    {
        tempreg |= (1U << USART_CR1_PCE);
        tempreg &= ~(1U << USART_CR1_PS);
    }
    else if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_EN_ODD)
    {
        tempreg |= (1U << USART_CR1_PCE);
        tempreg |= (1U << USART_CR1_PS);
    }
    else
    {
        tempreg &= ~(1U << USART_CR1_PCE);
    }

    pUSARTHandle->pUSARTx->CR1 = tempreg;

    /************** CR2 **************/
    tempreg = 0;
    tempreg |= ((uint32_t)pUSARTHandle->USART_Config.USART_NoOfStopBits << USART_CR2_STOP);
    pUSARTHandle->pUSARTx->CR2 = tempreg;

    /************** CR3 **************/
    tempreg = 0;

    if(pUSARTHandle->USART_Config.USART_HWFlowControl == USART_HW_FLOW_CTRL_CTS)
    {
        tempreg |= (1U << USART_CR3_CTSE);
    }
    else if(pUSARTHandle->USART_Config.USART_HWFlowControl == USART_HW_FLOW_CTRL_RTS)
    {
        tempreg |= (1U << USART_CR3_RTSE);
    }
    else if(pUSARTHandle->USART_Config.USART_HWFlowControl == USART_HW_FLOW_CTRL_CTS_RTS)
    {
        tempreg |= (1U << USART_CR3_CTSE);
        tempreg |= (1U << USART_CR3_RTSE);
    }

    pUSARTHandle->pUSARTx->CR3 = tempreg;

    USART_SetBaudRate(pUSARTHandle->pUSARTx, pUSARTHandle->USART_Config.USART_Baud);
    USART_PeripheralControl(pUSARTHandle->pUSARTx, ENABLE);

    pUSARTHandle->TxBusyState = USART_READY;
    pUSARTHandle->RxBusyState = USART_READY;
    pUSARTHandle->pTxBuffer = NULL;
    pUSARTHandle->pRxBuffer = NULL;
    pUSARTHandle->TxLen = 0;
    pUSARTHandle->RxLen = 0;
}

/**************************************
 * USART DeInit
 **************************************/
void USART_DeInit(USART_RegDef_t *pUSARTx)
{
    if(pUSARTx == USART2)
    {
        USART2_REG_RESET();
    }
}

/**************************************
 * Polling based send
 **************************************/
void USART_SendData(USART_Handle_t *pUSARTHandle, uint8_t *pTxBuffer, uint32_t Len)
{
    uint16_t *pdata;

    while(Len > 0)
    {
        while(USART_GetFlagStatus(pUSARTHandle->pUSARTx, USART_FLAG_TXE) == FLAG_RESET);

        if(pUSARTHandle->USART_Config.USART_WordLength == USART_WORDLEN_9BITS)
        {
            pdata = (uint16_t*)pTxBuffer;
            pUSARTHandle->pUSARTx->TDR = (*pdata & (uint16_t)0x01FF);

            if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE)
            {
                pTxBuffer += 2;
                Len -= 2;
            }
            else
            {
                pTxBuffer++;
                Len--;
            }
        }
        else
        {
            pUSARTHandle->pUSARTx->TDR = (*pTxBuffer & (uint8_t)0xFF);
            pTxBuffer++;
            Len--;
        }
    }

    while(USART_GetFlagStatus(pUSARTHandle->pUSARTx, USART_FLAG_TC) == FLAG_RESET);
}

/**************************************
 * Polling based receive
 **************************************/
void USART_ReceiveData(USART_Handle_t *pUSARTHandle, uint8_t *pRxBuffer, uint32_t Len)
{
    while(Len > 0)
    {
        while(USART_GetFlagStatus(pUSARTHandle->pUSARTx, USART_FLAG_RXNE) == FLAG_RESET);

        if(pUSARTHandle->USART_Config.USART_WordLength == USART_WORDLEN_9BITS)
        {
            if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE)
            {
                *((uint16_t*)pRxBuffer) = (pUSARTHandle->pUSARTx->RDR & (uint16_t)0x01FF);
                pRxBuffer += 2;
                Len -= 2;
            }
            else
            {
                *pRxBuffer = (uint8_t)(pUSARTHandle->pUSARTx->RDR & (uint8_t)0xFF);
                pRxBuffer++;
                Len--;
            }
        }
        else
        {
            if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE)
            {
                *pRxBuffer = (uint8_t)(pUSARTHandle->pUSARTx->RDR & (uint8_t)0xFF);
            }
            else
            {
                *pRxBuffer = (uint8_t)(pUSARTHandle->pUSARTx->RDR & (uint8_t)0x7F);
            }

            pRxBuffer++;
            Len--;
        }
    }
}

/**************************************
 * Interrupt based send
 **************************************/
uint8_t USART_SendDataIT(USART_Handle_t *pUSARTHandle, uint8_t *pTxBuffer, uint32_t Len)
{
    uint8_t txstate = pUSARTHandle->TxBusyState;

    if(txstate != USART_BUSY_IN_TX)
    {
        pUSARTHandle->pTxBuffer = pTxBuffer;
        pUSARTHandle->TxLen = Len;
        pUSARTHandle->TxBusyState = USART_BUSY_IN_TX;

        pUSARTHandle->pUSARTx->CR1 |= (1U << USART_CR1_TXEIE);

        return USART_READY;
    }

    return txstate;
}

/**************************************
 * Interrupt based receive
 **************************************/
uint8_t USART_ReceiveDataIT(USART_Handle_t *pUSARTHandle, uint8_t *pRxBuffer, uint32_t Len)
{
    uint8_t rxstate = pUSARTHandle->RxBusyState;

    if(rxstate != USART_BUSY_IN_RX)
    {
        pUSARTHandle->pRxBuffer = pRxBuffer;
        pUSARTHandle->RxLen = Len;
        pUSARTHandle->RxBusyState = USART_BUSY_IN_RX;

        pUSARTHandle->pUSARTx->CR1 |= (1U << USART_CR1_RXNEIE);

        return USART_READY;
    }

    return rxstate;
}

/**************************************
 * NVIC interrupt config
 **************************************/
void USART_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi)
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

/**************************************
 * NVIC priority config
 **************************************/
void USART_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority)
{
    uint8_t iprx = IRQNumber / 4;
    uint8_t iprx_section = IRQNumber % 4;
    uint8_t shift_amount = (8U * iprx_section) + (8U - NO_PR_BITS_IMPLEMENTED);

    *(NVIC_PR_BASEADDR + iprx) &= ~(0xFFU << (8U * iprx_section));
    *(NVIC_PR_BASEADDR + iprx) |= (IRQPriority << shift_amount);
}

/**************************************
 * USART IRQ handling
 **************************************/
void USART_IRQHandling(USART_Handle_t *pHandle)
{
    uint32_t temp1, temp2;

    temp1 = pHandle->pUSARTx->ISR & USART_FLAG_PE;
    temp2 = pHandle->pUSARTx->CR1 & (1U << USART_CR1_PEIE);
    if(temp1 && temp2)
    {
        pHandle->pUSARTx->ICR |= (1U << USART_ICR_PECF);
        USART_ApplicationEventCallback(pHandle, USART_EVENT_PE);
    }

    temp1 = pHandle->pUSARTx->ISR & USART_FLAG_FE;
    temp2 = pHandle->pUSARTx->CR3 & (1U << USART_CR3_EIE);
    if(temp1 && temp2)
    {
        pHandle->pUSARTx->ICR |= (1U << USART_ICR_FECF);
        USART_ApplicationEventCallback(pHandle, USART_ERR_FE);
    }

    temp1 = pHandle->pUSARTx->ISR & USART_FLAG_NE;
    temp2 = pHandle->pUSARTx->CR3 & (1U << USART_CR3_EIE);
    if(temp1 && temp2)
    {
        pHandle->pUSARTx->ICR |= (1U << USART_ICR_NCF);
        USART_ApplicationEventCallback(pHandle, USART_ERR_NE);
    }

    temp1 = pHandle->pUSARTx->ISR & USART_FLAG_ORE;
    temp2 = pHandle->pUSARTx->CR3 & (1U << USART_CR3_EIE);
    if(temp1 && temp2)
    {
        pHandle->pUSARTx->ICR |= (1U << USART_ICR_ORECF);
        USART_ApplicationEventCallback(pHandle, USART_ERR_ORE);
    }

    temp1 = pHandle->pUSARTx->ISR & USART_FLAG_IDLE;
    temp2 = pHandle->pUSARTx->CR1 & (1U << USART_CR1_IDLEIE);
    if(temp1 && temp2)
    {
        pHandle->pUSARTx->ICR |= (1U << USART_ICR_IDLECF);
        USART_ApplicationEventCallback(pHandle, USART_EVENT_IDLE);
    }

    temp1 = pHandle->pUSARTx->ISR & USART_FLAG_RXNE;
    temp2 = pHandle->pUSARTx->CR1 & (1U << USART_CR1_RXNEIE);
    if(temp1 && temp2)
    {
        USART_HandleRXNEInterrupt(pHandle);
    }

    temp1 = pHandle->pUSARTx->ISR & USART_FLAG_TC;
    temp2 = pHandle->pUSARTx->CR1 & (1U << USART_CR1_TCIE);
    if(temp1 && temp2)
    {
        USART_EndTx(pHandle);
    }

    temp1 = pHandle->pUSARTx->ISR & USART_FLAG_TXE;
    temp2 = pHandle->pUSARTx->CR1 & (1U << USART_CR1_TXEIE);
    if(temp1 && temp2)
    {
        USART_HandleTXEInterrupt(pHandle);
    }
}

/**************************************
 * TXE ISR helper
 **************************************/
static void USART_HandleTXEInterrupt(USART_Handle_t *pUSARTHandle)
{
    if(pUSARTHandle->TxLen > 0U)
    {
        if(pUSARTHandle->USART_Config.USART_WordLength == USART_WORDLEN_9BITS)
        {
            pUSARTHandle->pUSARTx->TDR = (*((uint16_t*)pUSARTHandle->pTxBuffer) & (uint16_t)0x01FF);

            if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE)
            {
                pUSARTHandle->pTxBuffer += 2;
                pUSARTHandle->TxLen -= 2;
            }
            else
            {
                pUSARTHandle->pTxBuffer++;
                pUSARTHandle->TxLen--;
            }
        }
        else
        {
            pUSARTHandle->pUSARTx->TDR = (*pUSARTHandle->pTxBuffer & (uint8_t)0xFF);
            pUSARTHandle->pTxBuffer++;
            pUSARTHandle->TxLen--;
        }
    }

    if(pUSARTHandle->TxLen == 0U)
    {
        pUSARTHandle->pUSARTx->CR1 &= ~(1U << USART_CR1_TXEIE);
        pUSARTHandle->pUSARTx->CR1 |=  (1U << USART_CR1_TCIE);
    }
}

/**************************************
 * RXNE ISR helper
 **************************************/
static void USART_HandleRXNEInterrupt(USART_Handle_t *pUSARTHandle)
{
    if(pUSARTHandle->RxLen > 0U)
    {
        if(pUSARTHandle->USART_Config.USART_WordLength == USART_WORDLEN_9BITS)
        {
            if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE)
            {
                *((uint16_t*)pUSARTHandle->pRxBuffer) =
                    (pUSARTHandle->pUSARTx->RDR & (uint16_t)0x01FF);
                pUSARTHandle->pRxBuffer += 2;
                pUSARTHandle->RxLen -= 2;
            }
            else
            {
                *pUSARTHandle->pRxBuffer =
                    (uint8_t)(pUSARTHandle->pUSARTx->RDR & (uint8_t)0xFF);
                pUSARTHandle->pRxBuffer++;
                pUSARTHandle->RxLen--;
            }
        }
        else
        {
            if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE)
            {
                *pUSARTHandle->pRxBuffer =
                    (uint8_t)(pUSARTHandle->pUSARTx->RDR & (uint8_t)0xFF);
            }
            else
            {
                *pUSARTHandle->pRxBuffer =
                    (uint8_t)(pUSARTHandle->pUSARTx->RDR & (uint8_t)0x7F);
            }

            pUSARTHandle->pRxBuffer++;
            pUSARTHandle->RxLen--;
        }
    }

    if(pUSARTHandle->RxLen == 0U)
    {
        USART_EndRx(pUSARTHandle);
        USART_ApplicationEventCallback(pUSARTHandle, USART_EVENT_RX_CMPLT);
    }
}

/**************************************
 * End TX helper
 **************************************/
static void USART_EndTx(USART_Handle_t *pUSARTHandle)
{
    pUSARTHandle->pUSARTx->CR1 &= ~(1U << USART_CR1_TCIE);
    pUSARTHandle->pUSARTx->ICR |= (1U << USART_ICR_TCCF);

    pUSARTHandle->TxLen = 0;
    pUSARTHandle->pTxBuffer = NULL;
    pUSARTHandle->TxBusyState = USART_READY;

    USART_ApplicationEventCallback(pUSARTHandle, USART_EVENT_TX_CMPLT);
}

/**************************************
 * End RX helper
 **************************************/
static void USART_EndRx(USART_Handle_t *pUSARTHandle)
{
    pUSARTHandle->pUSARTx->CR1 &= ~(1U << USART_CR1_RXNEIE);

    pUSARTHandle->RxLen = 0;
    pUSARTHandle->pRxBuffer = NULL;
    pUSARTHandle->RxBusyState = USART_READY;
}

/**************************************
 * Weak callback
 **************************************/
__attribute__((weak)) void USART_ApplicationEventCallback(USART_Handle_t *pUSARTHandle, uint8_t AppEv)
{
    (void)pUSARTHandle;
    (void)AppEv;
}

/**************************************
 * Clock helper
 **************************************/
static uint32_t RCC_GetPCLK1Value(void)
{
    return 16000000U;
}
