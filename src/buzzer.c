#include <stdint.h>
#include "stm32.h"
#include "buzzer.h"
#include "math_utils.h"

void buzzer_init(void) {
	RCC_AHB1PeriphClockCmd(BUZZER_RCC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB2Periph_TIM11, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;


	GPIO_InitStructure.GPIO_Pin = BUZZER_PIN;// Buzzer
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(BUZZER_GPIO_PORT, &GPIO_InitStructure);

    GPIO_PinAFConfig (GPIOB, GPIO_PinSource9, GPIO_AF_TIM11);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;

    //  60 KHZ, 24 MHz / 400
    // 72 MHz / 1200
    // 84 MHz / 1400
    //  60000 Hz chosen as a large, clean number fitting in 16-bit

	TIM_TimeBaseInitStruct.TIM_Prescaler = 1400 - 1; // 60 KHz timebase
	TIM_TimeBaseInitStruct.TIM_Period = 1000 - 1;    // Arbitary placeholder 60 Hz
	TIM_TimeBaseInitStruct.TIM_ClockDivision = 0;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM11, &TIM_TimeBaseInitStruct);

	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_Pulse = (TIM_TimeBaseInitStruct.TIM_Period + 1) / 2; // 50% Duty
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;

	TIM_OC1Init(TIM11, &TIM_OCInitStruct); // Channel 1 PB9 Buzzer

	TIM_Cmd(TIM11, ENABLE);
}

void buzzer_process(uint32_t msticks) {

}
