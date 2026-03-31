#ifndef SPI_H
#define SPI_H

#include "stm32l476xx.h"
#include <stdint.h>

void SPI1_Init(void);
uint8_t SPI1_Transfer(uint8_t data);

void SD_CS_LOW(void);
void SD_CS_HIGH(void);
void SPI1_SetHighSpeed(void);

#endif
