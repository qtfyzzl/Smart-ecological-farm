#ifndef __DHT11_H
#define __DHT11_H 

#include "stm32f10x.h"
#include "sys.h" // 确保你的 sys.h 中包含 PAout/PAin 宏
#include <stdint.h>

// --- 引脚配置  ---
#define DHT11_GPIO_PORT    	GPIOA			            
#define DHT11_GPIO_CLK 	    RCC_APB2Periph_GPIOA		
#define DHT11_GPIO_PIN		  GPIO_Pin_15// 

// --- IO操作宏 ---											   
#define	DHT11_DQ_OUT        PAout(15) 
#define	DHT11_DQ_IN         PAin(15)  

// --- 函数声明 ---
uint8_t DHT11_Init(void);
uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi);
uint8_t DHT11_Read_Byte(void);
uint8_t DHT11_Read_Bit(void);
uint8_t DHT11_Check(void);
void DHT11_Rst(void); 
void dht11_con(void);

#endif
