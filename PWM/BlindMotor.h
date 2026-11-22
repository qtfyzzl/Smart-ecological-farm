#ifndef __BLIND_MOTOR_H
#define __BLIND_MOTOR_H

#include "stm32f10x.h"

// 暴露出左右电机的独立速度变量，方便在 show.c 中修改
extern uint8_t g_BlindSpeed_Left;  // 左电机 (大滚筒) 速度
extern uint8_t g_BlindSpeed_Right; // 右电机 (小滚筒) 速度

// 卷帘电机初始化函数
void BlindMotor_Init(void);

// 卷帘动作控制函数
// Action: 0=停止，1=下降/合上，2=上升/拉开
void Blind_SetAction(uint8_t Action);

#endif
