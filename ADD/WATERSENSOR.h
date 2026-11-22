#ifndef __WATERSENSOR_H
#define __WATERSENSOR_H
#include "sys.h"

void watersensor_Init(void);
u16 Get_watersensor(u8 ch) ;
u16 Get_watersensor_Average(u8 ch,u8 times);              								 //湿度百分比转化函数
u16 Get_water(void);
u16 Get_light(void);
void water_con(void);
	
#endif


