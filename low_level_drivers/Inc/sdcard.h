#ifndef SDCARD_H
#define SDCARD_H

#include <stdint.h>

uint8_t SD_Init(void);
uint8_t SD_ReadBlock(uint32_t block_addr, uint8_t *buffer);
uint8_t SD_WriteBlock(uint32_t block_addr, const uint8_t *buffer);

#endif
