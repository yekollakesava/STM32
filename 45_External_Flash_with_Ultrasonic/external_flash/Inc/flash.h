#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>

void flash_init(void);
void flash_write(uint32_t addr, uint8_t data);

#endif
