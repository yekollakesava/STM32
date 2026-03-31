#ifndef SDCARD_H_
#define SDCARD_H_

#include <stdint.h>

uint8_t SD_Init(void);
uint8_t SD_ReadBlock(uint32_t sector, uint8_t *buffer);
uint8_t SD_WriteBlock(uint32_t sector, uint8_t *buffer);

extern uint8_t SDHC_FLAG;

#endif
