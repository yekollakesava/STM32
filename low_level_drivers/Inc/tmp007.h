#ifndef INC_TMP007_H_
#define INC_TMP007_H_

#include "stm32l47xx.h"
#include <stdint.h>

#define TMP007_I2C_ADDR        0x80
#define TMP007_REG_CONFIG      0x02
#define TMP007_REG_DIE_TEMP    0x01
#define TMP007_REG_OBJ_TEMP    0x03
#define TMP007_REG_DEVICE_ID   0x1F

void     TMP007_Init(void);
uint16_t TMP007_ReadRegister(uint8_t reg);
void     TMP007_WriteRegister(uint8_t reg, uint16_t value);
float    TMP007_ConvertRawTemp(uint16_t raw);

#endif
