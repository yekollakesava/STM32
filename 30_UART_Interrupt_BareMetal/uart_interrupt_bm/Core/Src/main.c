#include "stm32l476xx.h"

void UART2_Init(void);
void UART2_SendString_IT(char *str);
void Button_Init(void);

volatile char *txBuffer = 0;
volatile uint32_t txIndex = 0;

volatile uint8_t uartBusy = 0;
volatile uint8_t buttonFlag = 0;

/* ===================== MAIN ===================== */

int main(void)
{
    UART2_Init();
    Button_Init();

    while (1)
    {
        /* Continuous printing */
        if (!uartBusy)
        {
            UART2_SendString_IT("Running...\r\n");

            for (volatile int i = 0; i < 2000000; i++);  // Increase delay here
        }

        /* Button print */
        if (buttonFlag && !uartBusy)
        {
            buttonFlag = 0;
            UART2_SendString_IT("UART Interrupt!\r\n");
        }
    }
}

/* ===================== UART INIT ===================== */

void UART2_Init(void)
{
    /* 1. Enable GPIOA clock */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    /* 2. Set PA2, PA3 to Alternate Function mode */
    GPIOA->MODER &= ~(0xF << (2 * 2));
    GPIOA->MODER |=  (0xA << (2 * 2));   // AF mode

    /* 3. Select AF7 for USART2 */
    GPIOA->AFR[0] &= ~(0xFF << (4 * 2));
    GPIOA->AFR[0] |=  (0x77 << (4 * 2));

    /* 4. Enable USART2 clock */
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;

    /* 5. Disable USART before config */
    USART2->CR1 &= ~USART_CR1_UE;

    /* 6. Baudrate (adjust if clock different) */
    USART2->BRR = 417;   // For 4MHz → 9600 baud

    /* 7. Enable Transmitter */
    USART2->CR1 |= USART_CR1_TE;

    /* 8. Enable USART */
    USART2->CR1 |= USART_CR1_UE;

    /* 9. Enable NVIC Interrupt */
    NVIC_EnableIRQ(USART2_IRQn);
}

/* ===================== UART SEND ===================== */

void UART2_SendString_IT(char *str)
{
    if (uartBusy) return;

    uartBusy = 1;

    txBuffer = str;
    txIndex = 0;

    USART2->CR1 |= USART_CR1_TXEIE;
}

/* ===================== UART ISR ===================== */

void USART2_IRQHandler(void)
{
    if ((USART2->ISR & USART_ISR_TXE) &&
        (USART2->CR1 & USART_CR1_TXEIE))
    {
        if (txBuffer[txIndex] != '\0')
        {
            USART2->TDR = txBuffer[txIndex++];
        }
        else
        {
            USART2->CR1 &= ~USART_CR1_TXEIE;
            uartBusy = 0;
        }
    }
}

/* ===================== BUTTON INIT ===================== */

void Button_Init(void)
{
    /* Enable GPIOC clock */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;

    /* PC13 input mode */
    GPIOC->MODER &= ~(3 << (13 * 2));

    /* Enable SYSCFG clock */
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    /* Connect EXTI13 to PC13 */
    SYSCFG->EXTICR[3] &= ~(0xF << 4);
    SYSCFG->EXTICR[3] |=  (0x2 << 4);

    /* Enable EXTI line 13 */
    EXTI->IMR1  |= (1 << 13);
    EXTI->FTSR1 |= (1 << 13);   // Falling edge

    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/* ===================== BUTTON ISR ===================== */

void EXTI15_10_IRQHandler(void)
{
    if (EXTI->PR1 & (1 << 13))
    {
        EXTI->PR1 |= (1 << 13);   // Clear interrupt flag
        buttonFlag = 1;
    }
}
