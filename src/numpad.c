#include <stdint.h>
#include "stm32.h"
#include "numpad.h"
#include "math_utils.h"

//#define DEBOUNCE_CONSTANT 10
#define DEBOUNCE_CONSTANT 5

uint32_t numpad_msticks[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint32_t numpad_diff[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint32_t numpad_oldState = 0;
uint32_t numpad_newState = 0;

// virtual GPIO pins since we're using numpad here
uint32_t numpad_inputs[] = {
		GPIO_Pin_0,
		GPIO_Pin_1,
		GPIO_Pin_2,
		GPIO_Pin_3,
		GPIO_Pin_4,
		GPIO_Pin_5,
		GPIO_Pin_6,
		GPIO_Pin_7,
		GPIO_Pin_8,
		GPIO_Pin_9,
		GPIO_Pin_10,
		GPIO_Pin_11,
		GPIO_Pin_12,
		GPIO_Pin_13,
		GPIO_Pin_14,
		GPIO_Pin_15};

uint8_t numpad_input_count = sizeof(numpad_inputs) / sizeof(uint32_t);


void numpad_init(void) {


	RCC_AHB1PeriphClockCmd(NUMPAD_RCC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;

	/*
	 * set pin 1-4 as input
	 */
	GPIO_InitStructure.GPIO_Pin = (NUMPAD_PIN_1 | NUMPAD_PIN_2 | NUMPAD_PIN_3 | NUMPAD_PIN_4);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(NUMPAD_GPIO_PORT, &GPIO_InitStructure);

	/*
	 * set pin a-d as output
	 */
	GPIO_InitStructure.GPIO_Pin = (NUMPAD_PIN_A | NUMPAD_PIN_B | NUMPAD_PIN_C | NUMPAD_PIN_D);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(NUMPAD_GPIO_PORT, &GPIO_InitStructure);

	// set 1 for a-d
	GPIO_SetBits(NUMPAD_GPIO_PORT, (NUMPAD_PIN_A | NUMPAD_PIN_B | NUMPAD_PIN_C | NUMPAD_PIN_D));
}

uint16_t read_status_for(uint16_t pin) {
	GPIO_SetBits(NUMPAD_GPIO_PORT, (NUMPAD_PIN_A | NUMPAD_PIN_B | NUMPAD_PIN_C | NUMPAD_PIN_D));
	GPIO_ResetBits(NUMPAD_GPIO_PORT, pin);
	uint32_t pinstatus = GPIO_ReadInputData(NUMPAD_GPIO_PORT);
	uint16_t ret = 0;

	// note that data is inverted (logic 0 -> 1) since we're are using pull-ups
	if (!(pinstatus & NUMPAD_PIN_1)) {
		ret = ret | (1 << 0);
	}
	if (!(pinstatus & NUMPAD_PIN_2)) {
		ret = ret | (1 << 1);
	}
	if (!(pinstatus & NUMPAD_PIN_3)) {
		ret = ret | (1 << 2);
	}
	if (!(pinstatus & NUMPAD_PIN_4)) {
		ret = ret | (1 << 3);
	}
	return ret;
}

uint32_t numpad_read_status() {
	// we're are using GPIOC here !!!

	uint16_t status_a = read_status_for(NUMPAD_PIN_A);
	uint16_t status_b = read_status_for(NUMPAD_PIN_B);
	uint16_t status_c = read_status_for(NUMPAD_PIN_C);
	uint16_t status_d = read_status_for(NUMPAD_PIN_D);

	uint32_t ret = (status_a << 0) | (status_b << 4) | (status_c << 8) | (status_d << 16);
	return ret;
}

/**
 * process numpad with DEBOUNCE
 */
void numpad_process(uint32_t msticks) {
	uint8_t i;
	uint32_t d;

	for(i = 0; i < numpad_input_count; i++) {
		if (numpad_msticks[i] != 0) {
			d = math_calc_diff(msticks, numpad_msticks[i]);
			if (d > DEBOUNCE_CONSTANT) {
				numpad_diff[i] = d;
				numpad_msticks[i] = 0;
			}
		}
	}

	numpad_newState = numpad_read_status();
	if (numpad_newState != numpad_oldState) {
		for(i = 0; i < numpad_input_count; i++) {
			if (numpad_newState & numpad_inputs[i]) {
				// 0 to 1 transition
				if ((numpad_oldState & numpad_inputs[i]) == 0) {
					numpad_msticks[i] = msticks;
					numpad_diff[i] = 0;
				}
			} else {
				// 1 to 0 transition
				if (numpad_oldState & numpad_inputs[i]) {
					numpad_msticks[i] = 0;
				}
			}
		}
		numpad_oldState = numpad_newState;
	}
}

uint32_t numpad_triggered(uint8_t index) {
	if (numpad_diff[index] > 1) {
		numpad_diff[index] = 0;
		return 1;
	}
	return 0;
}
