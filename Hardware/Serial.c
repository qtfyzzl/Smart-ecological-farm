#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <stdarg.h>
#include <oled.h>
#include "string.h"  

/* 为三个串口分别定义接收缓存和标志位 */
char Serial_RxPacket_1[100];            // USART1 接收数组
uint8_t Serial_RxFlag_1;

char Serial_RxPacket_2[100];            // USART2 接收数组
uint8_t Serial_RxFlag_2;

char Serial_RxPacket_3[100];            // USART3 接收数组
uint8_t Serial_RxFlag_3;

/**
  * 函   数：全部三个串口初始化
  * 参   数：无
  * 返 回 值：无
  */
void Serial_Init(void)
{
    /* 1. 开启时钟 */
    // USART1在APB2，USART2和USART3在APB1。PA和PB均在APB2
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 | RCC_APB1Periph_USART3, ENABLE);
    
    /* 2. GPIO初始化 */
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // USART1: PA9(TX) 复用推挽, PA10(RX) 上拉输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);                  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOA, &GPIO_InitStructure);                  

//    // USART2: PA2(TX) 复用推挽, PA3(RX) 上拉输入
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);                  
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);  
    
    // USART3: PB10(TX) 复用推挽, PB11(RX) 上拉输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOB, &GPIO_InitStructure);                  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_Init(GPIOB, &GPIO_InitStructure);  
    
    /* 3. USART初始化配置 (三个串口统一配置为9600-8-N-1) */
    USART_InitTypeDef USART_InitStructure;                  
    USART_InitStructure.USART_BaudRate = 9600;              
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; 
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; 
    USART_InitStructure.USART_Parity = USART_Parity_No;     
    USART_InitStructure.USART_StopBits = USART_StopBits_1;  
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;     
    
    USART_Init(USART1, &USART_InitStructure);               
    USART_Init(USART2, &USART_InitStructure);
    USART_Init(USART3, &USART_InitStructure);
    
    /* 4. 中断输出配置 */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);          
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); 
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); 
    
    /* 5. NVIC中断分组 (整个工程只需配置一次，分组2) */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);         
    
    /* 6. NVIC配置 */
    NVIC_InitTypeDef NVIC_InitStructure;                    
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;         
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;       // 抢占优先级统一为1
    
    // USART1中断配置
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;       
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;      // 响应优先级1
    NVIC_Init(&NVIC_InitStructure);                         

//    // USART2中断配置
//    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;       
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;      // 响应优先级2
//    NVIC_Init(&NVIC_InitStructure);   

    // USART3中断配置
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;       
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;      // 响应优先级3
    NVIC_Init(&NVIC_InitStructure);   
    
    /* 7. USART使能 */
    USART_Cmd(USART1, ENABLE);                              
    USART_Cmd(USART2, ENABLE);
    USART_Cmd(USART3, ENABLE);
}

/**
  * 函   数：串口发送一个字节 (增加USARTx参数)
  */
void Serial_SendByte(USART_TypeDef* USARTx, uint8_t Byte)
{
    USART_SendData(USARTx, Byte);       
    while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);   
}

/**
  * 函   数：串口发送一个数组 (增加USARTx参数)
  */
void Serial_SendArray(USART_TypeDef* USARTx, uint8_t *Array, uint16_t Length)
{
    uint16_t i;
    for (i = 0; i < Length; i ++)       
    {
        Serial_SendByte(USARTx, Array[i]);      
    }
}

/**
  * 函   数：串口发送一个字符串 (增加USARTx参数)
  */
void Serial_SendString(USART_TypeDef* USARTx, char *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i ++)
    {
        Serial_SendByte(USARTx, String[i]);     
    }
}

/**
  * 函   数：次方函数（内部使用）
  */
uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;    
    while (Y --)            
    {
        Result *= X;        
    }
    return Result;
}

/**
  * 函   数：串口发送数字 (增加USARTx参数)
  */
void Serial_SendNumber(USART_TypeDef* USARTx, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i ++)       
    {
        Serial_SendByte(USARTx, Number / Serial_Pow(10, Length - i - 1) % 10 + '0');    
    }
}

/**
  * 函   数：使用printf需要重定向的底层函数 (默认只映射到USART1用于调试)
  */
int fputc(int ch, FILE *f)
{
    Serial_SendByte(USART1, ch);            
    return ch;
}

/**
  * 函   数：自己封装的prinf函数 (增加USARTx参数，可向指定串口打印)
  */
void Serial_Printf(USART_TypeDef* USARTx, char *format, ...)
{
    char String[256]; // <--- 必须把这里的 100 改成 256
    va_list arg;
    va_start(arg, format);
    vsprintf(String, format, arg);
    va_end(arg);
    Serial_SendString(USARTx, String);
}
/* ================== 下方为三个串口的独立中断函数 ================== */

void USART1_IRQHandler(void)
{
    static uint8_t RxState = 0;     
    static uint8_t pRxPacket = 0;   
    if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)    
    {
        uint8_t RxData = USART_ReceiveData(USART1);         
        
        if (RxState == 0) {
            if (RxData == '@' && Serial_RxFlag_1 == 0) {
                RxState = 1;            
                pRxPacket = 0;          
            }
        }
        else if (RxState == 1) {
            if (RxData == '\r') {
                RxState = 2;            
            } else {
                Serial_RxPacket_1[pRxPacket] = RxData;      
                pRxPacket ++;           
            }
        }
        else if (RxState == 2) {
            if (RxData == '\n') {
                RxState = 0;            
                Serial_RxPacket_1[pRxPacket] = '\0';            
                Serial_RxFlag_1 = 1;        
            }
        }
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);     
    }
}

void USART2_IRQHandler(void)
{
    static uint8_t RxState = 0;     
    static uint8_t pRxPacket = 0;   
    if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)    
    {
        uint8_t RxData = USART_ReceiveData(USART2);         
        
        if (RxState == 0) {
            if (RxData == '@' && Serial_RxFlag_2 == 0) {
                RxState = 1;            
                pRxPacket = 0;          
            }
        }
        else if (RxState == 1) {
            if (RxData == '\r') {
                RxState = 2;            
            } else {
                Serial_RxPacket_2[pRxPacket] = RxData;      
                pRxPacket ++;           
            }
        }
        else if (RxState == 2) {
            if (RxData == '\n') {
                RxState = 0;            
                Serial_RxPacket_2[pRxPacket] = '\0';            
                Serial_RxFlag_2 = 1;        
            }
        }
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);     
    }
}

void USART3_IRQHandler(void)
{
    static uint8_t RxState = 0;     
    static uint8_t pRxPacket = 0;   
    if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET)    
    {
        uint8_t RxData = USART_ReceiveData(USART3);         
        
        if (RxState == 0) {
            if (RxData == '@' && Serial_RxFlag_3 == 0) {
                RxState = 1;            
                pRxPacket = 0;          
            }
        }
        else if (RxState == 1) {
            if (RxData == '\r') {
                RxState = 2;            
            } else {
                Serial_RxPacket_3[pRxPacket] = RxData;      
                pRxPacket ++;           
            }
        }
        else if (RxState == 2) {
            if (RxData == '\n') {
                RxState = 0;            
                Serial_RxPacket_3[pRxPacket] = '\0';            
                Serial_RxFlag_3 = 1;        
            }
        }
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);     
    }
}
/**
  * 函    数：串口屏发送字符串参数
  * 参    数：USARTx：指定发送的串口 (如 USART1, USART2, USART3)
  * 参    数：name：串口屏上的变量名或控件属性 (如 "t0.txt")
  * 参    数：str：要修改的字符串内容
  * 返 回 值：无
  */
void Serial_SendScreen_String(USART_TypeDef* USARTx, char* name, char* str)
{
    char String[100];               
    // 按照串口屏要求的格式拼接字符串，末尾带上3个 0xFF 结束符
    sprintf(String, "%s=\"%s\"\xff\xff\xff", name, str); 
    Serial_SendString(USARTx, String);
}
void Serial_SendScreen_float(USART_TypeDef* USARTx, char* name,float num)
{		char String[100];
	sprintf(String, "%s=\"%.2f\"\xff\xff\xff", name, num);
	Serial_SendString(USARTx, String);
}

/**
  * 函    数：串口屏发送数字参数
  * 参    数：USARTx：指定发送的串口 (如 USART1, USART2, USART3)
  * 参    数：name：串口屏上的变量名或控件属性 (如 "n0.val" 或 "page")
  * 参    数：num：要修改的数字值
  * 返 回 值：无
  */
void Serial_SendScreen_Number(USART_TypeDef* USARTx, char* name, int num)
{
    char String[100];               
    // 数字类型不需要加双引号，末尾带上3个 0xFF 结束符
    sprintf(String, "%s=%d\xff\xff\xff", name, num);
    Serial_SendString(USARTx, String);
}
/**
  * 函    数：处理串口指令的核心逻辑封装
  * 参    数：USARTx：当前触发接收的串口号 (如 USART1, USART2, USART3)
  * 参    数：RxPacket：当前串口接收到的字符串数组内容
  * 参    数：PortNum：用于在OLED上显示的串口编号标志 (1, 2, 3)
  * 说    明：所有串口收到数据后都会调用此函数，实现统一的指令解析和回复
  */
void Process_Serial_Command(USART_TypeDef* USARTx, char* RxPacket, uint8_t PortNum)
{
    char OLED_Buffer[17]; // 定义OLED单行显存缓存（OLED一行最多16个字符 + 1个'\0'结束符）
    
    // --- 步骤1：在屏幕第4行实时显示刚刚接收到的原始指令 ---
    OLED_ShowString(4, 1, "                "); // 先用16个空格清除第4行旧数据
    OLED_ShowString(4, 1, RxPacket);           // 显示最新收到的字符串内容
    
    // --- 步骤2：指令比对与执行 ---
    /* 如果收到 "LED_ON" 指令 */
    if (strcmp(RxPacket, "LED_ON") == 0)
    {
//        LED1_ON();                                  // 硬件动作：点亮LED
        Serial_SendString(USARTx, "LED_ON_OK\r\n"); // 软件反馈：向发送指令的对应串口原路返回成功信息
        
        // 屏幕反馈：在第2行显示操作结果和来源串口（如 "P1: LED_ON_OK"）
        sprintf(OLED_Buffer, "P%d: LED_ON_OK", PortNum);
        OLED_ShowString(2, 1, "                ");  // 清除第2行
        OLED_ShowString(2, 1, OLED_Buffer);         // 刷新第2行
    }
    /* 如果收到 "LED_OFF" 指令 */
    else if (strcmp(RxPacket, "LED_OFF") == 0)
    {
//        LED1_OFF();                                 // 硬件动作：熄灭LED
        Serial_SendString(USARTx, "LED_OFF_OK\r\n");// 软件反馈：原路返回
        
        // 屏幕反馈
        sprintf(OLED_Buffer, "P%d: LED_OFF_OK", PortNum);
        OLED_ShowString(2, 1, "                ");
        OLED_ShowString(2, 1, OLED_Buffer);
    }
    /* 收到了未定义的指令（例如拼写错误或乱码） */
    else
    {
        Serial_SendString(USARTx, "ERROR_COMMAND\r\n");// 返回错误提示
        
        // 屏幕反馈
        sprintf(OLED_Buffer, "P%d: CMD_ERR", PortNum);
        OLED_ShowString(2, 1, "                ");
        OLED_ShowString(2, 1, OLED_Buffer);
    }
}

