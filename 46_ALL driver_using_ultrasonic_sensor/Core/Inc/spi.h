#ifndef SPI_H
#define SPI_H

#include "main.h"
#include <stdint.h>

void SPI1_Init(void);
uint8_t SPI1_Transfer(uint8_t data);
void SD_CS_High(void);
void SD_CS_Low(void);

#endif
