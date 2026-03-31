#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#include <stdint.h>

void I2C1_Init(void);
void I2C1_WriteByte(uint8_t slave_addr, uint8_t data);

#endif
