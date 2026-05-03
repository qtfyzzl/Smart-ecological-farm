#ifndef __DHT11_2_H
#define __DHT11_2_H 

#include "stm32f10x.h"
#include "sys.h" 

// --- 引脚配置 (完美接盘原来 SHT30 的 PB6) ---
#define DHT11_2_GPIO_PORT    GPIOB			            
#define DHT11_2_GPIO_CLK     RCC_APB2Periph_GPIOB		
#define DHT11_2_GPIO_PIN     GPIO_Pin_6 

// --- IO操作宏 ---											   
#define	DHT11_2_DQ_OUT       PBout(6) 
#define	DHT11_2_DQ_IN        PBin(6)  

// --- 函数声明 ---
uint8_t DHT11_2_Init(void);
uint8_t DHT11_2_Read_Data(uint8_t *temp, uint8_t *humi);
uint8_t DHT11_2_Read_Byte(void);
uint8_t DHT11_2_Read_Bit(void);
uint8_t DHT11_2_Check(void);
void DHT11_2_Rst(void); 

#endif
