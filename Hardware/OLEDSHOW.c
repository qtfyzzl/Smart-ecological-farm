#include "stm32f10x.h"
#include "OLEDSHOW_Font.h"
#include "OLEDSHOW.h"

/* ===================== 4线SPI 引脚定义（严格按你的要求） ===================== */
#define OLEDSHOW_W_D0(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_12, (BitAction)(x)) // SCL 时钟
#define OLEDSHOW_W_D1(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_13, (BitAction)(x)) // SDA 数据
#define OLEDSHOW_W_RES(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_14, (BitAction)(x)) // RES 复位
#define OLEDSHOW_W_DC(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_15, (BitAction)(x)) // DC 命令/数据
#define OLEDSHOW_W_CS(x)		GPIO_WriteBit(GPIOA, GPIO_Pin_8,  (BitAction)(x)) // CS 片选

/* 显存缓冲区（无需修改） */
static uint8_t OLEDSHOW_Buffer[8][128]; // 8页，每页128列
static uint8_t current_page = 0;    // 当前页
static uint8_t current_col = 0;     // 当前列

/* ===================== SPI 引脚初始化 ===================== */
void OLEDSHOW_SPI_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    // 初始化 PB12/13/14/15
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    // 初始化 PA8
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
    // 初始电平
    OLEDSHOW_W_CS(1);   // 片选拉高，不选中
    OLEDSHOW_W_DC(1);   // 默认数据模式
    OLEDSHOW_W_RES(1);  // 复位脚拉高
    OLEDSHOW_W_D0(1);   // 时钟默认高
}

/* ===================== SPI 发送一个字节（模拟时序） ===================== */
static void OLEDSHOW_SPI_SendByte(uint8_t Byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        OLEDSHOW_W_D0(0);               // 时钟拉低
        OLEDSHOW_W_D1(Byte & 0x80);      // 发送最高位
        Byte <<= 1;                  // 数据左移
        OLEDSHOW_W_D0(1);               // 时钟拉高，采样数据
    }
}

/* ===================== OLEDSHOW 写命令 ===================== */
void OLEDSHOW_WriteCommand(uint8_t Command)
{
    OLEDSHOW_W_CS(0);       // 拉低片选，选中OLEDSHOW
    OLEDSHOW_W_DC(0);       // DC=0，命令模式
    OLEDSHOW_SPI_SendByte(Command);
    OLEDSHOW_W_CS(1);       // 拉高片选，取消选中
}

/* ===================== OLEDSHOW 写数据 ===================== */
void OLEDSHOW_WriteData(uint8_t Data)
{
    OLEDSHOW_W_CS(0);       // 拉低片选
    OLEDSHOW_W_DC(1);       // DC=1，数据模式
    OLEDSHOW_SPI_SendByte(Data);
    OLEDSHOW_W_CS(1);       // 拉高片选
    
    // 更新缓冲区和列地址（无需修改）
    OLEDSHOW_Buffer[current_page][current_col] = Data;
    current_col++;
    if (current_col >= 128) {
        current_col = 0;
    }
}

/* ===================== 设置光标（无需修改） ===================== */
void OLEDSHOW_SetCursor(uint8_t Y, uint8_t X)
{
    current_page = Y;
    current_col = X;
    OLEDSHOW_WriteCommand(0xB0 | Y);                // 设置页地址
    OLEDSHOW_WriteCommand(0x10 | ((X & 0xF0) >> 4)); // 列高4位
    OLEDSHOW_WriteCommand(0x00 | (X & 0x0F));         // 列低4位
}

/* ===================== 清屏（无需修改） ===================== */
void OLEDSHOW_Clear(void)
{  
    uint8_t i, j;
    for (j = 0; j < 8; j++) {
        OLEDSHOW_SetCursor(j, 0);
        for (i = 0; i < 128; i++) {
            OLEDSHOW_WriteData(0x00);
        }
    }
}

/* ===================== 显示字符（无需修改） ===================== */
void OLEDSHOW_ShowChar(uint8_t Line, uint8_t Column, char Char)
{      	
    uint8_t i;
    OLEDSHOW_SetCursor((Line - 1) * 2, (Column - 1) * 8);
    for (i = 0; i < 8; i++)
    {
        OLEDSHOW_WriteData(OLEDSHOW_F8x16[Char - ' '][i]);
    }
    OLEDSHOW_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);
    for (i = 0; i < 8; i++)
    {
        OLEDSHOW_WriteData(OLEDSHOW_F8x16[Char - ' '][i + 8]);
    }
}

/* ===================== 显示字符串（无需修改） ===================== */
void OLEDSHOW_ShowString(uint8_t Line, uint8_t Column, char *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i++)
    {
        OLEDSHOW_ShowChar(Line, Column + i, String[i]);
    }
}

/* ===================== 次方函数（无需修改） ===================== */
uint32_t OLEDSHOW_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--)
    {
        Result *= X;
    }
    return Result;
}

/* ===================== 显示数字（无需修改） ===================== */
void OLEDSHOW_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)							
    {
        OLEDSHOW_ShowChar(Line, Column + i, Number / OLEDSHOW_Pow(10, Length - i - 1) % 10 + '0');
    }
}

/* ===================== 显示有符号数字（无需修改） ===================== */
void OLEDSHOW_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
    uint8_t i;
    uint32_t Number1;
    if (Number >= 0)
    {
        OLEDSHOW_ShowChar(Line, Column, '+');
        Number1 = Number;
    }
    else
    {
        OLEDSHOW_ShowChar(Line, Column, '-');
        Number1 = -Number;
    }
    for (i = 0; i < Length; i++)							
    {
        OLEDSHOW_ShowChar(Line, Column + i + 1, Number1 / OLEDSHOW_Pow(10, Length - i - 1) % 10 + '0');
    }
}

/* ===================== 显示十六进制数（无需修改） ===================== */
void OLEDSHOW_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i, SingleNumber;
    for (i = 0; i < Length; i++)							
    {
        SingleNumber = Number / OLEDSHOW_Pow(16, Length - i - 1) % 16;
        if (SingleNumber < 10)
        {
            OLEDSHOW_ShowChar(Line, Column + i, SingleNumber + '0');
        }
        else
        {
            OLEDSHOW_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
        }
    }
}

/* ===================== 显示二进制数（无需修改） ===================== */
void OLEDSHOW_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)							
    {
        OLEDSHOW_ShowChar(Line, Column + i, Number / OLEDSHOW_Pow(2, Length - i - 1) % 2 + '0');
    }
}

/* ===================== OLEDSHOW 初始化（SPI 硬件复位） ===================== */
void OLEDSHOW_Init(void)
{
    uint32_t i, j;
	
    // 上电延时
    for (i = 0; i < 1000; i++) {
        for (j = 0; j < 1000; j++);
    }
	
    OLEDSHOW_SPI_Init();		    // SPI端口初始化
	
    // ========== SPI 硬件复位 ==========
    OLEDSHOW_W_RES(0);
    for (i = 0; i < 1000; i++);
    OLEDSHOW_W_RES(1);
    for (i = 0; i < 1000; i++);
	
    // ========== OLEDSHOW 初始化命令（不变） ==========
    OLEDSHOW_WriteCommand(0xAE);	// 关闭显示
    OLEDSHOW_WriteCommand(0xD5);	// 时钟分频
    OLEDSHOW_WriteCommand(0x80);
    OLEDSHOW_WriteCommand(0xA8);	// 复用率
    OLEDSHOW_WriteCommand(0x3F);
    OLEDSHOW_WriteCommand(0xD3);	// 显示偏移
    OLEDSHOW_WriteCommand(0x00);
    OLEDSHOW_WriteCommand(0x40);	// 开始行
    OLEDSHOW_WriteCommand(0xA1);	// 左右方向
    OLEDSHOW_WriteCommand(0xC8);	// 上下方向
    OLEDSHOW_WriteCommand(0xDA);	// COM引脚配置
    OLEDSHOW_WriteCommand(0x12);
    OLEDSHOW_WriteCommand(0x81);	// 对比度
    OLEDSHOW_WriteCommand(0xCF);
    OLEDSHOW_WriteCommand(0xD9);	// 预充电
    OLEDSHOW_WriteCommand(0xF1);
    OLEDSHOW_WriteCommand(0xDB);	// VCOMH
    OLEDSHOW_WriteCommand(0x30);
    OLEDSHOW_WriteCommand(0xA4);	// 全局显示
    OLEDSHOW_WriteCommand(0xA6);	// 正常显示
    OLEDSHOW_WriteCommand(0x8D);	// 充电泵
    OLEDSHOW_WriteCommand(0x14);
    OLEDSHOW_WriteCommand(0xAF);	// 开启显示
		
    OLEDSHOW_Clear();				// 清屏
}

/* ===================== 显示汉字（无需修改） ===================== */
void OLEDSHOW_ShowCN(uint8_t Line, uint8_t Column, uint8_t Num, uint8_t mode)  
{      
    uint8_t i;
    uint8_t page_start = (Line - 1) * 2;
    uint8_t col_start = (Column - 1) * 16;

    OLEDSHOW_SetCursor(page_start, col_start);
    for (i = 0; i < 16; i++) 
    {
        uint8_t data = OLEDSHOW_F10x16[Num][i];
        OLEDSHOW_WriteData(mode ? data : ~data);
    }
    OLEDSHOW_SetCursor(page_start + 1, col_start);
    for (i = 16; i < 32; i++) 
    {
        uint8_t data = OLEDSHOW_F10x16[Num][i];
        OLEDSHOW_WriteData(mode ? data : ~data);
    }
}

/* ===================== 画点（无需修改） ===================== */
void OLEDSHOW_DrawPoint(uint8_t x, uint8_t y, uint8_t mode)
{
    if (x >= 128 || y >= 64) return;
    
    uint8_t page = y / 8;
    uint8_t bit_pos = y % 8;
    
    if (mode) {
        OLEDSHOW_Buffer[page][x] |= (1 << bit_pos);
    } else {
        OLEDSHOW_Buffer[page][x] &= ~(1 << bit_pos);
    }
    
    OLEDSHOW_SetCursor(page, x);
    OLEDSHOW_WriteData(OLEDSHOW_Buffer[page][x]);
}

/* ===================== 显示图片/位图（无需修改） ===================== */
void OLEDSHOW_ShowBMP(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bmp, uint8_t mode) 
{
    uint8_t page_end = y + height / 8;
    uint8_t col_end = x + width;
    uint16_t index = 0;
    
    for (uint8_t page = y; page < page_end; page++) 
    {
        OLEDSHOW_SetCursor(page, x);
        for (uint8_t col = x; col < col_end; col++) 
        {
            uint8_t data = bmp[index++];
            OLEDSHOW_WriteData(mode ? data : ~data);
        }
    }
}
