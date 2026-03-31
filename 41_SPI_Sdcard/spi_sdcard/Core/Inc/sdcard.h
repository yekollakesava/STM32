#ifndef SDCARD_H
#define SDCARD_H

#include "stm32f4xx.h"

uint8_t SD_Init(void);
uint8_t SD_WriteBlock(uint32_t block, uint8_t *data);
uint8_t SD_ReadBlock(uint32_t block, uint8_t *data);

#endif
