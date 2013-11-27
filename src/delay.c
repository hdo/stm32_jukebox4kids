#include <stdint.h>
#include "stm32.h"
#include "delay.h"

#define APB1_PRESCALER 4            /* see system_stm32f4x  */ //  RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;
#define DELAY_TIM_FREQUENCY 10000   /* = 10KHZ -> timer runs in 100 microseconds */
#define DELAY_TIM2_FREQUENCY 1000000   /* = 1MHZ -> timer runs in 1 microseconds */

void delay_init() {
	/* Enable timer clock  - use TIMER6 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

	/* Time base configuration */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);

	/*
	 * If APB1 prescaler is not 1 then clock of timer is 2xAPB1 clock
	 * e.g. HCLK=168 Mhz, APB1_CLK=42 Mhz, TIM_CLK=84 Mhz
	 */
	TIM_TimeBaseStructure.TIM_Prescaler = ((SystemCoreClock*2/APB1_PRESCALER) / DELAY_TIM_FREQUENCY) - 1;
	TIM_TimeBaseStructure.TIM_Period = UINT16_MAX;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

	TIM_TimeBaseStructure.TIM_Prescaler = ((SystemCoreClock*2/APB1_PRESCALER) / DELAY_TIM2_FREQUENCY) - 1;
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);


	/* Enable counter */
	TIM_Cmd(TIM6, ENABLE);
	TIM_Cmd(TIM7, ENABLE);
}

/* wait busy loop, milliseconds */
void delay_ms( uint16_t value ) {
	uint16_t wait_ms = value * 10; // since 1 = 100us = 0.1 ms
	TIM6->CNT = 0;
	while(TIM6->CNT < wait_ms);
}

void delay_100us( uint16_t value ) {
	uint16_t wait = value; // since 1 = 100us = 0.1 ms
	TIM6->CNT = 0;
	while(TIM6->CNT < wait);
}

void delay_us( uint16_t value ) {
	uint16_t wait_us = value; // since 1 = 1us
	TIM7->CNT = 0;
	while(TIM7->CNT < wait_us);
}
