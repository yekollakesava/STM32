#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#include "main.h"

HAL_StatusTypeDef I2C_Write_Byte(uint16_t devAddr, uint8_t regAddr, uint8_t data);
HAL_StatusTypeDef I2C_Read_Byte(uint16_t devAddr, uint8_t regAddr, uint8_t *data);

#endif
