#include "i2c_driver.h"

extern I2C_HandleTypeDef hi2c1;

HAL_StatusTypeDef I2C_Write_Byte(uint16_t devAddr, uint8_t regAddr, uint8_t data)
{
    return HAL_I2C_Mem_Write(&hi2c1,
                             devAddr,
                             regAddr,
                             I2C_MEMADD_SIZE_8BIT,
                             &data,
                             1,
                             HAL_MAX_DELAY);
}

HAL_StatusTypeDef I2C_Read_Byte(uint16_t devAddr, uint8_t regAddr, uint8_t *data)
{
    return HAL_I2C_Mem_Read(&hi2c1,
                            devAddr,
                            regAddr,
                            I2C_MEMADD_SIZE_8BIT,
                            data,
                            1,
                            HAL_MAX_DELAY);
}
