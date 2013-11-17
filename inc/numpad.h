#include "stm32.h"
#ifndef __NUMPAD_H
#define __NUMPAD_H

#define NUMPAD_GPIO_PORT GPIOE
#define NUMPAD_RCC RCC_AHB1Periph_GPIOE
#define NUMPAD_PIN_1 GPIO_Pin_7
#define NUMPAD_PIN_2 GPIO_Pin_8
#define NUMPAD_PIN_3 GPIO_Pin_9
#define NUMPAD_PIN_4 GPIO_Pin_10
#define NUMPAD_PIN_A GPIO_Pin_11
#define NUMPAD_PIN_B GPIO_Pin_12
#define NUMPAD_PIN_C GPIO_Pin_13
#define NUMPAD_PIN_D GPIO_Pin_14

#define NUMPAD_0 0
#define NUMPAD_1 1
#define NUMPAD_2 2
#define NUMPAD_3 3
#define NUMPAD_4 4
#define NUMPAD_5 5
#define NUMPAD_6 6
#define NUMPAD_7 7
#define NUMPAD_8 8
#define NUMPAD_9 9
#define NUMPAD_A 10
#define NUMPAD_B 11
#define NUMPAD_C 12
#define NUMPAD_D 13
#define NUMPAD_STAR 14
#define NUMPAD_POUND 15

void numpad_init(void);
void numpad_process(uint32_t msticks);
uint32_t numpad_triggered(uint8_t index);

#endif /* end __NUMPAD_H */
