#include "stm32f10x.h"

/**
  * 函    数：PWM初始化 (水泵PA0, LEDPA1, 风扇PA2)
  */
void PWM_Init(void)
{
	/* 1. 开启时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	/* 2. GPIO初始化 */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_3;; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* 3. 配置时钟源 */
	TIM_InternalClockConfig(TIM2);		
	
	/* 4. 时基单元初始化 (1kHz频率，适合电机驱动) */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;                // ARR: 0-100调速
	TIM_TimeBaseInitStructure.TIM_Prescaler = 36 - 1;             // PSC: 分频降至1kHz
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
	
	/* 5. 输出比较初始化 */
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCStructInit(&TIM_OCInitStructure);                         
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;               
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;       
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;   
	TIM_OCInitStructure.TIM_Pulse = 0;                              

	/* 6. 初始化通道 */
	TIM_OC1Init(TIM2, &TIM_OCInitStructure); // PA0
	TIM_OC2Init(TIM2, &TIM_OCInitStructure); // PA1
//	TIM_OC3Init(TIM2, &TIM_OCInitStructure); // PA2
	TIM_OC4Init(TIM2, &TIM_OCInitStructure); // <--- 修改这里：TIM_OC4Init 对应 PA3
	
	/* 7. 使能定时器 */
	TIM_Cmd(TIM2, ENABLE);
}

void PWM_SetCompare1(uint16_t Compare) { TIM_SetCompare1(TIM2, Compare); }
void PWM_SetCompare2(uint16_t Compare) { TIM_SetCompare2(TIM2, Compare); }
//void PWM_SetCompare3(uint16_t Compare) { TIM_SetCompare3(TIM2, Compare); }
void PWM_SetCompare4(uint16_t Compare) { TIM_SetCompare4(TIM2, Compare); }
