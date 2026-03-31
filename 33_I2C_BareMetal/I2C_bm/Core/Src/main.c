#include "stm32l476xx.h"

void Clock_4MHz_Init(void);
void I2C1_Master_Init(void);
void I2C2_Slave_Init(void);
void I2C1_Master_Write(uint8_t addr, uint8_t data);

volatile uint8_t slave_received = 0;

int main(void)
{
    Clock_4MHz_Init();
    I2C1_Master_Init();
    I2C2_Slave_Init();

    while(1)
    {
        I2C1_Master_Write(0x30, 0x41);
        for(volatile int i=0;i<500000;i++);
    }
}

/* ---------- FORCE 4 MHz MSI CLOCK ---------- */
void Clock_4MHz_Init(void)
{
    RCC->CR |= RCC_CR_MSION;
    while(!(RCC->CR & RCC_CR_MSIRDY));

    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_MSI;
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_MSI);

    RCC->CFGR &= ~RCC_CFGR_PPRE1;  // APB1 prescaler = 1
}

/* ---------- I2C1 MASTER ---------- */
void I2C1_Master_Init(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN;

    /* PB8, PB9 AF4 */
    GPIOB->MODER &= ~((3<<(8*2)) | (3<<(9*2)));
    GPIOB->MODER |=  ((2<<(8*2)) | (2<<(9*2)));

    GPIOB->OTYPER |= (1<<8) | (1<<9);

    GPIOB->AFR[1] &= ~((0xF<<0) | (0xF<<4));
    GPIOB->AFR[1] |=  ((4<<0) | (4<<4));

    I2C1->CR1 &= ~I2C_CR1_PE;

    I2C1->TIMINGR = 0x00303D5B;  // 100kHz @ 4MHz

    I2C1->CR1 |= I2C_CR1_PE;
}

/* ---------- I2C2 SLAVE ---------- */
void I2C2_Slave_Init(void)
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_I2C2EN;

    /* PB10, PB11 AF4 */
    GPIOB->MODER &= ~((3<<(10*2)) | (3<<(11*2)));
    GPIOB->MODER |=  ((2<<(10*2)) | (2<<(11*2)));

    GPIOB->OTYPER |= (1<<10) | (1<<11);

    GPIOB->AFR[1] &= ~((0xF<<8) | (0xF<<12));
    GPIOB->AFR[1] |=  ((4<<8) | (4<<12));

    I2C2->CR1 &= ~I2C_CR1_PE;

    I2C2->OAR1 = (0x30 << 1);
    I2C2->OAR1 |= I2C_OAR1_OA1EN;

    I2C2->CR1 |= I2C_CR1_RXIE;
    I2C2->CR1 |= I2C_CR1_PE;

    NVIC_EnableIRQ(I2C2_EV_IRQn);
}

/* ---------- MASTER WRITE ---------- */
void I2C1_Master_Write(uint8_t addr, uint8_t data)
{
    I2C1->CR2 = 0;
    I2C1->CR2 |= (addr << 1);
    I2C1->CR2 |= (1 << I2C_CR2_NBYTES_Pos);
    I2C1->CR2 |= I2C_CR2_START;

    while(!(I2C1->ISR & I2C_ISR_TXIS));

    I2C1->TXDR = data;

    while(!(I2C1->ISR & I2C_ISR_TC));

    I2C1->CR2 |= I2C_CR2_STOP;
}

/* ---------- SLAVE INTERRUPT ---------- */
void I2C2_EV_IRQHandler(void)
{
    if(I2C2->ISR & I2C_ISR_RXNE)
    {
        slave_received = I2C2->RXDR;
    }
}
