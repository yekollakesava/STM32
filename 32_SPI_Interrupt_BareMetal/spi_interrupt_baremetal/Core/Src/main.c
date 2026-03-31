#include "stm32l476xx.h"
#include <stdio.h>

volatile uint8_t tx_data = 0x55;
volatile uint8_t rx_data = 0;
volatile uint8_t transfer_done = 0;

/* ================= DELAY ================= */
void delay(volatile uint32_t t)
{
    while(t--);
}

/* ================= UART ================= */

void UART2_Init(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;

    GPIOA->MODER &= ~(GPIO_MODER_MODE2_Msk | GPIO_MODER_MODE3_Msk);
    GPIOA->MODER |= (2 << GPIO_MODER_MODE2_Pos) |
                    (2 << GPIO_MODER_MODE3_Pos);

    GPIOA->AFR[0] |= (7 << 8) | (7 << 12);

    USART2->BRR = SystemCoreClock / 9600;

    USART2->CR1 |= USART_CR1_TE;
    USART2->CR1 |= USART_CR1_UE;
}

void UART_SendChar(char c)
{
    while(!(USART2->ISR & USART_ISR_TXE));
    USART2->TDR = c;
}

void UART_Print(char *str)
{
    while(*str)
        UART_SendChar(*str++);
}

void UART_PrintHex(uint8_t val)
{
    char buf[6];
    sprintf(buf,"0x%02X ",val);
    UART_Print(buf);
}

/* ================= SPI ================= */

void SPI1_Init(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    /* PA5=SCK, PA6=MISO, PA7=MOSI */
    GPIOA->MODER &= ~(GPIO_MODER_MODE5_Msk |
                      GPIO_MODER_MODE6_Msk |
                      GPIO_MODER_MODE7_Msk);

    GPIOA->MODER |= (2 << GPIO_MODER_MODE5_Pos) |
                    (2 << GPIO_MODER_MODE6_Pos) |
                    (2 << GPIO_MODER_MODE7_Pos);

    GPIOA->AFR[0] |= (5 << 20) | (5 << 24) | (5 << 28);

    SPI1->CR1 = 0;
    SPI1->CR1 |= SPI_CR1_MSTR;
    SPI1->CR1 |= SPI_CR1_SSM;
    SPI1->CR1 |= SPI_CR1_SSI;
    SPI1->CR1 |= (3 << SPI_CR1_BR_Pos);

    SPI1->CR2 |= SPI_CR2_RXNEIE;  // Enable RX interrupt
    SPI1->CR2 |= SPI_CR2_TXEIE;   // Enable TX interrupt

    NVIC_EnableIRQ(SPI1_IRQn);

    SPI1->CR1 |= SPI_CR1_SPE;
}

/* ================= SPI INTERRUPT HANDLER ================= */

void SPI1_IRQHandler(void)
{
    /* TXE interrupt */
    if((SPI1->SR & SPI_SR_TXE) && (SPI1->CR2 & SPI_CR2_TXEIE))
    {
        SPI1->DR = tx_data;
        SPI1->CR2 &= ~SPI_CR2_TXEIE;   // Disable TX interrupt
    }

    /* RXNE interrupt */
    if((SPI1->SR & SPI_SR_RXNE) && (SPI1->CR2 & SPI_CR2_RXNEIE))
    {
        rx_data = SPI1->DR;
        transfer_done = 1;
    }
}

/* ================= MAIN ================= */

int main(void)
{
    UART2_Init();
    SPI1_Init();

    UART_Print("SPI Interrupt BareMetal \r\n");

    while(1)
    {
        transfer_done = 0;

        SPI1->CR2 |= SPI_CR2_TXEIE;   // Start transfer

        while(!transfer_done);

        UART_Print("TX: ");
        UART_PrintHex(tx_data);
        UART_Print(" RX: ");
        UART_PrintHex(rx_data);
        UART_Print("\r\n");

        delay(1000000);
    }
}
