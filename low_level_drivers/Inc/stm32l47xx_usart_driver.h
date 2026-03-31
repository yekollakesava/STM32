#ifndef INC_STM32L47XX_USART_DRIVER_H_
#define INC_STM32L47XX_USART_DRIVER_H_

#include "stm32l47xx.h"

/**************************************
 * USART modes
 **************************************/
#define USART_MODE_ONLY_RX          0
#define USART_MODE_ONLY_TX          1
#define USART_MODE_TXRX             2

/**************************************
 * Baud rates
 **************************************/
#define USART_STD_BAUD_1200         1200
#define USART_STD_BAUD_2400         2400
#define USART_STD_BAUD_9600         9600
#define USART_STD_BAUD_19200        19200
#define USART_STD_BAUD_38400        38400
#define USART_STD_BAUD_57600        57600
#define USART_STD_BAUD_115200       115200

/**************************************
 * USART parity
 **************************************/
#define USART_PARITY_DISABLE        0
#define USART_PARITY_EN_EVEN        1
#define USART_PARITY_EN_ODD         2

/**************************************
 * USART word length
 **************************************/
#define USART_WORDLEN_8BITS         0
#define USART_WORDLEN_9BITS         1

/**************************************
 * USART stop bits
 **************************************/
#define USART_STOPBITS_1            0
#define USART_STOPBITS_0_5          1
#define USART_STOPBITS_2            2
#define USART_STOPBITS_1_5          3

/**************************************
 * USART hardware flow control
 **************************************/
#define USART_HW_FLOW_CTRL_NONE     0
#define USART_HW_FLOW_CTRL_CTS      1
#define USART_HW_FLOW_CTRL_RTS      2
#define USART_HW_FLOW_CTRL_CTS_RTS  3

/**************************************
 * USART states
 **************************************/
#define USART_READY                 0
#define USART_BUSY_IN_RX            1
#define USART_BUSY_IN_TX            2

/**************************************
 * USART events
 **************************************/
#define USART_EVENT_TX_CMPLT        0
#define USART_EVENT_RX_CMPLT        1
#define USART_EVENT_IDLE            2
#define USART_EVENT_PE              3
#define USART_ERR_FE                4
#define USART_ERR_NE                5
#define USART_ERR_ORE               6

typedef struct
{
    uint8_t USART_Mode;
    uint32_t USART_Baud;
    uint8_t USART_NoOfStopBits;
    uint8_t USART_WordLength;
    uint8_t USART_ParityControl;
    uint8_t USART_HWFlowControl;
} USART_Config_t;

typedef struct
{
    USART_RegDef_t *pUSARTx;
    USART_Config_t USART_Config;

    uint8_t *pTxBuffer;
    uint8_t *pRxBuffer;
    uint32_t TxLen;
    uint32_t RxLen;
    uint8_t TxBusyState;
    uint8_t RxBusyState;

} USART_Handle_t;

/**************************************
 * APIs
 **************************************/
void USART_PeriClockControl(USART_RegDef_t *pUSARTx, uint8_t EnorDi);
void USART_PeripheralControl(USART_RegDef_t *pUSARTx, uint8_t EnorDi);
uint8_t USART_GetFlagStatus(USART_RegDef_t *pUSARTx, uint32_t FlagName);
void USART_SetBaudRate(USART_RegDef_t *pUSARTx, uint32_t BaudRate);

void USART_Init(USART_Handle_t *pUSARTHandle);
void USART_DeInit(USART_RegDef_t *pUSARTx);

void USART_SendData(USART_Handle_t *pUSARTHandle, uint8_t *pTxBuffer, uint32_t Len);
void USART_ReceiveData(USART_Handle_t *pUSARTHandle, uint8_t *pRxBuffer, uint32_t Len);

uint8_t USART_SendDataIT(USART_Handle_t *pUSARTHandle, uint8_t *pTxBuffer, uint32_t Len);
uint8_t USART_ReceiveDataIT(USART_Handle_t *pUSARTHandle, uint8_t *pRxBuffer, uint32_t Len);

void USART_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi);
void USART_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority);
void USART_IRQHandling(USART_Handle_t *pHandle);

void USART_ApplicationEventCallback(USART_Handle_t *pUSARTHandle, uint8_t AppEv);

#endif

