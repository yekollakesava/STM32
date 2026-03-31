#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H

#include <stdint.h>

void SPI1_Init(void);
void SPI1_Transmit(uint8_t data);
uint8_t SPI1_Receive(void);

#endif

