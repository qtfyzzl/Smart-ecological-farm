#include "Sonar.h"
#include "Delay.h"

/**
  * 函    数：超声波模块初始化
  */
void Sonar_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);  
    
    GPIO_InitTypeDef GPIO_InitStructure;          
    
    // 1. 初始化 Trig (PB9)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    
    GPIO_InitStructure.GPIO_Pin = SONAR_TRIG_PIN;             
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
    GPIO_Init(SONAR_TRIG_PORT, &GPIO_InitStructure);          
    
    // 2. 初始化 Echo (PA11)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;      
    GPIO_InitStructure.GPIO_Pin = SONAR_ECHO_PIN;             
    GPIO_Init(SONAR_ECHO_PORT, &GPIO_InitStructure);          
    
    // 3. 默认拉低
    GPIO_WriteBit(SONAR_TRIG_PORT, SONAR_TRIG_PIN, Bit_RESET);             
    Delay_us(15);                      
}

/**
  * 函    数：超声波测距 
  * 返 回 值：经过【误差校准】后的实际距离，单位：厘米 (cm)
  */
uint16_t Sonar_GetDistance_cm(void)                 
{
    uint32_t time = 0;
    uint32_t timeout = 0;
    int16_t raw_distance = 0;     // 原始读数
    int16_t actual_distance = 0;  // 校准后的真实距离
    
    /* 1. 发送触发信号 */
    GPIO_WriteBit(SONAR_TRIG_PORT, SONAR_TRIG_PIN, Bit_SET);             
    Delay_us(15);                    
    GPIO_WriteBit(SONAR_TRIG_PORT, SONAR_TRIG_PIN, Bit_RESET);             
    
    /* 2. 等待高电平 */
    timeout = 0;
    while(GPIO_ReadInputDataBit(SONAR_ECHO_PORT, SONAR_ECHO_PIN) == 0)    
    {
        Delay_us(10);
        timeout++;
        if(timeout > 1000) return 0; 
    }
    
    /* 3. 计时测距 */
    time = 0;                        
    while(GPIO_ReadInputDataBit(SONAR_ECHO_PORT, SONAR_ECHO_PIN) == 1)    
    {
        Delay_us(10);  
        time++;        
        if(time > 3800) break; 
    }
    
    /* 4. 距离换算与【软件标定补偿】 */
    if(time <= 3800)                 
    {
        raw_distance = (time * 346) / 2000;  // 算出原始误差距离
        
        // 加上你在头文件里定义的校准偏移量
        actual_distance = raw_distance + SONAR_CALIB_OFFSET;
        
        // 防御性编程：万一填了巨大的负数，强制清零，防止距离变负溢出
        if (actual_distance < 0) {
            actual_distance = 0;
        }
    }
    
    return (uint16_t)actual_distance;                  
}

/**
  * 函    数：计算剩余水位百分比
  * 参    数：actual_distance (刚刚测出来的【真实距离】)
  */
uint8_t Get_WaterLevel_Percent(uint16_t actual_distance)
{
    float percent;

    /* 1. 极限值保护：水空了 (大于等于空水距离) */
    if (actual_distance >= DISTANCE_EMPTY) 
    {
        return 0; 
    }
    /* 2. 极限值保护：水满了 (小于等于满水距离，要求实际15cm处为100%) */
    if (actual_distance <= DISTANCE_FULL) 
    {
        return 100; 
    }

    /* 3. 核心公式计算 
     *    容器总蓄水深度 = 空水距离 - 满水距离
     *    当前真实水深   = 空水距离 - 当前传感器测得的实际距离
     */
    float total_depth = DISTANCE_EMPTY - DISTANCE_FULL;
    float current_depth = DISTANCE_EMPTY - (float)actual_distance;
    
    // 计算百分比
    percent = (current_depth / total_depth) * 100.0f;

    return (uint8_t)percent;
}
