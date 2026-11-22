#include "stm32f10x.h"                  // Device header
#include "SG90.h"
/**
  * 函    数：PWM初始化 (改用 PB8，基于 TIM4_CH3)
  */
void PWMPB8_Init(void)
{
    /* 1. 开启时钟：TIM4 挂载在 APB1，GPIOB 挂载在 APB2 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);    // 变为 TIM4
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);   // 变为 GPIOB
    
    /* 2. GPIO初始化：配置 PB8 为复用推挽输出 */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;               // 变为 Pin_8
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);                  // 变为 GPIOB
    
    /* 3. 配置时钟源 */
    TIM_InternalClockConfig(TIM4);                          // 变为 TIM4
    
    /* 4. 时基单元初始化 (保持 50Hz 频率用于舵机) */
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 20000 - 1;       // ARR
    TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;       // PSC
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);     // 变为 TIM4
    
    /* 5. 输出比较初始化 (核心：PB8 对应通道 3) */
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;                      // CCR
    
    TIM_OC3Init(TIM4, &TIM_OCInitStructure);                // 核心改动：OC3Init，TIM4
    
    /* 6. 使能定时器 */
    TIM_Cmd(TIM4, ENABLE);                                  // 变为 TIM4
}

/**
  * 函    数：设置 PB8 (TIM4_CH3) 的占空比
  */
void PWM_SetCompare_PB8(uint16_t Compare)
{
    TIM_SetCompare3(TIM4, Compare);                         // 核心改动：SetCompare3，TIM4
}
#include "stm32f10x.h"                  // Device header
#include "PWM.h"

void Servo_Init(void)
{
	PWMPB8_Init();
}

void Servo_SetAngle(float Angle)
{
	PWM_SetCompare_PB8(Angle / 180 * 2000 + 500);
}

