#include "solar.h"

/**
 * @brief  电压检测模块初始化 (配置 PB0 为模拟输入, 初始化 ADC1)
 */
void Voltage_Sensor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    // 1. 使能 GPIOB 和 ADC1 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_ADC1, ENABLE);
    
    // 2. 设置 ADC 时钟分频 (72MHz / 6 = 12MHz, ADC 最大时钟不能超过 14MHz)
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    // 3. 配置 PB0 为模拟输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 4. ADC1 初始化配置
    ADC_DeInit(ADC1);                                   // 复位 ADC1
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;  // 独立工作模式
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;       // 单通道模式
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; // 单次转换模式
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // 软件触发
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;              // 数据右对齐
    ADC_InitStructure.ADC_NbrOfChannel = 1;                             // 顺序进行规则转换的 ADC 通道的数目
    ADC_Init(ADC1, &ADC_InitStructure);

    // 5. 使能 ADC1 并进行内部校准
    ADC_Cmd(ADC1, ENABLE);

    ADC_ResetCalibration(ADC1);                 // 使能复位校准
    while(ADC_GetResetCalibrationStatus(ADC1)); // 等待复位校准结束

    ADC_StartCalibration(ADC1);                 // 开启 AD 校准
    while(ADC_GetCalibrationStatus(ADC1));      // 等待校准结束
}

/**
 * @brief  获取一次指定 ADC 通道的值
 * @param  ch: ADC 通道号 (例如 ADC_Channel_8)
 * @return uint16_t: 12位 ADC 原始值 (0~4095)
 */
uint16_t Get_Adc_Value(uint8_t ch)
{
    // 设置指定 ADC 的规则组通道，序列号，采样时间 (选择 239.5 周期以提高采集稳定性)
    ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5);

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);         // 使能软件转换启动

    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));  // 等待转换结束

    return ADC_GetConversionValue(ADC1);            // 返回转换结果
}

/**
 * @brief  获取多次 ADC 值取平均（简单的均值滤波）
 * @param  ch: ADC 通道号
 * @param  times: 采样次数
 * @return uint16_t: 平均 ADC 值
 */
uint16_t Get_Adc_Average(uint8_t ch, uint8_t times)
{
    uint32_t temp_val = 0;
    uint8_t t;
    
    for(t = 0; t < times; t++)
    {
        temp_val += Get_Adc_Value(ch);
    }
    
    return temp_val / times;
}

/**
 * @brief  获取并计算模块输入端的实际电压
 * @return float: 实际电压值 (V)
 */
float Get_Actual_Voltage(void)
{
    // 1. 采集 10 次 PB0 (ADC_Channel_8) 的值取平均
    uint16_t adc_avg = Get_Adc_Average(ADC_Channel_8, 10);
    
    // 2. 将 ADC 原始值转换为单片机引脚侧的测量电压 (假设单片机供电/基准电压为精确的 3.3V)
    float measure_volts = (float)adc_avg * (3.3f / 4096.0f);
    
    // 3. 根据电压检测模块的分压比例 (30k 和 7.5k，即 5:1) 还原真实电压
    float actual_volts = measure_volts * 5.0f;
    
    return actual_volts;
}
/**
 * @brief  获取光照强度百分比 (0% - 100%)
 * @return uint8_t: 光照百分比
 */
uint8_t Get_Light_Intensity_Percent(void)
{
    // 1. 获取当前太阳能板的实际输出电压
    float current_voltage = Get_Actual_Voltage();
    float percent = 0.0f;
    
    // 2. 异常值处理 (防止电压为负)
    if (current_voltage <= 0.0f) {
        return 0;
    }
    
    // 3. 计算百分比
    // 公式: (当前电压 / 最大电压) * 100
    percent = (current_voltage / SOLAR_PANEL_MAX_VOLTAGE) * 100.0f;
    
    // 4. 饱和处理 (防止超出 100%)
    if (percent > 100.0f) {
        percent = 100.0f;
    }
    
    return (uint8_t)percent;
}
