#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f10x.h"

// ================= 调试参数配置区 =================
// 1. 遮阳布安全限速
#define SHADE_SPEED_MAX     100     // 最大允许速度 (0-100)
#define SHADE_SPEED_MIN     30      // 最小启动速度死区 (低于此值电机可能发热不转)

// 2. 硬件极性配置 (如果发现代码里的"正转"和实际机械方向反了，调换这两个值即可)
#define DIR_FORWARD         0       // 展开方向的 GPIO 状态
#define DIR_REVERSE         1       // 收起方向的 GPIO 状态
// ==================================================

void Pump_Init(void);
void Pump_SetPower(uint8_t Power);
void LED_SetBrightness(uint8_t Brightness);

// --- 新增电机接口 ---
void ExtraMotors_Init(void);
void Fan_SetSpeed(uint8_t Speed);
void ShadeMotor1_Set(int8_t Speed);
void ShadeMotor2_Set(int8_t Speed);
void Fan_Calibration_Test(uint8_t a);


#endif
