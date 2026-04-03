#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdint.h>

void ultrasonic_trigger(void);
uint32_t ultrasonic_read(void);
float ultrasonic_distance(void);

#endif
