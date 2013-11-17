#include <stdint.h>
#include "stm32.h"
#include "rotary.h"

uint16_t rotary_begin = ROTARY_DEFAULT_BEGIN;
uint16_t rotary_end = ROTARY_DEFAULT_END;
uint16_t rotary_value = ROTARY_DEFAULT_BEGIN;


void rotary_init() {
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_PinAFConfig (GPIOB, GPIO_PinSource4, GPIO_AF_TIM3);
	GPIO_PinAFConfig (GPIOB, GPIO_PinSource5, GPIO_AF_TIM3);

	TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Falling);
	TIM_Cmd(TIM3, ENABLE);
}


void revalidate_value() {
	if (TIM_GetCounter(TIM3) < ROTARY_BASE_VALUE) {
		TIM_SetCounter(TIM3, ROTARY_BASE_VALUE);
	}
	uint16_t temp = (TIM_GetCounter(TIM3) - ROTARY_BASE_VALUE) / ROTARY_DEFAULT_DIVIDER;
	if (temp < rotary_begin) {
		temp = rotary_begin;
		TIM_SetCounter(TIM3, (temp * ROTARY_DEFAULT_DIVIDER) + ROTARY_BASE_VALUE);
	}
	else if (temp > rotary_end) {
		temp = rotary_end;
		TIM_SetCounter(TIM3, (temp * ROTARY_DEFAULT_DIVIDER) + ROTARY_BASE_VALUE);
	}
	rotary_value = temp;
}

void rotary_set_boundary(uint16_t begin, uint16_t end) {
	rotary_begin = begin;
	rotary_end = end;
}

void rotary_set_value(uint16_t value) {
	uint16_t newValue = value;
	if (value < rotary_begin) {
		newValue = rotary_begin;
	} else 	if (value > rotary_end) {
		newValue = rotary_end;
	}
	if (newValue >= rotary_begin && newValue <= rotary_end) {
		TIM_SetCounter(TIM3, (newValue * ROTARY_DEFAULT_DIVIDER) + ROTARY_BASE_VALUE);
		revalidate_value();
	}
}

uint16_t rotary_get_value() {
	revalidate_value();
	return rotary_value;
}
