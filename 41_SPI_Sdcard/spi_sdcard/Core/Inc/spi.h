#ifndef SPI_H
#define SPI_H

#include "stm32f4xx.h"

void SPI1_Init(void);
void SPI1_SetSpeedLow(void);
void SPI1_SetSpeedHigh(void);
uint8_t SPI1_Transfer(uint8_t data);

#endif
