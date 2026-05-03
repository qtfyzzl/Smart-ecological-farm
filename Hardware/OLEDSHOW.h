#ifndef __OLEDSHOW_H
#define __OLEDSHOW_H

#define OLEDSHOW_W_D0(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_12, (BitAction)(x)) // SCL 时钟线
#define OLEDSHOW_W_D1(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_13, (BitAction)(x)) // SDA 数据线
#define OLEDSHOW_W_RES(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_14, (BitAction)(x)) // RES 复位线
#define OLEDSHOW_W_DC(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_15, (BitAction)(x)) // DC  数据/命令
#define OLEDSHOW_W_CS(x)		GPIO_WriteBit(GPIOA, GPIO_Pin_8,  (BitAction)(x)) // CS  片选线

void OLEDSHOW_Init(void);
void OLEDSHOW_Clear(void);
void OLEDSHOW_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLEDSHOW_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLEDSHOW_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLEDSHOW_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLEDSHOW_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLEDSHOW_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLEDSHOW_DrawPoint(uint8_t x, uint8_t y, uint8_t mode);
void OLEDSHOW_show_picture(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *pic, uint8_t mode);

void OLEDSHOW_ShowCN(uint8_t Line, uint8_t Column, uint8_t Num, uint8_t mode);
void OLEDSHOW_ShowBMP(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bmp, uint8_t mode) ;

#endif
