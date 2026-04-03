#include "stm32l476xx.h"
#include <stdio.h>

void UART2_Init(void);
void UART2_SendString_IT(char *str);
void ADC1_Init(void);
uint16_t ADC1_Read(void);

volatile char *txBuffer = 0;
volatile uint32_t txIndex = 0;
volatile uint8_t uartBusy = 0;

char msg[50];

/* ================= MAIN ================= */

int main(void)
{
    UART2_Init();
    ADC1_Init();

    while (1)
    {
        uint16_t adcValue = ADC1_Read();

        if (!uartBusy)
        {
            sprintf(msg, "ADC Value: %d\r\n", adcValue);
            UART2_SendString_IT(msg);
        }

        for (volatile int i = 0; i < 3000000; i++);
    }
}

/* ================= UART ================= */

void UART2_Init(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;

    /* PA2, PA3 Alternate Function */
    GPIOA->MODER &= ~(0xF << (2 * 2));
    GPIOA->MODER |=  (0xA << (2 * 2));

    GPIOA->AFR[0] &= ~(0xFF << (4 * 2));
    GPIOA->AFR[0] |=  (0x77 << (4 * 2));

    USART2->CR1 &= ~USART_CR1_UE;

    /* 🔴 IMPORTANT: Adjust if clock is 16 MHz */
    USART2->BRR = 1667;   // 16MHz / 9600

    USART2->CR1 |= USART_CR1_TE;
    USART2->CR1 |= USART_CR1_UE;

    NVIC_EnableIRQ(USART2_IRQn);
}

void UART2_SendString_IT(char *str)
{
    if (uartBusy) return;

    uartBusy = 1;
    txBuffer = str;
    txIndex = 0;

    USART2->CR1 |= USART_CR1_TXEIE;
}

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

/* ================= ADC ================= */

void ADC1_Init(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN;

    /* 🔴 IMPORTANT: Select SYSCLK as ADC clock */
    RCC->CCIPR &= ~(3 << 28);   // 00 = SYSCLK

    /* PA0 Analog mode */
    GPIOA->MODER |= (3 << (0 * 2));

    /* Exit deep power down */
    ADC1->CR &= ~ADC_CR_DEEPPWD;

    /* Enable regulator */
    ADC1->CR |= ADC_CR_ADVREGEN;
    for (volatile int i = 0; i < 20000; i++);

    /* Calibration */
    ADC1->CR &= ~ADC_CR_ADEN;
    ADC1->CR |= ADC_CR_ADCAL;
    while (ADC1->CR & ADC_CR_ADCAL);

    /* Enable ADC */
    ADC1->ISR |= ADC_ISR_ADRDY;
    ADC1->CR |= ADC_CR_ADEN;
    while (!(ADC1->ISR & ADC_ISR_ADRDY));

    /* Channel 5 (PA0) */
    ADC1->SQR1 &= ~(0x1F << 6);
    ADC1->SQR1 |= (5 << 6);
}

uint16_t ADC1_Read(void)
{
    ADC1->CR |= ADC_CR_ADSTART;

    while (!(ADC1->ISR & ADC_ISR_EOC));

    return ADC1->DR;
}
