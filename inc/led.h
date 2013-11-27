#include "stm32.h"
#ifndef __LED_H
#define __LED_H

#define LED_RCC RCC_AHB1Periph_GPIOD
#define LED_GPIO_PORT GPIOD
#define LED_CHANNEL_0_PIN GPIO_Pin_12 // onboard green
#define LED_CHANNEL_1_PIN GPIO_Pin_13 // onboard orange
#define LED_CHANNEL_2_PIN GPIO_Pin_14 // onboard red
#define LED_CHANNEL_3_PIN GPIO_Pin_15 // onboard blue
#define LED_CHANNEL_4_PIN GPIO_Pin_0 // red
#define LED_CHANNEL_5_PIN GPIO_Pin_1 // green
#define LED_CHANNEL_6_PIN GPIO_Pin_2 // blue

#define LED_OB_GREEN  0 // onboard green
#define LED_OB_ORANGE 1 // onboard orange
#define LED_OB_RED    2 // onboard red
#define LED_OB_BLUE   3 // onboard blue
#define LED_RED       4 // led red
#define LED_GREEN     5 // led green
#define LED_BLUE      6 // blue

void led_init();
void led_process(uint32_t msticks);
void led_on(uint8_t channel);
void led_off(uint8_t channel);
void led_signal(uint8_t channel, uint32_t timeout);

#endif /* end __LED_H */
