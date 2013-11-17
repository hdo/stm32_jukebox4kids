#include <stdint.h>
#include "stm32.h"
#include "buttons.h"
#include "math_utils.h"

//#define DEBOUNCE_CONSTANT 10
#define DEBOUNCE_CONSTANT 5

uint32_t buttons_msticks[] = {0, 0, 0, 0, 0};
uint32_t buttons_diff[] = {0, 0, 0, 0, 0};
uint32_t buttons_oldState = 0;
uint32_t buttons_newState = 0;
uint32_t buttons_inputs[] = {BUTTON0, BUTTON1, BUTTON2, BUTTON3, BUTTON4};
uint8_t buttons_input_count = sizeof(buttons_inputs) / sizeof(uint32_t);


void buttons_init(void) {

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = (BUTTON0 | BUTTON1 | BUTTON2 | BUTTON3 | BUTTON4);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

uint32_t buttons_read_status() {
	// we're are using GPIOC here !!!
	// note that data is inverted (logic 0 -> 1) since we're are using pull-ups

	uint32_t dataHalfWord = GPIO_ReadInputData(GPIOC);
	return ~dataHalfWord & (BUTTON0 | BUTTON1 | BUTTON2 | BUTTON3 | BUTTON4);
}

/**
 * process buttons with DEBOUNCE
 */
void buttons_process(uint32_t msticks) {
	uint8_t i;
	uint32_t d;

	for(i = 0; i < buttons_input_count; i++) {
		if (buttons_msticks[i] != 0) {
			d = math_calc_diff(msticks, buttons_msticks[i]);
			if (d > DEBOUNCE_CONSTANT) {
				buttons_diff[i] = d;
				buttons_msticks[i] = 0;
			}
		}
	}

	buttons_newState = buttons_read_status();
	if (buttons_newState != buttons_oldState) {
		for(i = 0; i < buttons_input_count; i++) {
			if (buttons_newState & buttons_inputs[i]) {
				// 0 to 1 transition
				if ((buttons_oldState & buttons_inputs[i]) == 0) {
					buttons_msticks[i] = msticks;
					buttons_diff[i] = 0;
				}
			} else {
				// 1 to 0 transition
				if (buttons_oldState & buttons_inputs[i]) {
					buttons_msticks[i] = 0;
				}
			}
		}
		buttons_oldState = buttons_newState;
	}
}

uint32_t buttons_triggered(uint8_t index) {
	if (buttons_diff[index] > 1) {
		buttons_diff[index] = 0;
		return 1;
	}
	return 0;
}
