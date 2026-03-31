#ifndef SDCARD_H
#define SDCARD_H

#include <stdint.h>

uint8_t SD_Init(void);
uint8_t SD_WriteBlock(uint32_t sector, uint8_t *buffer);
uint8_t SD_ReadBlock(uint32_t sector, uint8_t *buffer);
void SD_SendClockTrain(void);

#endif
