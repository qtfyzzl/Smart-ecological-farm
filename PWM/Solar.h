#ifndef __SOLAR_H
#define __SOLAR_H



#include "stm32f10x.h"

/* --- 太阳能板参数配置 --- */
// 请用万用表或串口打印，在最强光照下测得的最大电压值替换此宏
#define SOLAR_PANEL_MAX_VOLTAGE  4.5f  

/* 函数声明 */
void Voltage_Sensor_Init(void);
uint16_t Get_Adc_Value(uint8_t ch);
uint16_t Get_Adc_Average(uint8_t ch, uint8_t times);
float Get_Actual_Voltage(void);

// 新增：获取光照强度百分比函数
uint8_t Get_Light_Intensity_Percent(void); 

#endif /* __SOLAR_H */
