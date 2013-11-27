#include <stdint.h>
#include "stm32.h"
#include "relay.h"
#include "math_utils.h"

uint16_t relay_map[] = { RELAY_CHANNEL_0_PIN, RELAY_CHANNEL_1_PIN };

void relay_init(void) {
	RCC_AHB1PeriphClockCmd(RELAY_RCC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = (RELAY_CHANNEL_0_PIN | RELAY_CHANNEL_1_PIN);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(RELAY_GPIO_PORT, &GPIO_InitStructure);

	GPIO_SetBits(RELAY_GPIO_PORT, RELAY_CHANNEL_0_PIN | RELAY_CHANNEL_1_PIN);
}

void relay_process(uint32_t msticks) {

}

void relay_reset(uint8_t channel) {
	if (channel < sizeof(relay_map)) {
		GPIO_SetBits(RELAY_GPIO_PORT, relay_map[channel]);
	}
}

void relay_set(uint8_t channel) {
	if (channel < sizeof(relay_map)) {
		GPIO_ResetBits(RELAY_GPIO_PORT, relay_map[channel]);
	}
}

uint8_t relay_get_status(uint8_t channel) {
	return 0;
}
