#include "Timer.h"

void TIM1Initialize(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_BaseInitStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_TIM1);
	
    TIM_BaseInitStructure.TIM_Period = 2-1;     				//24MHz
	TIM_BaseInitStructure.TIM_Prescaler = 4-1;
	TIM_BaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; 
    TIM_BaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;     
	TIM_BaseInitStructure.TIM_RepetitionCounter = 0;     
	TIM_TimeBaseInit(TIM1, &TIM_BaseInitStructure);     

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;     
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;   
	TIM_OCInitStructure.TIM_Pulse = 1;
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIM1, ENABLE);
	
    TIM_Cmd(TIM1, ENABLE);
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

void TIM1ChangeDiv(uint32_t DivCount){
	TIM_TimeBaseInitTypeDef TIM_BaseInitStructure;
	TIM_CtrlPWMOutputs(TIM1, DISABLE);
	TIM_Cmd(TIM1, DISABLE);
	TIM_BaseInitStructure.TIM_Period = 2-1;     				//22.5MHz
	TIM_BaseInitStructure.TIM_Prescaler = DivCount-1;     		// 4 -> 24MHz
																// 2 -> 48MHz
	TIM_BaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; 
    TIM_BaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;     
	TIM_BaseInitStructure.TIM_RepetitionCounter = 0;     
	TIM_TimeBaseInit(TIM1, &TIM_BaseInitStructure);
	TIM_Cmd(TIM1, ENABLE);
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
}
