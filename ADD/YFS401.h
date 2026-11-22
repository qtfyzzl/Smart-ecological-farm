#ifndef __YFS401_H
#define __YFS401_H
#include "stm32f10x.h"

typedef unsigned char uint8_t;

// --- 已更改为 PB1 配置 ---
#define YFS201_GPIO_CLOCK      RCC_APB2Periph_GPIOB    // 更改为 GPIOB
#define YFS201_GPIO_PORT       GPIOB                   // 更改为 GPIOB
#define YFS201_GPIO_PIN        GPIO_Pin_1              // 更改为 Pin 1
#define YFS201_EXTI_LINE       EXTI_Line1              // 更改为 Line 1
#define YFS201_GPIO_PORTSOURCE GPIO_PortSourceGPIOB    // 更改为 B 端口源
#define YFS201_GPIO_PINSOURCE  GPIO_PinSource1         // 更改为 Pin 1 源
#define YFS201_EXTI_IRQn       EXTI1_IRQn              // 重要：PB1 使用独立的 EXTI1 中断

// YF-S401 规格参数
#define YFS201_PULSES_PER_LITER  (5880UL)

void YFS201_Init(void);
uint32_t YFS201_GetTotalMl(void);
uint32_t YFS201_GetFlowMlMin(void);
uint32_t YFS201_GetPulseCount(void);
void YFS201_ResetTotal(void);
void yfs401_con(void);

#endif
