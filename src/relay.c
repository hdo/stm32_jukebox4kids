#include <stdint.h>
#include "stm32.h"
#include "relay.h"
#include "math_utils.h"

void relay_init(void) {
	RCC_AHB1PeriphClockCmd(RELAY_RCC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = (RELAY_CHANNEL_0 | RELAY_CHANNEL_1);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(RELAY_GPIO_PORT, &GPIO_InitStructure);
}

void relay_process(uint32_t msticks) {

}

void relay_reset(uint8_t channel) {
	if (channel == 0) {
		GPIO_SetBits(RELAY_GPIO_PORT, RELAY_CHANNEL_0);
	}
	if (channel == 1) {
		GPIO_SetBits(RELAY_GPIO_PORT, RELAY_CHANNEL_1);
	}
}

void relay_set(uint8_t channel) {
	if (channel == 0) {
		GPIO_ResetBits(RELAY_GPIO_PORT, RELAY_CHANNEL_0);
	}
	if (channel == 1) {
		GPIO_ResetBits(RELAY_GPIO_PORT, RELAY_CHANNEL_1);
	}
}

uint8_t relay_get_status(uint8_t channel) {
	return 0;
}
