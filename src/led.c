#include <stdint.h>
#include "stm32.h"
#include "led.h"
#include "math_utils.h"

uint16_t led_map[] = {LED_CHANNEL_0_PIN, LED_CHANNEL_1_PIN, LED_CHANNEL_2_PIN,
		LED_CHANNEL_3_PIN, LED_CHANNEL_4_PIN, LED_CHANNEL_5_PIN, LED_CHANNEL_6_PIN
};

uint32_t led_current_msticks = 0;
uint32_t led_msticks[] = {0, 0, 0, 0, 0, 0, 0, 0};
uint32_t led_timeout[] = {0, 0, 0, 0, 0, 0, 0, 0};


void led_init(void) {
	RCC_AHB1PeriphClockCmd(LED_RCC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = (LED_CHANNEL_0_PIN | LED_CHANNEL_1_PIN | LED_CHANNEL_2_PIN
			| LED_CHANNEL_3_PIN | LED_CHANNEL_4_PIN
			| LED_CHANNEL_5_PIN | LED_CHANNEL_6_PIN);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(LED_GPIO_PORT, &GPIO_InitStructure);

	// clear all
	GPIO_SetBits(LED_GPIO_PORT, (LED_CHANNEL_0_PIN | LED_CHANNEL_1_PIN | LED_CHANNEL_2_PIN
			| LED_CHANNEL_3_PIN | LED_CHANNEL_4_PIN
			| LED_CHANNEL_5_PIN | LED_CHANNEL_6_PIN));
}


void led_on(uint8_t channel) {
	if (channel < sizeof(led_map)) {
		GPIO_ResetBits(LED_GPIO_PORT, led_map[channel]);
	}
}

void led_off(uint8_t channel) {
	if (channel < sizeof(led_map)) {
		GPIO_SetBits(LED_GPIO_PORT, led_map[channel]);
	}
}

void led_signal(uint8_t channel, uint32_t timeout) {
	if (channel < sizeof(led_map)) {
		led_timeout[channel] = timeout;
		led_msticks[channel] = led_current_msticks;
		led_on(channel);
	}
}

void led_process(uint32_t msticks) {
	led_current_msticks = msticks;
	uint8_t i;
	for(i=0; i < sizeof(led_map); i++) {
		if (led_timeout[i] > 0) {
			if (math_calc_diff(msticks, led_msticks[i]) > led_timeout[i]) {
				led_timeout[i] = 0;
				led_msticks[i] = 0;
				led_off(i);
			}
		}
	}
}
