#include <stdint.h>

#ifndef __DELAY_H
#define __DELAY_H

void delay_init();
void delay_ms( uint16_t value );
void delay_100us( uint16_t value );
void delay_us( uint16_t value );

#endif /* end __DELAY_H */
