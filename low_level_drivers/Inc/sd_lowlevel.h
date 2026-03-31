/*
 * sd_lowlevel.h
 *
 *  Created on: Mar 25, 2026
 *      Author: yekol
 */
#ifndef SD_LOWLEVEL_H_
#define SD_LOWLEVEL_H_

#include <stdint.h>

void SD_CS_LOW(void);
void SD_CS_HIGH(void);
uint8_t SPI1_TxRx_Byte(uint8_t data);

#endif
