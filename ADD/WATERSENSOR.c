//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//欣薇电子
//
//  文 件 名   : ad.c
//  版 本 号   : v1.0
//  作    者   : 欣薇电子
//  生成日期   : 20241001
//  最近修改   : 
//  功能描述   : adc配置
// 日    期   : 
// 作    者   : 欣薇电子
// 修改内容   : 创建文件
//版权所有，盗版必究。
//Copyright(C) 欣薇电子2024/10/1
//All rights reserved
//******************************************************************************/
#include "Delay.h"
#include "WATERSENSOR.h"
#include "Serial.h"
#include <stdio.h> 
#include "string.h"

typedef unsigned          char uint8_t;
/** watersensor
  * 函    数：AD初始化
  * 参    数：无
  * 返 回 值：无
  */
void watersensor_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);	//开启ADC1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//开启GPIOA的时钟
	
	/*设置ADC时钟*/
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);						//选择时钟6分频，ADCCLK = 72MHz / 6 = 12MHz
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 |GPIO_Pin_1 ;         //通道8 通道9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//将引脚初始化为模拟输入
	
	/*不在此处配置规则组序列，而是在每次AD转换前配置，这样可以灵活更改AD转换的通道*/
	
	/*ADC初始化*/
	ADC_InitTypeDef ADC_InitStructure;						//定义结构体变量
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;		//模式，选择独立模式，即单独使用ADC1
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//数据对齐，选择右对齐
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//外部触发，使用软件触发，不需要外部触发
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;		//连续转换，失能，每转换一次规则组序列后停止
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;			//扫描模式，失能，只转换规则组的序列1这一个位置
	ADC_InitStructure.ADC_NbrOfChannel = 1;					//通道数，为1，仅在扫描模式下，才需要指定大于1的数，在非扫描模式下，只能是1
	ADC_Init(ADC1, &ADC_InitStructure);						//将结构体变量交给ADC_Init，配置ADC1
	
	/*ADC使能*/
	ADC_Cmd(ADC1, ENABLE);									//使能ADC1，ADC开始运行
	
	/*ADC校准*/
	ADC_ResetCalibration(ADC1);								//固定流程，内部有电路会自动执行校准
	while (ADC_GetResetCalibrationStatus(ADC1) == SET);
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1) == SET);
}

/**
  * 函    数：获取AD转换的值
  * 参    数：ADC_Channel 指定AD转换的通道，范围：ADC_Channel_x，其中x可以是0/1/2/3
  * 返 回 值：AD转换的值，范围：0~4095
  */
//获得ADC值
//ch:通道值 0~3
u16 Get_watersensor(u8 ch)   
{
  	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}

u16 Get_watersensor_Average(u8 ch,u8 times)    //获取平均值
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val+=Get_watersensor(ch);
		Delay_ms(5);
	}
	return temp_val/times;
} 	 


uint32_t M_dry = 	1450;    	// 干燥时的 ADC 原始值
uint32_t M_wet =	3250; 		// 完全淹没时的 ADC 原始值



u16 Get_water(void)
{
    u16 aa = 0;
    aa = Get_watersensor_Average(9, 30);
    
    // 1. 限幅处理 (正向逻辑：防跌破下限，防冲破上限)
    if(aa < M_dry) aa = M_dry;
    if(aa > M_wet) aa = M_wet;
    
    // 2. 转换为百分比 (0% ~ 100%)
    // 逻辑：(当前值 - 干燥值) * 100 / (满水值 - 干燥值)
    aa = (aa - M_dry) * 100 / (M_wet - M_dry);
    
    return aa; 
}

// 1280 ~ 4090
u16 Get_light(void)
{
    u16 bb = 0;
    bb = Get_watersensor_Average(8,30);
    
    if(bb > 4090) bb = 4090;      // 限幅上限
    if(bb < 1280) bb = 1280;      // 限幅下限
    
    // 修正了分母，保持与限幅值一致
    bb = (4090 - bb) * 100 / (4090 - 1280);  
    
    if(bb == 100) bb = 99;
    return bb; 
}

void water_con()
{
    char oled_str[100]; 

    // --- 1. 获取水位传感器数据 (通道9) ---
    uint16_t water_raw = Get_watersensor_Average(9, 30); 
    uint16_t water_percent = Get_water();              

    // --- 2. 获取光敏传感器数据 (通道8) ---
    uint16_t light_raw = Get_watersensor_Average(8, 30); 
    uint16_t light_percent = Get_light();                

    // --- 3. 格式化并打印 ---
    sprintf(oled_str, "Water RAW: %d (%d %%) | Light RAW: %d (%d %%)", 
            water_raw, water_percent, light_raw, light_percent);
    
    Serial_SendString(USART2, oled_str);
    Serial_SendString(USART2, "\r\n");
    Delay_ms(100); 
}
