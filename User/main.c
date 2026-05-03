
/*USART1 (挂载在APB2): TX = PA9, RX = PA10
USART2 (挂载在APB1): TX = PA2, RX = PA3
USART3 (挂载在APB1): TX = PB10, RX = PB11


TB661-水泵			PA0(PWM) 		 		3.3v*2 5v*1	
TB661-LED				PA1(PWM)  			12v	
电机-排气扇     PA3
串口屏 					PA9 PA10 				5v 
湿度传感器			PA15  					5v
N20减速电机			PA6 PA7(PWM) PA4 PA5 PA2 PB5(位控线)

太阳能板				PB0
水流量计 				PB1															3.5-24vcd
超声波测量 			Trig(PB9), Echo(PA11) 																3.3v
温度传感器			PB6
舵机            PB8  														5v*1 
蓝牙						PB10 PB11  											3.3v
oled            PB12-14 PA8
                           								

// ==================== 调试参数区 ====================
#define PUMP_POWER_LOW      20   		//水泵占空比 该注释不可删除
#define PUMP_POWER_MID      35    
#define PUMP_POWER_HIGH     45   
#define CURTAIN_RUN_SPEED   40      //N20电机运行速率 也是占空比 该注释不可删
// ====================================================

*/
#include "stm32f10x.h"
#include "show.h"

int main(void)
{		
    Show_Init();       // 底层外设一次性初始化
   
    while(1)
    {
       Show_MainLoop(); // 非阻塞主状态机
//				yfs401_con();
    }
}

