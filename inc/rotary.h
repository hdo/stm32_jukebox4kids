#include <stdint.h>

#ifndef __ROTARY_H
#define __ROTARY_H

#define ROTARY_DEFAULT_DIVIDER 1
#define ROTARY_DEFAULT_MULTIPLIER 2
#define ROTARY_BASE_VALUE 1000
#define ROTARY_DEFAULT_BEGIN 0
#define ROTARY_DEFAULT_END 100

void rotary_init();
void rotary_set_boundary(uint16_t begin, uint16_t end);
void rotary_set_value(uint16_t value);
uint16_t rotary_get_value();

#endif /* end __ROTARY_H */
