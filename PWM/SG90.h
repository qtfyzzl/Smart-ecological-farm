#ifndef __SG90_H
#define __SG90_H
#include "stm32f10x.h"

void PWMPB8_Init(void);
void PWM_SetCompare_PB8(uint16_t Compare);
void Servo_Init(void);                  // ²¹³äÉùÃ÷
void Servo_SetAngle(float Angle);       // ²¹³äÉùÃ÷

#endif
