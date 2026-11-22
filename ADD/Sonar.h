#ifndef __SONAR_H
#define __SONAR_H

#include "stm32f10x.h"

// ================= 宏定义区 (引脚配置) =================
// Trig 发送脚: PB9
#define SONAR_TRIG_PIN  GPIO_Pin_9
#define SONAR_TRIG_PORT GPIOB

// Echo 接收脚: PA11 
#define SONAR_ECHO_PIN  GPIO_Pin_11
#define SONAR_ECHO_PORT GPIOA

// ================= 宏定义区 (误差校准与水位标定) =================
/* 1. 超声波硬件误差校准值 (支持正负数)
 *    公式：真实距离 = 传感器读数 + 校准值
 *    现象：实测读出 10cm，但用尺子量实际是 20cm。少了10cm。
 *    解决：填入 10。(即 10 + 10 = 20cm) 
 */
#define SONAR_CALIB_OFFSET   10  

/* 2. 容器水位两极标定 (从超声波探头往下量)
 *    要求：在实际 15cm 处，让水位显示为 100%。
 */
#define DISTANCE_FULL        10.0f  // 100% 满水时，探头到水面的实际距离 (cm)
#define DISTANCE_EMPTY       24.0f  // 0% 空水时，探头到槽底的实际距离 (cm)

// ================= 函数声明区 =================
void Sonar_Init(void);
uint16_t Sonar_GetDistance_cm(void);
uint8_t Get_WaterLevel_Percent(uint16_t actual_distance);

#endif
