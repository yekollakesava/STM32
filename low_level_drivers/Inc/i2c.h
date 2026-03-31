#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>

void I2C1_GPIO_Init(void);
void I2C1_Init(void);

uint8_t I2C_ReadReg(uint8_t devAddr, uint8_t regAddr);
void I2C_WriteReg(uint8_t devAddr, uint8_t regAddr, uint8_t data);

#endif
