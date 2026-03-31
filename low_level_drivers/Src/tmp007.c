#include "tmp007.h"
#include "stm32l47xx_i2c_driver.h"

void delay_ms(uint32_t ms);

uint16_t TMP007_ReadRegister(uint8_t reg)
{
    uint8_t regAddr = reg;
    uint8_t data[2] = {0};

    I2C_MasterSendData(I2C1, &regAddr, 1, TMP007_I2C_ADDR);
    delay_ms(2);
    I2C_MasterReceiveData(I2C1, data, 2, TMP007_I2C_ADDR);

    return (uint16_t)(((uint16_t)data[0] << 8) | data[1]);
}

void TMP007_WriteRegister(uint8_t reg, uint16_t value)
{
    uint8_t data[3];

    data[0] = reg;
    data[1] = (uint8_t)(value >> 8);
    data[2] = (uint8_t)(value & 0xFF);

    I2C_MasterSendData(I2C1, data, 3, TMP007_I2C_ADDR);
    delay_ms(10);
}

void TMP007_Init(void)
{
    TMP007_WriteRegister(TMP007_REG_CONFIG, 0x7000);
    delay_ms(1000);
}

float TMP007_ConvertRawTemp(uint16_t raw)
{
    int16_t temp_raw = (int16_t)raw;
    temp_raw = temp_raw >> 2;
    return temp_raw * 0.03125f;
}
