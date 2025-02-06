/*SSD1306 C语言驱动库
 *仅支持I2C方式
 *仅支持默认屏幕方向
 *使用1KB RAM作为显存，更新时全部上传
 *允许的运行方法：
    传统阻塞式运行
    Protothread式的可重入函数
 *兼容SSD1315
 */
#ifndef __SSD1306_H__
#define __SSD1306_H__

#ifdef __cplusplus
extern C {
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "ascii_fonts.h"//字体库

/*****************************用户配置区开始***********************************/
/*SSD1306 I2C地址
 *根据芯片接线决定，具体参阅数据手册
 */
#ifndef SSD1306_I2C_ADDR
//#define SSD1306_I2C_ADDR         0x78
#define SSD1306_I2C_ADDR       0x7A
#endif

#ifndef SSD1306_WIDTH
#define SSD1306_WIDTH            128
#endif

#ifndef SSD1306_HEIGHT
#define SSD1306_HEIGHT           64
#endif

#define SSD1306_USE_PROTOTHREAD//配置与Protothread库一同使用

//I2C写入函数，需要用户提供
//参数为寄存器地址、写入数组的首地址、数据长度
#define SSD1306_ASYNC_TRANSMIT(reg,pdata,len,prior) ASYNC_I2C_Transmit(SSD1306_I2C_ADDR, reg, pdata, len, prior, &SSD1306.Online)
/***************************用户配置区结束*************************************/
#define SSD1306_COLOR_BLACK 0x00
#define SSD1306_COLOR_WHITE 0x01

#define SSD1306_CMD_REG     0x00
#define SSD1306_DAT_REG     0x40

/* Private SSD1306 structure */
typedef struct {
    uint16_t CurrentX;
    uint16_t CurrentY;
    uint8_t Inverted;
    uint8_t Initialized;
    uint8_t Online;
    uint8_t sendbuf[2];
} SSD1306_t;

//操作语法宏，方便添加freeRTOS之类的支持
#ifdef SSD1306_USE_PROTOTHREAD
    #define SSD1306_RET         char
    #define SSD1306_NOARG       struct pt *pt
    #define SSD1306_ARGS(...)   struct pt *pt, __VA_ARGS__
    #define SSD1306_EXEC(cond)  if(cond == 0) PT_YIELD(pt)
    #define SSD1306_UNTIL(cond) PT_WAIT_UNTIL(pt, cond)
    #define SSD1306_FUNC_BEGIN  PT_BEGIN(pt)
    #define SSD1306_FUNC_END    PT_END(pt)
#else
    #define SSD1306_RET         void
    #define SSD1306_NOARG       void
    #define SSD1306_ARGS(...)   __VA_ARGS__
    #define SSD1306_EXEC(cond)  cond
    #define SSD1306_UNTIL(cond) while(cond == 0)
    #define SSD1306_FUNC_BEGIN  {}
    #define SSD1306_FUNC_END    {}
#endif
    

/*******************************函数声明区*************************************/
/******************************内部操作函数************************************/
uint8_t SSD1306_WriteCommand(uint8_t command);
uint8_t SSD1306_WriteData(uint8_t data);
/*******************非阻塞函数（执行时间仍可能较长）***************************/
void SSD1306_ToggleInvert(void);//反色
void SSD1306_Fill(uint8_t Color);//全屏填充
void SSD1306_GotoXY(uint16_t x, uint16_t y);//设置光标位置（左上角）
void SSD1306_DrawPixel(uint16_t x, uint16_t y, uint8_t color);
char SSD1306_Putc(char ch, FontDef_t* Font, uint8_t color);
char SSD1306_Puts(char* str, FontDef_t* Font, uint8_t color);//打印字符串
char SSD1306_Printf(FontDef_t* PrintFont, uint8_t Printcolor, const char* SSD1306_format, ...);//直接打印输出

void SSD1306_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t c, uint8_t isdotted);//画线
void SSD1306_DrawRectangle();//空心矩形
void SSD1306_DrawBox();//实心矩形
void SSD1306_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint8_t c);
void SSD1306_Image(uint8_t *img, uint8_t frame, uint8_t x, uint8_t y);

//判断SSD1306是否已初始化过
uint8_t SSD1306_IsInitialized(void);


/*********************************阻塞相关函数*********************************/
//刷新屏幕
SSD1306_RET SSD1306_UpdateScreen(SSD1306_NOARG);
//设置亮度
SSD1306_RET SSD1306_SetIndensity(SSD1306_ARGS(uint8_t SSD1306_density));
//开启屏幕
SSD1306_RET SSD1306_ON(SSD1306_NOARG);
//关闭屏幕
SSD1306_RET SSD1306_OFF(SSD1306_NOARG);
//初始化屏幕
SSD1306_RET SSD1306_Init(SSD1306_NOARG);

#ifdef __cplusplus
}
#endif

#endif
