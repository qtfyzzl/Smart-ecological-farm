#ifndef __SERIAL_H
#define __SERIAL_H

#include "stm32f10x.h"                  // 必须包含此头文件，编译器才能认识 USART_TypeDef
#include <stdint.h>
#include <stdio.h>

/* 三个串口的接收数组和标志位 */
extern char Serial_RxPacket_1[];
extern uint8_t Serial_RxFlag_1;

extern char Serial_RxPacket_2[];
extern uint8_t Serial_RxFlag_2;

extern char Serial_RxPacket_3[];
extern uint8_t Serial_RxFlag_3;

/* 核心函数声明 (注意：全部增加了 USART_TypeDef* 参数) */
void Serial_Init(void);
void Serial_SendByte(USART_TypeDef* USARTx, uint8_t Byte);
void Serial_SendArray(USART_TypeDef* USARTx, uint8_t *Array, uint16_t Length);
void Serial_SendString(USART_TypeDef* USARTx, char *String);
void Serial_SendNumber(USART_TypeDef* USARTx, uint32_t Number, uint8_t Length);
void Serial_Printf(USART_TypeDef* USARTx, char *format, ...);
void Serial_SendScreen_String(USART_TypeDef* USARTx, char* name, char* str);
void Serial_SendScreen_Number(USART_TypeDef* USARTx, char* name, int num);
void Serial_SendScreen_float(USART_TypeDef* USARTx, char* name, float num);
void Process_Serial_Command(USART_TypeDef* USARTx, char* RxPacket, uint8_t PortNum);

#endif
