#include "stm32.h"
#ifndef __RELAY_H
#define __RELAY_H

#define RELAY_RCC RCC_AHB1Periph_GPIOH
#define RELAY_GPIO_PORT GPIOH
#define RELAY_CHANNEL_0 0
#define RELAY_CHANNEL_1 1

void relay_init(void);
void relay_process(uint32_t msticks);
void relay_set(uint8_t channel);
void relay_reset(uint8_t channel);
uint8_t relay_get_status(uint8_t channel);

#endif /* end __RELAY_H */
