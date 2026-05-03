#ifndef __SHOW_H
#define __SHOW_H

#include "stm32f10x.h"
#include "show.h"
#include "OLED.h"
#include "Serial.h"
#include "DHT11.h"
#include "WATERSENSOR.h"
#include "YFS401.h"
#include "Delay.h"
#include <stdio.h>

/**
  * 函    数：全硬件初始化
  * 说    明：包含所有底层外设、传感器、OLED及串口的初始化。
  * 用    法：在 main 函数的 while(1) 之前调用一次即可。
  */
void Show_Init(void);

/**
  * 函    数：显示与采集主循环模块
  * 说    明：包含了数据采集、OLED刷新、串口屏推送以及蓝牙指令处理。
  * 用    法：放在 main 函数的 while(1) 中死循环调用。内部已实现非阻塞时间片管理，不会卡死主程序。
  */
void Show_MainLoop(void);

#endif
