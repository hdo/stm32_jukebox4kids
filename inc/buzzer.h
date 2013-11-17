#include "stm32.h"
#ifndef __BUZZER_H
#define __BUZZER_H

#define BUZZER_RCC RCC_AHB1Periph_GPIOB
#define BUZZER_GPIO_PORT GPIOB
#define BUZZER_PIN 9

void buzzer_init(void);
void buzzer_process(uint32_t msticks);

#endif /* end __BUZZER_H */
