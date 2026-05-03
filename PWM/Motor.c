#include "stm32f10x.h"
#include "PWM.h"
#include "Delay.h"

/**
  * 函    数：水泵设置运行功率 (PA0)
  */
void Pump_SetPower(uint8_t Power)
{       
    if (Power > 100) Power = 100;
	PWM_SetCompare1(Power);             
}

/**
  * 函    数：LED亮度设置 (PA1)
  */
void LED_SetBrightness(uint8_t Brightness)
{
    if (Brightness > 100) Brightness = 100;
    PWM_SetCompare2(Brightness);            
}

/**
  * 函    数：风扇调速 (PA2)
  */


void Fan_SetSpeed(uint8_t Speed)
{		
    uint8_t i;
    static uint8_t last_speed = 0; 

    if (Speed > 100) Speed = 100;
    
    if (last_speed == 0 && Speed > 0 && Speed < 70) {
        for (i = 100; i > Speed; i--)
        {
            PWM_SetCompare4(i); // <--- 改成 4
            Delay_ms(1);
        }  
    }
    
    PWM_SetCompare4(Speed); // <--- 改成 4
    last_speed = Speed;
}

/**
  * 函    数：底层驱动初始化
  */
void Pump_Init(void)
{     
    /* 初始化底层PWM (同时开启 PA0水泵, PA1LED, PA2风扇 的硬件输出) */
    PWM_Init();													
    
    /* 强行给所有设备赋初值 0，防止上电乱转 */
    Pump_SetPower(0);
    LED_SetBrightness(0);
    Fan_SetSpeed(0);
}
