#include "DHT11.h"
#include "delay.h"
#include "Serial.h"

// 配置为推挽输出模式
void DHT11_IO_OUT(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(DHT11_GPIO_CLK | RCC_APB2Periph_AFIO, ENABLE);	 
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); // 释放 PA15 作为普通 IO
    
    GPIO_InitStructure.GPIO_Pin = DHT11_GPIO_PIN;			
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStructure);				
}

// 配置为浮空输入模式
// 配置为输入模式
void DHT11_IO_IN(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(DHT11_GPIO_CLK, ENABLE);	// 这里删掉了 AFIO 
    // GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); // PA0不需要关闭JTAG，可以注释掉
    
    GPIO_InitStructure.GPIO_Pin = DHT11_GPIO_PIN;				 
    // 【关键修复】将浮空输入改为上拉输入！
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 
    GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStructure);				 
}

// 复位DHT11并发送起始信号
void DHT11_Rst(void)	   
{                 
    DHT11_IO_OUT(); 	
    DHT11_DQ_OUT = 0; 	// 主机拉低总线
    Delay_ms(20);    	// 保持至少18ms
    DHT11_DQ_OUT = 1; 	// 释放总线 
    Delay_us(30);     	// 主机延时20~40us，等待DHT11响应
}

// 等待DHT11的回应
// 返回值：0:存在, 1:未检测到
uint8_t DHT11_Check(void) 	   
{   
    uint8_t retry = 0;
    DHT11_IO_IN();
    
    // DHT11会拉低40~80us
    while (DHT11_DQ_IN && retry < 100)
    {
        retry++;
        Delay_us(1);
    }	 
    if(retry >= 100) return 1;
    else retry = 0;
    
    // DHT11拉低后会再次拉高40~80us
    while (!DHT11_DQ_IN && retry < 100)
    {
        retry++;
        Delay_us(1);
    }
    if(retry >= 100) return 1;	    
    
    return 0;
}

// 从DHT11读取一个位 (返回 1 或 0)
uint8_t DHT11_Read_Bit(void) 			 
{
    uint8_t retry = 0;
    
    // 等待上一个高电平结束（等待变为低电平准备状态）
    while(DHT11_DQ_IN && retry < 100)
    {
        retry++;
        Delay_us(1);
    }
    
    retry = 0;
    // 等待低电平结束（等待数据位的高电平到来）
    while(!DHT11_DQ_IN && retry < 100)
    {
        retry++;
        Delay_us(1);
    }
    
    Delay_us(40); // 延时40us后判断电平。0为26-28us，1为70us
    
    if(DHT11_DQ_IN) return 1;
    else return 0;		   
}

// 从DHT11读取一个字节 (返回读到的数据)
uint8_t DHT11_Read_Byte(void)    
{        
    uint8_t i, dat = 0;
    for (i = 0; i < 8; i++) 
    {
        dat <<= 1; 
        dat |= DHT11_Read_Bit();
    }						    
    return dat;
}

// 读取一次温湿度数据
// temp:温度值(范围:0~50°), humi:湿度值(范围:20%~90%)
// 返回值：0:正常, 1:读取失败(校验错或无响应)
// 返回值：0:正常, 1:无响应, 2:数据校验错误
uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi)    
{        
    uint8_t buf[5];
    uint8_t i;
    DHT11_Rst();
    
    if(DHT11_Check() == 0) // 传感器有响应
    {
        for(i = 0; i < 5; i++) buf[i] = DHT11_Read_Byte();
        
        if((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])
        {
            *humi = buf[0];
            *temp = buf[2];
            return 0; // 成功
        }
        else return 2; // 【新增】数据校验错误
    }
    return 1; // 传感器彻底无响应（可能没插紧或引脚不对）	    
}

// 初始化DHT11
// 返回值：0:存在, 1:不存在    	 
uint8_t DHT11_Init(void)
{	 
    DHT11_Rst();  
    return DHT11_Check();
}

