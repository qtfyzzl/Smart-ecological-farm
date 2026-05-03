#include "BlindMotor.h"

// 初始设定：左边大滚筒(30mm)，右边小滚筒(2mm)
uint8_t g_BlindSpeed_Left  = 50;  
uint8_t g_BlindSpeed_Right = 100; 

/**
  * 函    数：卷帘双电机初始化 
  */
void BlindMotor_Init(void)
{
    /* 1. 开启时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    
    /* 2. 初始化方向控制 GPIO (推挽输出) */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
    // 初始化 PA2(左方向), PA4(右方向), PA5(右方向)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 初始化 PB5(左方向)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* 3. 初始化 PWM GPIO (PA6, PA7 复用推挽输出) */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* 4. TIM3 时基单元初始化 (频率 20kHz) */
    TIM_InternalClockConfig(TIM3);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;       
    TIM_TimeBaseInitStructure.TIM_Prescaler = 36 - 1;     
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
    
    /* 5. TIM3 通道1(PA6) 和 通道2(PA7) 初始化 */
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    
    TIM_OC1Init(TIM3, &TIM_OCInitStructure); // PA6 -> 右电机 (Motor A)
    TIM_OC2Init(TIM3, &TIM_OCInitStructure); // PA7 -> 左电机 (Motor B)
    
    TIM_Cmd(TIM3, ENABLE);
    
    // 默认上电停止
    GPIO_ResetBits(GPIOA, GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_5);
    GPIO_ResetBits(GPIOB, GPIO_Pin_5);
}

/**
  * 函    数：控制卷帘双电机的动作
  */
void Blind_SetAction(uint8_t Action)
{
    // 安全限幅保护
    if (g_BlindSpeed_Left > 100)  g_BlindSpeed_Left = 100;
    if (g_BlindSpeed_Right > 100) g_BlindSpeed_Right = 100;
    
    if (Action == 1) // 动作1：合上/下降 
    {
        // 右电机 (Motor A) -> 逆时针
        GPIO_ResetBits(GPIOA, GPIO_Pin_4);
        GPIO_SetBits(GPIOA, GPIO_Pin_5);
        
        // 左电机 (Motor B) -> 逆时针 【关键修改：释放刹车，改为逆时针】
        GPIO_ResetBits(GPIOA, GPIO_Pin_2);
        GPIO_SetBits(GPIOB, GPIO_Pin_5);
        
        // 输出动力：右边给当前设定速度，左边固定给 60
        TIM_SetCompare1(TIM3, g_BlindSpeed_Right); // PA6 输出给右电机 (100)
        TIM_SetCompare2(TIM3, 60);                 // PA7 左电机固定为 60
    }
    else if (Action == 2) // 动作2：拉开/上升 (两边同时顺时针)
    {
        // 右电机 (Motor A) -> 顺时针
        GPIO_SetBits(GPIOA, GPIO_Pin_4);
        GPIO_ResetBits(GPIOA, GPIO_Pin_5);
        
        // 左电机 (Motor B) -> 顺时针
        GPIO_SetBits(GPIOA, GPIO_Pin_2);
        GPIO_ResetBits(GPIOB, GPIO_Pin_5);
        
        // 精准输出对应的左右速度
        TIM_SetCompare1(TIM3, g_BlindSpeed_Right); // PA6 输出给右电机
        TIM_SetCompare2(TIM3, g_BlindSpeed_Left);  // PA7 输出给左电机
    }
    else // 停止
    {
        // 全部刹车抱死
        GPIO_ResetBits(GPIOA, GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_5);
        GPIO_ResetBits(GPIOB, GPIO_Pin_5);
        
        TIM_SetCompare1(TIM3, 0); 
        TIM_SetCompare2(TIM3, 0); 
    }
}
