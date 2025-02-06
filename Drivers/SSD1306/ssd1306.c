#include "ssd1306.h"

//显存定义
__attribute__ ((aligned(4))) uint8_t SSD1306_Buffer_all[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

//参数相关结构体
static SSD1306_t SSD1306;

uint8_t SSD1306_IsInitialized(){
    return SSD1306.Initialized;
}

uint8_t SSD1306_IsOnline(){
    return SSD1306.Online;
}

void SSD1306_ToggleInvert(void) 
{
    uint16_t i;
    
    /* Toggle invert */
    SSD1306.Inverted = !SSD1306.Inverted;
    
    /* Do memory toggle */
    for (i = 0; i < sizeof(SSD1306_Buffer_all); i++)
    {
        SSD1306_Buffer_all[i] = ~SSD1306_Buffer_all[i];
    }
}

void SSD1306_Fill(uint8_t color){
    if (SSD1306.Inverted) color = (uint8_t)!color;
    memset(SSD1306_Buffer_all, (color == SSD1306_COLOR_BLACK) ? 0x00 : 0xFF, SSD1306_WIDTH * SSD1306_HEIGHT / 8);//Set memory
}

void SSD1306_DrawPixel(uint16_t x, uint16_t y, uint8_t color)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;//Error

    /* Check if pixels are inverted */
    if (SSD1306.Inverted)
    {
        color = (uint8_t)!color;
    }

    /* Set color */
    if (color == SSD1306_COLOR_WHITE) SSD1306_Buffer_all[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
    else SSD1306_Buffer_all[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
}

void SSD1306_GotoXY(uint16_t x, uint16_t y)
{
    /* Set write pointers */
    SSD1306.CurrentX = x;
    SSD1306.CurrentY = y;
}

char SSD1306_Putc(char ch, FontDef_t* font, uint8_t color)
{
    uint32_t i, b, j, k;

    for (i = 0; i < font->height; i++)
    {
        for (j = 0; j < font->bytes; j++)
        {
            b = font->data[((ch - 32) * font->height + i) * font->bytes + j];
            if (font->order == 0)
            {
                for (k = 0; k < 8 && k < font->width - j * 8; k++)
                {
                    if ((b << k) & 0x80)
                    {
                        SSD1306_DrawPixel(SSD1306.CurrentX + (j * 8) + k, (SSD1306.CurrentY + i), (uint8_t) color);
                    }
                    else
                    {
                        SSD1306_DrawPixel(SSD1306.CurrentX + (j * 8) + k, (SSD1306.CurrentY + i), (uint8_t) !color);
                    }
                }
            }
            else
            {
                for (k = 0; k < 8 && k < font->width - j * 8; k++)
                {
                    if (b & (0x0001 << k))
                    {
                        SSD1306_DrawPixel(SSD1306.CurrentX + (j * 8) + k, (SSD1306.CurrentY + i), (uint8_t) color);
                    }
                    else
                    {
                        SSD1306_DrawPixel(SSD1306.CurrentX + (j * 8) + k, (SSD1306.CurrentY + i), (uint8_t) !color);
                    }
                }
            }
        }
    }

    /* Increase pointer */
    SSD1306.CurrentX += font->width;

    /* Return character written */
    return ch;
}

char SSD1306_Puts(char* str, FontDef_t* Font, uint8_t color)
{
    /* Write characters */
    while (*str)
    {
        /* Write character by character */
        if (SSD1306_Putc(*str, Font, color) != *str)
        {
            /* Return error */
            return *str;
        }
        
        /* Increase string pointer */
        str++;
    }
    
    /* Everything OK, zero should be returned */
    return *str;
}

char SSD1306_Printf(FontDef_t* PrintFont, uint8_t Printcolor, const char* SSD1306_format, ...){
    char SSD1306_buf[44];
    va_list SSD1306_argptr;
    va_start(SSD1306_argptr, SSD1306_format);
    vsprintf(SSD1306_buf, SSD1306_format, SSD1306_argptr);
    va_end(SSD1306_argptr);
    return SSD1306_Puts(SSD1306_buf, PrintFont, Printcolor);
}


void SSD1306_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t c, uint8_t isdotted){
    int16_t dx, dy, sx, sy, err, e2, i, tmp; 
    
    /* Check for overflow */
    if (x0 >= SSD1306_WIDTH){
        x0 = SSD1306_WIDTH - 1;
    }
    if (x1 >= SSD1306_WIDTH){
        x1 = SSD1306_WIDTH - 1;
    }
    if (y0 >= SSD1306_HEIGHT){
        y0 = SSD1306_HEIGHT - 1;
    }
    if (y1 >= SSD1306_HEIGHT){
        y1 = SSD1306_HEIGHT - 1;
    }
    
    dx = (x0 < x1) ? (x1 - x0) : (x0 - x1); 
    dy = (y0 < y1) ? (y1 - y0) : (y0 - y1); 
    if(isdotted){
        sx = (x0 < x1) ? 2 : -2; 
        sy = (y0 < y1) ? 2 : -2; 
    }
    else{
        sx = (x0 < x1) ? 1 : -1; 
        sy = (y0 < y1) ? 1 : -1; 
    }
    err = ((dx > dy) ? dx : -dy) / 2; 

    if (dx == 0){//Vertical line
        for (i = y0; i <= y1; i += sy){
            SSD1306_DrawPixel(x0, i, c);
        }
        return;
    }
    
    if (dy == 0){//Horizontal line
        for (i = x0; i <= x1; i += sx){
            SSD1306_DrawPixel(i, y0, c);
        }
        return;
    }

    while (1){
        SSD1306_DrawPixel(x0, y0, c);
        if (x0 == x1 && y0 == y1)
        {
            break;
        }
        e2 = err;
        if (e2 > -dx)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dy)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void SSD1306_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint8_t c)
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    SSD1306_DrawPixel(x0, y0 + r, c);
    SSD1306_DrawPixel(x0, y0 - r, c);
    SSD1306_DrawPixel(x0 + r, y0, c);
    SSD1306_DrawPixel(x0 - r, y0, c);

    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        SSD1306_DrawPixel(x0 + x, y0 + y, c);
        SSD1306_DrawPixel(x0 - x, y0 + y, c);
        SSD1306_DrawPixel(x0 + x, y0 - y, c);
        SSD1306_DrawPixel(x0 - x, y0 - y, c);

        SSD1306_DrawPixel(x0 + y, y0 + x, c);
        SSD1306_DrawPixel(x0 - y, y0 + x, c);
        SSD1306_DrawPixel(x0 + y, y0 - x, c);
        SSD1306_DrawPixel(x0 - y, y0 - x, c);
    }
}
//img逐行取模，第一个数据为宽，第二个为高
void SSD1306_Image(uint8_t *img, uint8_t frame, uint8_t x, uint8_t y){
    uint16_t i, j, b = 0;
    for (i = 0; i < img[1]; i++) {
        for (j = 0; j < img[0]; j++) {
            SSD1306_DrawPixel(x + j, (y + i), (uint8_t)(img[b/8U + 2] >> (b%8)) & 1U);
            b++;
        }
    }
}

const uint8_t SSD1306_Init_Array[] = {
    0xAE, //0xAE, Display OFF (sleep mode)
    0x00, //---set low column address
    0x10, //---set high column address    
    0x81,
    0X3F, //set contrast control register, 2 bytes, 0x00 - 0xFF
    0xA4, //Output follows RAM content
    0xA6, //Normal display (RESET)
    0x20, //Set Memory Addressing Mode, 2 bytes 
    0x00, //Horizontal Addressing Mode (slide horizontally and goto next page)
    0xB0, //Set Page Address, 3 bytes,For Horizontal and Vertical Addressing Mode only
    0x22,
    0x00, // From Page 0
    0x07, // To Page 7
        /** 
         * COM Output Scan Direction
         * 0xC0: normal mode (RESET) Scan from COM0 to COM[N –1]
         * 0xC8: remapped mode. Scan from COM[N-1] to COM0 */
    0xC8, //Set COM Output Scan Direction
        /**
         * Set display RAM display start line register from 0-63 */
    0x40,
        /**
         * Segment Re-map
         * 0xA0: column address 0 is mapped to SEG0 (RESET),
         * 0xA1: column address 127 is mapped to SEG0 */
    0xA1,
        /**
         * Set MUX ratio to N+1 MUX
         * N=A[5:0]: from 16MUX to 64MUX, RESET=111111b (i.e. 63d, 64MUX)
         * A[5:0] from 0 to 14 are invalid entry.*/
    0xA8,
    0x3F,
        /** 
         * Set Display Offset, Set vertical shift by COM from 0d~63d
         * The value is reset to 00h after RESET */
    0xD3,
    0x00, // offset in vertical
        /**
         * Set COM Pins Hardware Configuration
         * A[4]=0b, Sequential COM pin configuration
         * A[4]=1b(RESET), Alternative COM pin configuration
         * A[5]=0b(RESET), Disable COM Left/Right remap
         * A[5]=1b, Enable COM Left/Right remap */
    0xDA,
    0x12, // A[4]=0, A[5]=1
    0xD5, //Set Display Divide Ratio/Oscillator Frequency
    0xF0, // divide ratio
    0xD9, //Set Pre-charge Period 
    0x22,
    0xDB, //Set V COMH Deselect Level
    0x10, //0.77 * Vcc (RESET)
    0x8D, //charge pump setting
    0x14, //Enable charge pump during display on
    0xAF //Display ON in normal mode
};

SSD1306_RET SSD1306_Init(SSD1306_NOARG) {
    SSD1306_FUNC_BEGIN;
    PT_SEM_WAIT(pt, &i2c_mutex);
    if(SSD1306.Initialized == 1) return 1;//已完成配置
    SSD1306_EXEC(SSD1306_ASYNC_TRANSMIT(SSD1306_CMD_REG, (uint8_t*)SSD1306_Init_Array, sizeof(SSD1306_Init_Array)/sizeof(uint8_t), 0));
    SSD1306_UNTIL(SSD1306.Online);
    SSD1306_Fill(SSD1306_COLOR_BLACK);
    SSD1306.CurrentX = 0;
    SSD1306.CurrentY = 0;
    SSD1306.Initialized = 1;
    PT_SEM_SIGNAL(pt, &i2c_mutex);
    SSD1306_FUNC_END;
}

SSD1306_RET SSD1306_UpdateScreen(SSD1306_NOARG) {
    SSD1306_FUNC_BEGIN;
    PT_SEM_WAIT(pt, &i2c_mutex);
    SSD1306_EXEC(SSD1306_ASYNC_TRANSMIT(SSD1306_DAT_REG, SSD1306_Buffer_all, SSD1306_WIDTH * SSD1306_HEIGHT /8, 7));
    SSD1306_UNTIL(SSD1306.Online);
    PT_SEM_SIGNAL(pt, &i2c_mutex);
    SSD1306_FUNC_END;
}

uint8_t SSD1306_Indensity_Array[] = {0x81, 0x00};
SSD1306_RET SSD1306_SetIndensity(SSD1306_ARGS(uint8_t SSD1306_density)){
    SSD1306_FUNC_BEGIN;
    PT_SEM_WAIT(pt, &i2c_mutex);
    SSD1306_Indensity_Array[1] = SSD1306_density;
    SSD1306_EXEC(SSD1306_ASYNC_TRANSMIT(SSD1306_CMD_REG, SSD1306_Indensity_Array, 2, 0));
    SSD1306_UNTIL(SSD1306.Online);
    PT_SEM_SIGNAL(pt, &i2c_mutex);
    SSD1306_FUNC_END;
}

const uint8_t SSD1306_Wake_Array[] = {0x8D, 0x14, 0xAF};//开电荷泵，开屏幕
SSD1306_RET SSD1306_ON(SSD1306_NOARG){
    SSD1306_FUNC_BEGIN;
    PT_SEM_WAIT(pt, &i2c_mutex);
    SSD1306_EXEC(SSD1306_ASYNC_TRANSMIT(SSD1306_CMD_REG, (uint8_t*)SSD1306_Wake_Array, sizeof(SSD1306_Wake_Array)/sizeof(uint8_t), 0));
    SSD1306_UNTIL(SSD1306.Online);
    PT_SEM_SIGNAL(pt, &i2c_mutex);
    SSD1306_FUNC_END;
}

const uint8_t SSD1306_Sleep_Array[] = {0x8D, 0x10, 0xAE};//关电荷泵，关屏幕
SSD1306_RET SSD1306_OFF(SSD1306_NOARG){
    SSD1306_FUNC_BEGIN;
    PT_SEM_WAIT(pt, &i2c_mutex);
    SSD1306_EXEC(SSD1306_ASYNC_TRANSMIT(SSD1306_CMD_REG, (uint8_t*)SSD1306_Sleep_Array, sizeof(SSD1306_Sleep_Array)/sizeof(uint8_t), 0));
    SSD1306_UNTIL(SSD1306.Online);
    PT_SEM_SIGNAL(pt, &i2c_mutex);
    SSD1306_FUNC_END;
}
