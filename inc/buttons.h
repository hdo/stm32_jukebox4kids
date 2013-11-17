#include "stm32.h"
#ifndef __BUTTONS_H
#define __BUTTONS_H

#define BUTTON0 GPIO_Pin_1 // ON/OFF
#define BUTTON1 GPIO_Pin_2 // PREV
#define BUTTON2 GPIO_Pin_3 // PLAY/PAUSE
#define BUTTON3 GPIO_Pin_4 // NEXT
#define BUTTON4 GPIO_Pin_5 // VOL_BUTTON

#define BUTTON_ON_OFF 0
#define BUTTON_PREV 1
#define BUTTON_PLAY_PAUSE 2
#define BUTTON_NEXT 3
#define BUTTON_VOLUME 4

void buttons_init(void);
void buttons_process(uint32_t msticks);
uint32_t buttons_triggered(uint8_t index);


#endif /* end __BUTTONS_H */
