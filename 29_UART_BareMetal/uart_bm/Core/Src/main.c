#include "stm32l476xx.h"

void UART2_Init(void);
void UART2_Transmit(char ch);
void UART2_SendString(char *str);
void delay(void);

int main(void)
{
    UART2_Init();

    while(1)
    {
        UART2_SendString("Hello from STM32 UART BareMetal\r\n");
        delay();
    }
}

/* Simple delay */
void delay(void)
{
    for(volatile int i = 0; i < 300000; i++);
}

/* ---------------- UART2 Initialization ---------------- */
void UART2_Init(void)
{
    /* 1. Enable MSI (4 MHz default) */
    RCC->CR |= RCC_CR_MSION;
    while(!(RCC->CR & RCC_CR_MSIRDY));

    /* 2. Select MSI as system clock */
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_MSI;
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_MSI);

    /* 3. Enable Clocks */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;

    /* 4. Configure PA2 (TX) and PA3 (RX) as Alternate Function */
    GPIOA->MODER &= ~(3 << (2*2));
    GPIOA->MODER |=  (2 << (2*2));

    GPIOA->MODER &= ~(3 << (3*2));
    GPIOA->MODER |=  (2 << (3*2));

    /* 5. Set AF7 (USART2) */
    GPIOA->AFR[0] &= ~(0xF << (4*2));
    GPIOA->AFR[0] |=  (7 << (4*2));

    GPIOA->AFR[0] &= ~(0xF << (4*3));
    GPIOA->AFR[0] |=  (7 << (4*3));

    /* 6. Set Baud Rate
       MSI = 4 MHz
       4000000 / 9600 = 417
    */
    USART2->BRR = 80000000 / 9600;

    /* 7. Enable TX and RX */
    USART2->CR1 |= USART_CR1_TE;
    USART2->CR1 |= USART_CR1_RE;

    /* 8. Enable USART */
    USART2->CR1 |= USART_CR1_UE;
}

/* ---------------- Transmit Character ---------------- */
void UART2_Transmit(char ch)
{
    while(!(USART2->ISR & USART_ISR_TXE));
    USART2->TDR = ch;

    while(!(USART2->ISR & USART_ISR_TC));
}

/* ---------------- Send String ---------------- */
void UART2_SendString(char *str)
{
    while(*str)
    {
        UART2_Transmit(*str++);
    }
}
