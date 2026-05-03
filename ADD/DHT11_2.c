#include "DHT11_2.h"
#include "delay.h"

// 配置为推挽输出模式
void DHT11_2_IO_OUT(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(DHT11_2_GPIO_CLK, ENABLE);	 
    
    GPIO_InitStructure.GPIO_Pin = DHT11_2_GPIO_PIN;			
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_2_GPIO_PORT, &GPIO_InitStructure);				
}

// 配置为上拉输入模式
void DHT11_2_IO_IN(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(DHT11_2_GPIO_CLK, ENABLE);	
    
    GPIO_InitStructure.GPIO_Pin = DHT11_2_GPIO_PIN;				 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 
    GPIO_Init(DHT11_2_GPIO_PORT, &GPIO_InitStructure);				 
}

void DHT11_2_Rst(void)	   
{                 
    DHT11_2_IO_OUT(); 	
    DHT11_2_DQ_OUT = 0; 
    Delay_ms(20);    	
    DHT11_2_DQ_OUT = 1; 	 
    Delay_us(30);     	
}

uint8_t DHT11_2_Check(void) 	   
{   
    uint8_t retry = 0;
    DHT11_2_IO_IN();
    
    while (DHT11_2_DQ_IN && retry < 100) {
        retry++;
        Delay_us(1);
    }	 
    if(retry >= 100) return 1;
    else retry = 0;
    
    while (!DHT11_2_DQ_IN && retry < 100) {
        retry++;
        Delay_us(1);
    }
    if(retry >= 100) return 1;	    
    
    return 0;
}

uint8_t DHT11_2_Read_Bit(void) 			 
{
    uint8_t retry = 0;
    while(DHT11_2_DQ_IN && retry < 100) {
        retry++;
        Delay_us(1);
    }
    retry = 0;
    while(!DHT11_2_DQ_IN && retry < 100) {
        retry++;
        Delay_us(1);
    }
    Delay_us(40); 
    if(DHT11_2_DQ_IN) return 1;
    else return 0;		   
}

uint8_t DHT11_2_Read_Byte(void)    
{        
    uint8_t i, dat = 0;
    for (i = 0; i < 8; i++) 
    {
        dat <<= 1; 
        dat |= DHT11_2_Read_Bit();
    }						    
    return dat;
}

uint8_t DHT11_2_Read_Data(uint8_t *temp, uint8_t *humi)    
{        
    uint8_t buf[5];
    uint8_t i;
    DHT11_2_Rst();
    
    if(DHT11_2_Check() == 0) 
    {
        for(i = 0; i < 5; i++) buf[i] = DHT11_2_Read_Byte();
        
        if((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])
        {
            *humi = buf[0];
            *temp = buf[2];
            return 0; 
        }
        else return 2; 
    }
    return 1; 
}

uint8_t DHT11_2_Init(void)
{	 
    DHT11_2_Rst();  
    return DHT11_2_Check();
}
