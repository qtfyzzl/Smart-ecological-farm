#include "stm32f10x.h"
#include "YFS401.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_tim.h"
#include "misc.h"
#include "OLED.h"
#include "Serial.h"

static volatile uint32_t s_yfs201_pulse_count = 0;
static volatile uint32_t s_yfs201_last_sec_count = 0;
static volatile uint32_t s_yfs201_flow_ml_min = 0; 

// 滑动平均滤波数组
#define FILTER_LEN 4
static uint32_t delta_buffer[FILTER_LEN] = {0};
static uint8_t filter_idx = 0;

// 【关键修改】：启用完全空闲的 TIM1 作为 1Hz (1秒) 的结算定时器
static void YFS201_TIM1_1Hz_Config(void)
{
    // TIM1 挂载在 APB2 上，频率为 72MHz
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    TIM_TimeBaseInitTypeDef tim;
    tim.TIM_ClockDivision = TIM_CKD_DIV1;
    tim.TIM_CounterMode = TIM_CounterMode_Up;
    tim.TIM_Period = 10000 - 1;       // 计数值 10000
    tim.TIM_Prescaler = 7200 - 1;     // 分频 7200 -> (72M/7200) = 10000Hz, 再计数10000次正好是 1Hz(1秒)
    tim.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &tim);

    TIM_ClearFlag(TIM1, TIM_FLAG_Update);
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);

    NVIC_InitTypeDef nvic;
    nvic.NVIC_IRQChannel = TIM1_UP_IRQn; // TIM1 的更新中断
    nvic.NVIC_IRQChannelCmd = ENABLE;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&nvic);

    TIM_Cmd(TIM1, ENABLE);
}

void YFS201_Init(void)
{
    RCC_APB2PeriphClockCmd(YFS201_GPIO_CLOCK | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = YFS201_GPIO_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IPU; 
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(YFS201_GPIO_PORT, &gpio);

    GPIO_EXTILineConfig(YFS201_GPIO_PORTSOURCE, YFS201_GPIO_PINSOURCE);

    EXTI_InitTypeDef exti;
    EXTI_StructInit(&exti);
    exti.EXTI_Line = YFS201_EXTI_LINE;
    exti.EXTI_Mode = EXTI_Mode_Interrupt;
    exti.EXTI_Trigger = EXTI_Trigger_Falling; 
    exti.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti);

    NVIC_InitTypeDef nvic;
    nvic.NVIC_IRQChannel = YFS201_EXTI_IRQn;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&nvic);

    // 启动 TIM1 定时器
    YFS201_TIM1_1Hz_Config();
}

uint32_t YFS201_GetPulseCount(void) { return s_yfs201_pulse_count; }

uint32_t YFS201_GetTotalMl(void) {
    return (uint32_t)(((float)s_yfs201_pulse_count / (float)YFS201_PULSES_PER_LITER) * 1000.0f);
}

uint32_t YFS201_GetFlowMlMin(void) { return s_yfs201_flow_ml_min; }

void YFS201_ResetTotal(void)
{
    __disable_irq();
    s_yfs201_pulse_count = 0;
    s_yfs201_last_sec_count = 0;
    s_yfs201_flow_ml_min = 0;
    for(int i=0; i<FILTER_LEN; i++) delta_buffer[i] = 0;
    __enable_irq();
}

/**
 * 外部中断1服务函数 (对应你头文件里的 PB1)
 */
void EXTI1_IRQHandler(void)
{
    if(EXTI_GetITStatus(YFS201_EXTI_LINE) != RESET)
    {
        s_yfs201_pulse_count++; // 脉冲累加
        EXTI_ClearITPendingBit(YFS201_EXTI_LINE); 
    }
}

/**
 * 【关键修改】：这里改为 TIM1 的中断服务函数
 */
void TIM1_UP_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
        
        uint32_t now = s_yfs201_pulse_count;
        uint32_t delta = now - s_yfs201_last_sec_count;
        s_yfs201_last_sec_count = now;
        
        delta_buffer[filter_idx] = delta;
        filter_idx = (filter_idx + 1) % FILTER_LEN;
        
        uint32_t sum = 0;
        for(int i = 0; i < FILTER_LEN; i++) {
            sum += delta_buffer[i];
        }
        float avg_delta = (float)sum / (float)FILTER_LEN;
        
        s_yfs201_flow_ml_min = (uint32_t)((avg_delta / (float)YFS201_PULSES_PER_LITER) * 60000.0f);
    }
}

void yfs401_con(void)
{
    char str_buffer[60]; 
    uint32_t total_ml;   
    uint32_t flow_mlmin; 
                
    total_ml = YFS201_GetTotalMl();
    flow_mlmin = YFS201_GetFlowMlMin();

    sprintf(str_buffer, "总量: %d mL, 流速: %d mL/min\r\n", total_ml, flow_mlmin);
    Serial_SendString(USART3, str_buffer);

    sprintf(str_buffer, "Vol : %-4d mL   ", total_ml);
    OLED_ShowString(1, 1, str_buffer);
    
    sprintf(str_buffer, "Flow: %-4d mL/m ", flow_mlmin);
    OLED_ShowString(3, 1, str_buffer);
}
