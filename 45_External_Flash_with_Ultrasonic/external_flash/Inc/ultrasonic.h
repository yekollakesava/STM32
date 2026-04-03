#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdint.h>

void ultrasonic_init(void);
void ultrasonic_trigger(void);
uint32_t ultrasonic_read(void);
uint8_t ultrasonic_distance(void);

#endif
