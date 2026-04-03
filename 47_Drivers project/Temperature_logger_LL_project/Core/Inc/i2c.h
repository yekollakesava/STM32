#ifndef I2C_H
#define I2C_H

#include <stdint.h>

void I2C1_Init(void);
uint8_t I2C_ReadReg(uint8_t dev, uint8_t reg);
void I2C_WriteReg(uint8_t dev, uint8_t reg, uint8_t data);

#endif
