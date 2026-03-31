#include "stm32l47xx.h"
#include "stm32l47xx_i2c_driver.h"

I2C_Handle_t I2C1Handle;
uint8_t txData = 0xAA;
volatile uint8_t txDone = 1;

static void delay(void)
{
    for(volatile uint32_t i = 0; i < 500000; i++);
}

int main(void)
{
    I2C_GPIO_Init();

    I2C1Handle.pI2Cx = I2C1;
    I2C1Handle.Timing = 0x10909CEC;
    I2C1Handle.OwnAddress1 = 0x00;

    I2C_Init(&I2C1Handle);

    I2C_IRQInterruptConfig(IRQ_NO_I2C1_EV, ENABLE);
    I2C_IRQInterruptConfig(IRQ_NO_I2C1_ER, ENABLE);

    I2C_IRQPriorityConfig(IRQ_NO_I2C1_EV, 2);
    I2C_IRQPriorityConfig(IRQ_NO_I2C1_ER, 1);

    while(1)
    {
        if(txDone)
        {
            txDone = 0;
            I2C_MasterSendDataIT(&I2C1Handle, &txData, 1, 0x20);
        }

        delay();
    }
}

void I2C1_EV_IRQHandler(void)
{
    I2C_EV_IRQHandling(&I2C1Handle);
}

void I2C1_ER_IRQHandler(void)
{
    I2C_ER_IRQHandling(&I2C1Handle);
}

void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle, uint8_t AppEv)
{
    (void)pI2CHandle;

    if(AppEv == I2C_EV_TX_CMPLT)
    {
        txDone = 1;
    }
    else
    {
        txDone = 1;
    }
}
