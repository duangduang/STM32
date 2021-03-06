//JLX12864G-378,串行接口
//驱动IC：ST7565R
//维护日期：2014.3.2
//维护人员：金雨鑫、王恩策
#include "stm32f10x.h"
#include "sys.h"
#include "delay.h"
#include "JLX12864G.h"
#include <string.h>
#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"
#ifdef USING_UCGUI
u32 OLED_BUF[64][4];
#endif

//* 整屏最多容纳 8*4 个汉字 ,16*4  个大字,21*8 个小字
//! printf函数需勾选microLib
/***********无UCGUI***********/
/*

Example:
	OLED_Init(); //同时清屏
	OLED_printfAt(1,1,"pi=%g",3.1415926);

*/

/***********有UCGUI****************/
/*
需添加lib文件
Example:
	GUI_Init();
	OLED_printf("hello\nworld");
\_____________________________________________/
/---------------------------------------------\

常用函数：	详见文档或调试跳转或GUI.h等头文件

//显示
	void OLED_printf (char *fmt, ...);//在当前坐标显示格式化字符串
	
	GUI_DispChar() 在当前坐标显示单个字符 
	GUI_DispCharAt() 在指定坐标显示单个字符 
	GUI_DispChars() 按指定重复次数显示一个字符 
	
	GUI_DispString() 在当前坐标显示字符串 
	GUI_DispStringAt() 在指定坐标显示字符串 
	GUI_DispStringAtCEOL() 在指定坐标显示字符串，并清除到行末 
	GUI_DispStringLen() 在当前坐标显示指定字符数量的字符串 
	
//设置：
	GUI_SetTextMode() 设置文本绘图模式 
	GUI_GetTextAlign() 返回当前文本对齐模式 
	GUI_SetLBorder() 设置换行后的左边界 
	
//坐标：
	GUI_GotoX() 设置当前X坐标 
	GUI_GotoXY() 设置当前X、Y坐标 
	GUI_GotoY() 设置当前Y坐标 
	GUI_GetDispPosX() 返回当前X坐标 
	GUI_GetDispPosY() 返回当前Y坐标 
	
//清除：
	GUI_Clear() 清除活动视窗
	GUI_DispCEOL() 清除从当前坐标到行末的显示内容
	
//图形：
	GUI_SetDrawMode() 设置绘图模式（设置为GUI_DRAWMODE_XOR时，重叠部分取反）
	GUI_ClearRect() 使用背景颜色填充一个矩形区域  
	GUI_DrawPixel() 绘一个单像素点  
	GUI_DrawPoint() 绘一个点  
	GUI_FillRect() 绘一个填充的矩形  
	GUI_InvertRect() 反转一个矩形区域  
	GUI_DrawBitmap() 绘制一幅位图  
	GUI_DrawBitmapExp() 绘制一幅位图  
	GUI_DrawBitmapMag() 绘制一幅放大的位图  
	GUI_DrawStreamedBitmap() 从一个位图数据流的数据绘制一幅位图  
	GUI_DrawHLine() 绘一根水平线  
	GUI_DrawLine() 绘一根线  
	GUI_DrawLineRel() 从当前坐标到端点绘一根线，该端点由X轴距
	GUI_DrawLineTo() 从当前坐标到端点(X,Y)绘一根线  
	GUI_DrawPolyLine() 绘折线  
	GUI_DrawVLine() 绘一根垂直线  
	GUI_DrawPolygon() 绘一个多边形  
	GUI_EnlargePolygon() 对一个多边形进行扩边  
	GUI_FillPolygon() 绘一个填充的多边形  
	GUI_MagnifyPolygon() 放大一个多边形  
	GUI_RotatePolygon() 按指定角度旋转一个多边形  
	GUI_DrawCircle() 绘一个圆  
	GUI_FillCircle() 绘一个填充的圆  
	GUI_DrawEllipse() 绘一个椭圆  
	GUI_FillEllipse() 绘一个填充的椭圆
//窗口：
	详见文档
*/
//***********************************
// IO口宏定义 使用时请修改成自己的
//***********************************
//#define OLED_rs		   4           //RS    
//#define OLED_reset     5           //RST
//#define OLED_cs        6           //CS
//#define OLED_sid       7           //SDA数据
//#define OLED_sclk      8           //CLK时钟

//#define GPIO_rs       GPIO_Pin_4
//#define GPIO_reset    GPIO_Pin_5
//#define GPIO_cs       GPIO_Pin_6
//#define GPIO_sid      GPIO_Pin_7
//#define GPIO_sclk     GPIO_Pin_8

//#define OLED_Port RCC_APB2Periph_GPIOA
//#define OLED_GPIO GPIOA
//#define set(bitx)	PAout(bitx)=1
//#define clr(bitx) PAout(bitx)=0   //这几个口用同一个PORT的

#define OLED_rs		   4           //RS    
#define OLED_reset     5           //RST
#define OLED_cs        6           //CS
#define OLED_sid       7           //SDA数据
#define OLED_sclk      8           //CLK时钟

#define GPIO_rs       GPIO_Pin_4
#define GPIO_reset    GPIO_Pin_5
#define GPIO_cs       GPIO_Pin_6
#define GPIO_sid      GPIO_Pin_7
#define GPIO_sclk     GPIO_Pin_8

#define OLED_Port RCC_APB2Periph_GPIOC
#define OLED_GPIO GPIOC

#define set(bitx)	PCout(bitx)=1
#define clr(bitx) PCout(bitx)=0   //这几个口用同一个PORT的

void transfer_command_lcd(u8 data1);
void transfer_data_lcd(u8 data1);
void set_ads_lcd(u8 page,u8 column);

//***********************************
//显示英文字符串
//***********************************
void OLED_WriteString(u8 page, u8 column, u8 *s, u8 str_long, u8 size);
void OLED_WriteChar(u8 page,u8 column,u8 c, u8 size);
void OLED_WriteNumber(u8 page,u8 column, unsigned long number, u8 num_bit, u8 dot_bit, u8 front_no_zero, u8 size);
void OLED_Draw_Pic(u8 * p);
void OLED_Init(void);
void OLED_Clear(void);


//**********************************
//功能：写一个字符
//page为“页。相当于行，为1~8(64/8)
//column 为列，1~128
//c 为要显示的字符
//size 0为小字，1为大字
//**********************************
void OLED_WriteChar(u8 page,u8 column,u8 c,u8 size)
{
	u8 *dp = 0;
	u8 col_cnt;
	
	clr(OLED_cs);
	if(size == 0)
	{
			dp = (u8 *)font_s[c-' '];
			set_ads_lcd(page,column);
			for(col_cnt=0;col_cnt<6;col_cnt++)
		  {
				transfer_data_lcd(*dp);
				dp++;
      }
  }
	else if(size == 1)
	{
		transfer_data_lcd(*dp);
		dp++;
	}
	set(OLED_cs);

}
//*********************************
//写英文字符
//显示一个字符串,str_long 用于要限定长度的时候，适用于存在很多东西的矩阵，
//这时候没有结束标志，但又不至于专门去弄一个循环
//平时的时候只要保持 str_long 比自己的字符串长就行了
//*********************************
void OLED_WriteString(u8 page, u8 column, u8 *s, u8 str_long, u8 size)
{
	u8 col_cnt = 0;
    u8 * dp = 0;
    u8 str_long_bak = str_long, *s_bak = s;
    
    clr(OLED_cs);
    set_ads_lcd(page, column);    
    
    if(size == 0)
    {
        while (*s && (str_long-- > 0))
        {
            dp = (u8 *)font_s[*s-' '];
            for (col_cnt=0;col_cnt<6;col_cnt++)
            {	
                transfer_data_lcd(*dp);
                dp++;
            }    
            s++;
        }
    }
    else if(size == 1)
    {
        while (*s && (str_long-- > 0))
        {
            dp = (u8 *)font_l[*s-' '];
            for (col_cnt=0;col_cnt<8;col_cnt++)
            {	
                transfer_data_lcd(*dp);
                dp++;
            }    
            s++;
        }

        set_ads_lcd(page + 1, column);    
        s = s_bak;
        str_long = str_long_bak;
        while (*s && (str_long-- > 0))
        {
            dp = (u8 *)font_l[*s-' '] + 8;
            for (col_cnt=0;col_cnt<8;col_cnt++)
            {	
                transfer_data_lcd(*dp);
                dp++;
            }    
            s++;
        }
        
    }
    
    set(OLED_cs);
}
//**********************************************
//写几个汉子，需要提前取模，并在程序里头写出字数
//
//**********************************************
void OLED_WriteHz(u8 page, u8 column, u8 *s, u8 str_long)
{
    u8 col_cnt = 0;
    u8 str_long_bak = str_long, *s_bak = s;
    
    clr(OLED_cs);
    
    set_ads_lcd(page, column);    
    while (str_long-- > 0)
    {
        for (col_cnt=0;col_cnt<16;col_cnt++)
        {	
            transfer_data_lcd(*s);
            s++;
        }    
        s += 16;
    }

    set_ads_lcd(page + 1, column);   
    s = s_bak + 16;
    str_long = str_long_bak;
    while (str_long-- > 0)
    {
        for (col_cnt=0;col_cnt<16;col_cnt++)
        {	
            transfer_data_lcd(*s);
            s++;
        }    
        s += 16;
    }
    
    set(OLED_cs);
}

//********************************************
//写一个任意数字
//想要这样实现：给一个整数（单片机一般用不起浮点数）
//确定位长（为了格式要求，最大11位，包括小数点）num_bit
//确定起始位置（必须的）page/colum
//确定转换成小数的位数（便于把整数当成小数来显示）dot_bit
//确定前面空位是否把零弄成空格（适用于不同模式，而且用于拼接）front_no_zero
void OLED_WriteNumber(u8 page,u8 column, unsigned long number, u8 num_bit, u8 dot_bit, u8 front_no_zero, u8 size)
{
	 u8 num_str[12]={""}, i = 0, num_bit_bak = num_bit;
   
    if(num_bit > 11)
    {
        num_bit = 11;   
    }
		//先处理小数部分
		if(dot_bit > 0)
    {
        for(i = 0; i < dot_bit; i++)
        {
            num_str[--num_bit] = number%10 + '0';//每位以字符表示
            number /= 10;
        }
        num_str[--num_bit] = '.';
    }
		//整数部分
		while(num_bit > 0)
    {
        num_str[--num_bit] = number%10 + '0';//每位以字符表示
        number /= 10;
    }
		//填充空格
		if(front_no_zero == 1)
    {
        num_bit_bak -= (dot_bit + 2);
        for(i = 0; i < num_bit_bak; i++)
        {
            if(num_str[i] == '0')
            {
                num_str[i] = ' ';
            }
            else
            {
                break;   
            }
        }
    }
        
    OLED_WriteString(page,column,num_str,12,size);		
}

//**********************************************
//画一幅图
//**********************************************
void OLED_Draw_Pic(u8 * p)
{
    u8 i = 0, j = 0;

    clr(OLED_cs);

    for(i = 1; i < 9; i++)
    {
        set_ads_lcd(i, 1);    
        for(j = 1; j < 129; j++)
        {
            transfer_data_lcd(*p++);
        }
    }

    set(OLED_cs);
}

//******************************************
//LCD模块初始化
void OLED_Init()
{   
	  GPIO_InitTypeDef GPIO_InitStructure;
		delay_init();
    RCC_APB2PeriphClockCmd(OLED_Port,ENABLE);
    
		GPIO_InitStructure.GPIO_Pin=GPIO_rs|GPIO_reset|GPIO_cs|GPIO_sid|GPIO_sclk;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_10MHz;
	  GPIO_Init(OLED_GPIO,&GPIO_InitStructure);
    clr(OLED_cs);
    clr(OLED_reset);        //低电平复位
    delay_us(20);
    set(OLED_reset);		    //复位完毕
    delay_us(20);        
    transfer_command_lcd(0xe2);	 //软复位
    delay_us(5);
    transfer_command_lcd(0x2c);  //升压步骤1
    delay_us(5);	
    transfer_command_lcd(0x2e);  //升压步骤2
    delay_us(5);
    transfer_command_lcd(0x2f);  //升压步骤3
    delay_us(5);
    transfer_command_lcd(0x25);  //粗调对比度，可设置范围0x20~0x27
    transfer_command_lcd(0x81);  //微调对比度
    transfer_command_lcd(0x16);  //0x1a,微调杜比度值，可设置范围0x00~0x3f
    transfer_command_lcd(0xa2);  //1/9偏压比（bias）
    transfer_command_lcd(0xc8);  //行扫描顺序：从上到下
    transfer_command_lcd(0xa0);  //列扫描顺序：从左到右
    transfer_command_lcd(0x40);  //起始行：第一行开始
    transfer_command_lcd(0xaf);  //开显示vv
    set(OLED_cs);
    OLED_Clear();
}

//******************************************************
//全屏清屏
//******************************************************
void OLED_Clear()
{
    u8 i,j;
    clr(OLED_cs);
    for(i=0;i<8;i++)
    {
        transfer_command_lcd(0xb0+i);
        transfer_command_lcd(0x10);
        transfer_command_lcd(0x00);
        for(j=0;j<128;j++)
        {
            transfer_data_lcd(0x00);
        }
    }
    set(OLED_cs);
}

//***************************************************
//设置显示位置
//设置行和列，液晶本身不会自己跨行，但是我们的应用通常是不希望他跨行的，所以超过部分不显示
//***************************************************
void set_ads_lcd(u8 page, u8 column)
{
    u8 page_address;
    u8 column_address_L,column_address_H;
    
    page_address = 0xb0 + page - 1;
    column = column - 1;	
    
    column_address_L = column & 0x0f;
    column_address_H = ((column >> 4) & 0x0f) + 0x10;
    
    transfer_command_lcd(page_address); 		/*Set Page Address*/
    transfer_command_lcd(column_address_H);	/*Set MSB of column Address*/
    transfer_command_lcd(column_address_L);	/*Set LSB of column Address*/	
}
//*************************************************
//写指令到LCD模块
//*************************************************
void transfer_command_lcd(u8 data1)   
{
    u8 i;
    clr(OLED_rs);
    for(i=0;i<8;i++)
    {
        clr(OLED_sclk);
        if(data1&0x80) set(OLED_sid);
        else clr(OLED_sid);
        set(OLED_sclk);
        data1<<=1;
    }
}

//*************************************************
//写数据到LCD模块
//*************************************************
void transfer_data_lcd(u8 data1)
{
    u8 i;
    set(OLED_rs);
    for(i=0;i<8;i++)
    {
        clr(OLED_sclk);
        if(data1&0x80) set(OLED_sid);
        else clr(OLED_sid);
        set(OLED_sclk);
        data1<<=1;
    }
}
//u8 spi_RW(u8 data1)
//{
//	u8 i,tmp=0;
//	set(OLED_rs);
//	for(i=0;i<8;i++)
//	{
//		clr(OLED_sclk);
//		if(data1&0x80) set(OLED_sid);
//		else clr(OLED_sid);
//		set(OLED_sclk);
//		tmp|=MISO;
//		if(i<7)
//		tmp=tmp<<1;
//		data1<<=1;
//	}
//	return tmp;
//
//}
//**********************************************
//小字体部分
//**********************************************
const u8 font_s[][6] =
{
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // sp
	{ 0x00, 0x00, 0x00, 0x2f, 0x00, 0x00 },   // !
	{ 0x00, 0x00, 0x07, 0x00, 0x07, 0x00 },   // "
	{ 0x00, 0x14, 0x7f, 0x14, 0x7f, 0x14 },   // #
	{ 0x00, 0x24, 0x2a, 0x7f, 0x2a, 0x12 },   // $
	{ 0x00, 0x62, 0x64, 0x08, 0x13, 0x23 },   // %
	{ 0x00, 0x36, 0x49, 0x55, 0x22, 0x50 },   // &
	{ 0x00, 0x00, 0x05, 0x03, 0x00, 0x00 },   // '
	{ 0x00, 0x00, 0x1c, 0x22, 0x41, 0x00 },   // (
	{ 0x00, 0x00, 0x41, 0x22, 0x1c, 0x00 },   // )
	{ 0x00, 0x14, 0x08, 0x3E, 0x08, 0x14 },   // *
	{ 0x00, 0x08, 0x08, 0x3E, 0x08, 0x08 },   // +
	{ 0x00, 0x00, 0x00, 0xA0, 0x60, 0x00 },   // ,
	{ 0x00, 0x08, 0x08, 0x08, 0x08, 0x08 },   // -
	{ 0x00, 0x00, 0x60, 0x60, 0x00, 0x00 },   // .
	{ 0x00, 0x20, 0x10, 0x08, 0x04, 0x02 },   // /
	{ 0x00, 0x3E, 0x51, 0x49, 0x45, 0x3E },   // 0
	{ 0x00, 0x00, 0x42, 0x7F, 0x40, 0x00 },   // 1
	{ 0x00, 0x42, 0x61, 0x51, 0x49, 0x46 },   // 2
	{ 0x00, 0x21, 0x41, 0x45, 0x4B, 0x31 },   // 3
	{ 0x00, 0x18, 0x14, 0x12, 0x7F, 0x10 },   // 4
	{ 0x00, 0x27, 0x45, 0x45, 0x45, 0x39 },   // 5
	{ 0x00, 0x3C, 0x4A, 0x49, 0x49, 0x30 },   // 6
	{ 0x00, 0x01, 0x71, 0x09, 0x05, 0x03 },   // 7
	{ 0x00, 0x36, 0x49, 0x49, 0x49, 0x36 },   // 8
	{ 0x00, 0x06, 0x49, 0x49, 0x29, 0x1E },   // 9
	{ 0x00, 0x00, 0x36, 0x36, 0x00, 0x00 },   // :
	{ 0x00, 0x00, 0x56, 0x36, 0x00, 0x00 },   // ;
	{ 0x00, 0x08, 0x14, 0x22, 0x41, 0x00 },   // <
	{ 0x00, 0x14, 0x14, 0x14, 0x14, 0x14 },   // =
	{ 0x00, 0x00, 0x41, 0x22, 0x14, 0x08 },   // >
	{ 0x00, 0x02, 0x01, 0x51, 0x09, 0x06 },   // ?
	{ 0x00, 0x32, 0x49, 0x59, 0x51, 0x3E },   // @
	{ 0x00, 0x7C, 0x12, 0x11, 0x12, 0x7C },   // A
	{ 0x00, 0x7F, 0x49, 0x49, 0x49, 0x36 },   // B
	{ 0x00, 0x3E, 0x41, 0x41, 0x41, 0x22 },   // C
	{ 0x00, 0x7F, 0x41, 0x41, 0x22, 0x1C },   // D
	{ 0x00, 0x7F, 0x49, 0x49, 0x49, 0x41 },   // E
	{ 0x00, 0x7F, 0x09, 0x09, 0x09, 0x01 },   // F
	{ 0x00, 0x3E, 0x41, 0x49, 0x49, 0x7A },   // G
	{ 0x00, 0x7F, 0x08, 0x08, 0x08, 0x7F },   // H
	{ 0x00, 0x00, 0x41, 0x7F, 0x41, 0x00 },   // I
	{ 0x00, 0x20, 0x40, 0x41, 0x3F, 0x01 },   // J
	{ 0x00, 0x7F, 0x08, 0x14, 0x22, 0x41 },   // K
	{ 0x00, 0x7F, 0x40, 0x40, 0x40, 0x40 },   // L
	{ 0x00, 0x7F, 0x02, 0x0C, 0x02, 0x7F },   // M
	{ 0x00, 0x7F, 0x04, 0x08, 0x10, 0x7F },   // N
	{ 0x00, 0x3E, 0x41, 0x41, 0x41, 0x3E },   // O
	{ 0x00, 0x7F, 0x09, 0x09, 0x09, 0x06 },   // P
	{ 0x00, 0x3E, 0x41, 0x51, 0x21, 0x5E },   // Q
	{ 0x00, 0x7F, 0x09, 0x19, 0x29, 0x46 },   // R
	{ 0x00, 0x46, 0x49, 0x49, 0x49, 0x31 },   // S
	{ 0x00, 0x01, 0x01, 0x7F, 0x01, 0x01 },   // T
	{ 0x00, 0x3F, 0x40, 0x40, 0x40, 0x3F },   // U
	{ 0x00, 0x1F, 0x20, 0x40, 0x20, 0x1F },   // V
	{ 0x00, 0x3F, 0x40, 0x38, 0x40, 0x3F },   // W
	{ 0x00, 0x63, 0x14, 0x08, 0x14, 0x63 },   // X
	{ 0x00, 0x07, 0x08, 0x70, 0x08, 0x07 },   // Y
	{ 0x00, 0x61, 0x51, 0x49, 0x45, 0x43 },   // Z
	{ 0x00, 0x00, 0x7F, 0x41, 0x41, 0x00 },   // [
	{ 0x00, 0x55, 0x2A, 0x55, 0x2A, 0x55 },   // 55
	{ 0x00, 0x00, 0x41, 0x41, 0x7F, 0x00 },   // ]
	{ 0x00, 0x04, 0x02, 0x01, 0x02, 0x04 },   // ^
	{ 0x00, 0x40, 0x40, 0x40, 0x40, 0x40 },   // _
	{ 0x00, 0x00, 0x01, 0x02, 0x04, 0x00 },   // '
	{ 0x00, 0x20, 0x54, 0x54, 0x54, 0x78 },   // a
	{ 0x00, 0x7F, 0x48, 0x44, 0x44, 0x38 },   // b
	{ 0x00, 0x38, 0x44, 0x44, 0x44, 0x20 },   // c
	{ 0x00, 0x38, 0x44, 0x44, 0x48, 0x7F },   // d
	{ 0x00, 0x38, 0x54, 0x54, 0x54, 0x18 },   // e
	{ 0x00, 0x08, 0x7E, 0x09, 0x01, 0x02 },   // f
	{ 0x00, 0x18, 0xA4, 0xA4, 0xA4, 0x7C },   // g
	{ 0x00, 0x7F, 0x08, 0x04, 0x04, 0x78 },   // h
	{ 0x00, 0x00, 0x44, 0x7D, 0x40, 0x00 },   // i
	{ 0x00, 0x40, 0x80, 0x84, 0x7D, 0x00 },   // j
	{ 0x00, 0x7F, 0x10, 0x28, 0x44, 0x00 },   // k
	{ 0x00, 0x00, 0x41, 0x7F, 0x40, 0x00 },   // l
	{ 0x00, 0x7C, 0x04, 0x18, 0x04, 0x78 },   // m
	{ 0x00, 0x7C, 0x08, 0x04, 0x04, 0x78 },   // n
	{ 0x00, 0x38, 0x44, 0x44, 0x44, 0x38 },   // o
	{ 0x00, 0xFC, 0x24, 0x24, 0x24, 0x18 },   // p
	{ 0x00, 0x18, 0x24, 0x24, 0x18, 0xFC },   // q
	{ 0x00, 0x7C, 0x08, 0x04, 0x04, 0x08 },   // r
	{ 0x00, 0x48, 0x54, 0x54, 0x54, 0x20 },   // s
	{ 0x00, 0x04, 0x3F, 0x44, 0x40, 0x20 },   // t
	{ 0x00, 0x3C, 0x40, 0x40, 0x20, 0x7C },   // u
	{ 0x00, 0x1C, 0x20, 0x40, 0x20, 0x1C },   // v
	{ 0x00, 0x3C, 0x40, 0x30, 0x40, 0x3C },   // w
	{ 0x00, 0x44, 0x28, 0x10, 0x28, 0x44 },   // x
	{ 0x00, 0x1C, 0xA0, 0xA0, 0xA0, 0x7C },   // y
	{ 0x00, 0x44, 0x64, 0x54, 0x4C, 0x44 },   // z
	{ 0x14, 0x14, 0x14, 0x14, 0x14, 0x14 }    // horiz lines
};

//*************************************************
//大字体部分
//*************************************************
const u8 font_l[][16] =
{
	/*--  ??:     --*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*--  ??:  !  --*/
	{0x00,0x00,0x00,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x33,0x30,0x00,0x00,0x00},
	/*--  ??:  "  --*/
	{0x00,0x10,0x0C,0x06,0x10,0x0C,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*--  ??:  #  --*/
	{0x40,0xC0,0x78,0x40,0xC0,0x78,0x40,0x00,0x04,0x3F,0x04,0x04,0x3F,0x04,0x04,0x00},
	/*--  ??:  $  --*/
	{0x00,0x70,0x88,0xFC,0x08,0x30,0x00,0x00,0x00,0x18,0x20,0xFF,0x21,0x1E,0x00,0x00},
	/*--  ??:  %  --*/
	{0xF0,0x08,0xF0,0x00,0xE0,0x18,0x00,0x00,0x00,0x21,0x1C,0x03,0x1E,0x21,0x1E,0x00},
	/*--  ??:  &  --*/
	{0x00,0xF0,0x08,0x88,0x70,0x00,0x00,0x00,0x1E,0x21,0x23,0x24,0x19,0x27,0x21,0x10},
	/*--  ??:  '  --*/
	{0x10,0x16,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*--  ??:  (  --*/
	{0x00,0x00,0x00,0xE0,0x18,0x04,0x02,0x00,0x00,0x00,0x00,0x07,0x18,0x20,0x40,0x00},
	/*--  ??:  )  --*/
	{0x00,0x02,0x04,0x18,0xE0,0x00,0x00,0x00,0x00,0x40,0x20,0x18,0x07,0x00,0x00,0x00},
	/*--  ??:  *  --*/
	{0x40,0x40,0x80,0xF0,0x80,0x40,0x40,0x00,0x02,0x02,0x01,0x0F,0x01,0x02,0x02,0x00},
	/*--  ??:  +  --*/
	{0x00,0x00,0x00,0xF0,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x1F,0x01,0x01,0x01,0x00},
	/*--  ??:  ,  --*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xB0,0x70,0x00,0x00,0x00,0x00,0x00},
	/*--  ??:  -  --*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01},
	/*--  ??:  .  --*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x00,0x00,0x00},
	/*--  ??:  /  --*/
	{0x00,0x00,0x00,0x00,0x80,0x60,0x18,0x04,0x00,0x60,0x18,0x06,0x01,0x00,0x00,0x00},
	/*--  ??:  0  --*/
	{0x00,0xE0,0x10,0x08,0x08,0x10,0xE0,0x00,0x00,0x0F,0x10,0x20,0x20,0x10,0x0F,0x00},
	/*--  ??:  1  --*/
	{0x00,0x10,0x10,0xF8,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00},
	/*--  ??:  2  --*/
	{0x00,0x70,0x08,0x08,0x08,0x88,0x70,0x00,0x00,0x30,0x28,0x24,0x22,0x21,0x30,0x00},
	/*--  ??:  3  --*/
	{0x00,0x30,0x08,0x88,0x88,0x48,0x30,0x00,0x00,0x18,0x20,0x20,0x20,0x11,0x0E,0x00},
	/*--  ??:  4  --*/
	{0x00,0x00,0xC0,0x20,0x10,0xF8,0x00,0x00,0x00,0x07,0x04,0x24,0x24,0x3F,0x24,0x00},
	/*--  ??:  5  --*/
	{0x00,0xF8,0x08,0x88,0x88,0x08,0x08,0x00,0x00,0x19,0x21,0x20,0x20,0x11,0x0E,0x00},
	/*--  ??:  6  --*/
	{0x00,0xE0,0x10,0x88,0x88,0x18,0x00,0x00,0x00,0x0F,0x11,0x20,0x20,0x11,0x0E,0x00},
	/*--  ??:  7  --*/
	{0x00,0x38,0x08,0x08,0xC8,0x38,0x08,0x00,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00},
	/*--  ??:  8  --*/
	{0x00,0x70,0x88,0x08,0x08,0x88,0x70,0x00,0x00,0x1C,0x22,0x21,0x21,0x22,0x1C,0x00},
	/*--  ??:  9  --*/
	{0x00,0xE0,0x10,0x08,0x08,0x10,0xE0,0x00,0x00,0x00,0x31,0x22,0x22,0x11,0x0F,0x00},
	/*--  ??:  :  --*/
	{0x00,0x00,0x00,0xC0,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x00},
	/*--  ??:  ;  --*/
	{0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x60,0x00,0x00,0x00,0x00},
	/*--  ??:  <  --*/
	{0x00,0x00,0x80,0x40,0x20,0x10,0x08,0x00,0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x00},
	/*--  ??:  =  --*/
	{0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x00,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x00},
	/*--  ??:  >  --*/
	{0x00,0x08,0x10,0x20,0x40,0x80,0x00,0x00,0x00,0x20,0x10,0x08,0x04,0x02,0x01,0x00},
	/*--  ??:  ?  --*/
	{0x00,0x70,0x48,0x08,0x08,0x08,0xF0,0x00,0x00,0x00,0x00,0x30,0x36,0x01,0x00,0x00},
	/*--  ??:  @  --*/
	{0xC0,0x30,0xC8,0x28,0xE8,0x10,0xE0,0x00,0x07,0x18,0x27,0x24,0x23,0x14,0x0B,0x00},
	/*--  ??:  A  --*/
	{0x00,0x00,0xC0,0x38,0xE0,0x00,0x00,0x00,0x20,0x3C,0x23,0x02,0x02,0x27,0x38,0x20},
	/*--  ??:  B  --*/
	{0x08,0xF8,0x88,0x88,0x88,0x70,0x00,0x00,0x20,0x3F,0x20,0x20,0x20,0x11,0x0E,0x00},
	/*--  ??:  C  --*/
	{0xC0,0x30,0x08,0x08,0x08,0x08,0x38,0x00,0x07,0x18,0x20,0x20,0x20,0x10,0x08,0x00},
	/*--  ??:  D  --*/
	{0x08,0xF8,0x08,0x08,0x08,0x10,0xE0,0x00,0x20,0x3F,0x20,0x20,0x20,0x10,0x0F,0x00},
	/*--  ??:  E  --*/
	{0x08,0xF8,0x88,0x88,0xE8,0x08,0x10,0x00,0x20,0x3F,0x20,0x20,0x23,0x20,0x18,0x00},
	/*--  ??:  F  --*/
	{0x08,0xF8,0x88,0x88,0xE8,0x08,0x10,0x00,0x20,0x3F,0x20,0x00,0x03,0x00,0x00,0x00},
	/*--  ??:  G  --*/
	{0xC0,0x30,0x08,0x08,0x08,0x38,0x00,0x00,0x07,0x18,0x20,0x20,0x22,0x1E,0x02,0x00},
	/*--  ??:  H  --*/
	{0x08,0xF8,0x08,0x00,0x00,0x08,0xF8,0x08,0x20,0x3F,0x21,0x01,0x01,0x21,0x3F,0x20},
	/*--  ??:  I  --*/
	{0x00,0x08,0x08,0xF8,0x08,0x08,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00},
	/*--  ??:  J  --*/
	{0x00,0x00,0x08,0x08,0xF8,0x08,0x08,0x00,0xC0,0x80,0x80,0x80,0x7F,0x00,0x00,0x00},
	/*--  ??:  K  --*/
	{0x08,0xF8,0x88,0xC0,0x28,0x18,0x08,0x00,0x20,0x3F,0x20,0x01,0x26,0x38,0x20,0x00},
	/*--  ??:  L  --*/
	{0x08,0xF8,0x08,0x00,0x00,0x00,0x00,0x00,0x20,0x3F,0x20,0x20,0x20,0x20,0x30,0x00},
	/*--  ??:  M  --*/
	{0x08,0xF8,0xF8,0x00,0xF8,0xF8,0x08,0x00,0x20,0x3F,0x00,0x3F,0x00,0x3F,0x20,0x00},
	/*--  ??:  N  --*/
	{0x08,0xF8,0x30,0xC0,0x00,0x08,0xF8,0x08,0x20,0x3F,0x20,0x00,0x07,0x18,0x3F,0x00},
	/*--  ??:  O  --*/
	{0xE0,0x10,0x08,0x08,0x08,0x10,0xE0,0x00,0x0F,0x10,0x20,0x20,0x20,0x10,0x0F,0x00},
	/*--  ??:  P  --*/
	{0x08,0xF8,0x08,0x08,0x08,0x08,0xF0,0x00,0x20,0x3F,0x21,0x01,0x01,0x01,0x00,0x00},
	/*--  ??:  Q  --*/
	{0xE0,0x10,0x08,0x08,0x08,0x10,0xE0,0x00,0x0F,0x18,0x24,0x24,0x38,0x50,0x4F,0x00},
	/*--  ??:  R  --*/
	{0x08,0xF8,0x88,0x88,0x88,0x88,0x70,0x00,0x20,0x3F,0x20,0x00,0x03,0x0C,0x30,0x20},
	/*--  ??:  S  --*/
	{0x00,0x70,0x88,0x08,0x08,0x08,0x38,0x00,0x00,0x38,0x20,0x21,0x21,0x22,0x1C,0x00},
	/*--  ??:  T  --*/
	{0x18,0x08,0x08,0xF8,0x08,0x08,0x18,0x00,0x00,0x00,0x20,0x3F,0x20,0x00,0x00,0x00},
	/*--  ??:  U  --*/
	{0x08,0xF8,0x08,0x00,0x00,0x08,0xF8,0x08,0x00,0x1F,0x20,0x20,0x20,0x20,0x1F,0x00},
	/*--  ??:  V  --*/
	{0x08,0x78,0x88,0x00,0x00,0xC8,0x38,0x08,0x00,0x00,0x07,0x38,0x0E,0x01,0x00,0x00},
	/*--  ??:  W  --*/
	{0xF8,0x08,0x00,0xF8,0x00,0x08,0xF8,0x00,0x03,0x3C,0x07,0x00,0x07,0x3C,0x03,0x00},
	/*--  ??:  X  --*/
	{0x08,0x18,0x68,0x80,0x80,0x68,0x18,0x08,0x20,0x30,0x2C,0x03,0x03,0x2C,0x30,0x20},
	/*--  ??:  Y  --*/
	{0x08,0x38,0xC8,0x00,0xC8,0x38,0x08,0x00,0x00,0x00,0x20,0x3F,0x20,0x00,0x00,0x00},
	/*--  ??:  Z  --*/
	{0x10,0x08,0x08,0x08,0xC8,0x38,0x08,0x00,0x20,0x38,0x26,0x21,0x20,0x20,0x18,0x00},
	/*--  ??:  [  --*/
	{0x00,0x00,0x00,0xFE,0x02,0x02,0x02,0x00,0x00,0x00,0x00,0x7F,0x40,0x40,0x40,0x00},
	/*--  ??:  \  --*/
	{0x00,0x0C,0x30,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x06,0x38,0xC0,0x00},
	/*--  ??:  ]  --*/
	{0x00,0x02,0x02,0x02,0xFE,0x00,0x00,0x00,0x00,0x40,0x40,0x40,0x7F,0x00,0x00,0x00},
	/*--  ??:  ^  --*/
	{0x00,0x00,0x04,0x02,0x02,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*--  ??:  _  --*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80},
	/*--  ??:  `  --*/
	{0x00,0x02,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*--  ??:  a  --*/
	{0x00,0x00,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x19,0x24,0x22,0x22,0x22,0x3F,0x20},
	/*--  ??:  b  --*/
	{0x08,0xF8,0x00,0x80,0x80,0x00,0x00,0x00,0x00,0x3F,0x11,0x20,0x20,0x11,0x0E,0x00},
	/*--  ??:  c  --*/
	{0x00,0x00,0x00,0x80,0x80,0x80,0x00,0x00,0x00,0x0E,0x11,0x20,0x20,0x20,0x11,0x00},
	/*--  ??:  d  --*/
	{0x00,0x00,0x00,0x80,0x80,0x88,0xF8,0x00,0x00,0x0E,0x11,0x20,0x20,0x10,0x3F,0x20},
	/*--  ??:  e  --*/
	{0x00,0x00,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x1F,0x22,0x22,0x22,0x22,0x13,0x00},
	/*--  ??:  f  --*/
	{0x00,0x80,0x80,0xF0,0x88,0x88,0x88,0x18,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00},
	/*--  ??:  g  --*/
	{0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x6B,0x94,0x94,0x94,0x93,0x60,0x00},
	/*--  ??:  h  --*/
	{0x08,0xF8,0x00,0x80,0x80,0x80,0x00,0x00,0x20,0x3F,0x21,0x00,0x00,0x20,0x3F,0x20},
	/*--  ??:  i  --*/
	{0x00,0x80,0x98,0x98,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00},
	/*--  ??:  j  --*/
	{0x00,0x00,0x00,0x80,0x98,0x98,0x00,0x00,0x00,0xC0,0x80,0x80,0x80,0x7F,0x00,0x00},
	/*--  ??:  k  --*/
	{0x08,0xF8,0x00,0x00,0x80,0x80,0x80,0x00,0x20,0x3F,0x24,0x02,0x2D,0x30,0x20,0x00},
	/*--  ??:  l  --*/
	{0x00,0x08,0x08,0xF8,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00},
	/*--  ??:  m  --*/
	{0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x20,0x3F,0x20,0x00,0x3F,0x20,0x00,0x3F},
	/*--  ??:  n  --*/
	{0x80,0x80,0x00,0x80,0x80,0x80,0x00,0x00,0x20,0x3F,0x21,0x00,0x00,0x20,0x3F,0x20},
	/*--  ??:  o  --*/
	{0x00,0x00,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x1F,0x20,0x20,0x20,0x20,0x1F,0x00},
	/*--  ??:  p  --*/
	{0x80,0x80,0x00,0x80,0x80,0x00,0x00,0x00,0x80,0xFF,0xA1,0x20,0x20,0x11,0x0E,0x00},
	/*--  ??:  q  --*/
	{0x00,0x00,0x00,0x80,0x80,0x80,0x80,0x00,0x00,0x0E,0x11,0x20,0x20,0xA0,0xFF,0x80},
	/*--  ??:  r  --*/
	{0x80,0x80,0x80,0x00,0x80,0x80,0x80,0x00,0x20,0x20,0x3F,0x21,0x20,0x00,0x01,0x00},
	/*--  ??:  s  --*/
	{0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x33,0x24,0x24,0x24,0x24,0x19,0x00},
	/*--  ??:  t  --*/
	{0x00,0x80,0x80,0xE0,0x80,0x80,0x00,0x00,0x00,0x00,0x00,0x1F,0x20,0x20,0x00,0x00},
	/*--  ??:  u  --*/
	{0x80,0x80,0x00,0x00,0x00,0x80,0x80,0x00,0x00,0x1F,0x20,0x20,0x20,0x10,0x3F,0x20},
	/*--  ??:  v  --*/
	{0x80,0x80,0x80,0x00,0x00,0x80,0x80,0x80,0x00,0x01,0x0E,0x30,0x08,0x06,0x01,0x00},
	/*--  ??:  w  --*/
	{0x80,0x80,0x00,0x80,0x00,0x80,0x80,0x80,0x0F,0x30,0x0C,0x03,0x0C,0x30,0x0F,0x00},
	/*--  ??:  x  --*/
	{0x00,0x80,0x80,0x00,0x80,0x80,0x80,0x00,0x00,0x20,0x31,0x2E,0x0E,0x31,0x20,0x00},
	/*--  ??:  y  --*/
	{0x80,0x80,0x80,0x00,0x00,0x80,0x80,0x80,0x80,0x81,0x8E,0x70,0x18,0x06,0x01,0x00},
	/*--  ??:  z  --*/
	{0x00,0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x21,0x30,0x2C,0x22,0x21,0x30,0x00},
	/*--  ??:  {  --*/
	{0x00,0x00,0x00,0x00,0x80,0x7C,0x02,0x02,0x00,0x00,0x00,0x00,0x00,0x3F,0x40,0x40},
	/*--  ??:  |  --*/
	{0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00},
	/*--  ??:  }  --*/
	{0x00,0x02,0x02,0x7C,0x80,0x00,0x00,0x00,0x00,0x40,0x40,0x3F,0x00,0x00,0x00,0x00},
	/*--  ??:  ~  --*/
	{0x00,0x06,0x01,0x01,0x02,0x02,0x04,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
};

//*********************************************************
//汉字
//填16*16 字模，要多少字填多少字，显示程序那边会填多少字的
//纵向取模，字续倒置
//*********************************************************
const u8 font_hz_tongxgc[][16] = {
/*--  文字：通  --*/
/*--	宋体12；此字体下对应的点阵为：宽*高=16*16	--*/
0x40,0x42,0xCC,0x00,0x00,0xE2,0x22,0x2A,0x2A,0xF2,0x2A,0x26,0x22,0xE0,0x00,0x00,
0x80,0x40,0x3F,0x40,0x80,0xFF,0x89,0x89,0x89,0xBF,0x89,0xA9,0xC9,0xBF,0x80,0x00,

/*--  文字：信  --*/
/*--	宋体12；此字体下对应的点阵为：宽*高=16*16	--*/
0x00,0x80,0x60,0xF8,0x07,0x00,0x04,0x24,0x24,0x25,0x26,0x24,0x24,0x24,0x04,0x00,
0x01,0x00,0x00,0xFF,0x00,0x00,0x00,0xF9,0x49,0x49,0x49,0x49,0x49,0xF9,0x00,0x00,

/*--  文字：工  --*/
/*--	宋体12；此字体下对应的点阵为：宽*高=16*16	--*/
0x00,0x04,0x04,0x04,0x04,0x04,0x04,0xFC,0x04,0x04,0x04,0x04,0x04,0x04,0x00,0x00,
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x3F,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x00,

/*--  文字：程  --*/
/*--	宋体12；此字体下对应的点阵为：宽*高=16*16	--*/
0x24,0x24,0xA4,0xFE,0x23,0x22,0x00,0x3E,0x22,0x22,0x22,0x22,0x22,0x3E,0x00,0x00,
0x08,0x06,0x01,0xFF,0x01,0x06,0x40,0x49,0x49,0x49,0x7F,0x49,0x49,0x49,0x41,0x00,   
};

//************************************************
//128*64 兔斯基图片
//该图片作为例子
//************************************************
const u8 tsktsk[] = {
0x00,0x00,0x00,0x00,0x00,0x00,0x70,0xD0,0x10,0x30,0x60,0xC0,0x80,0x80,0x00,0x00,
0x00,0x00,0xE0,0x20,0x20,0x60,0xC0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xC0,
0x40,0x60,0x60,0x20,0x20,0xA0,0xE0,0x00,0x00,0x00,0xC0,0x60,0x30,0x10,0x08,0x0C,
0x04,0x04,0x84,0xEC,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x07,0x1C,0x30,0xE0,0xC0,0x01,0x07,0x1C,
0x30,0x10,0x11,0x19,0x0E,0x08,0x08,0x09,0x0F,0x0C,0x08,0x08,0x08,0x08,0x18,0x10,
0x10,0x30,0x20,0x20,0x60,0x40,0xC0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,
0xC0,0x60,0x30,0x10,0x18,0x08,0x08,0x0C,0x04,0x04,0x04,0x04,0x06,0x07,0x05,0x04,
0x04,0x04,0x04,0x04,0x0E,0x07,0x04,0x04,0x06,0x0F,0x08,0x08,0x18,0x10,0x10,0x38,
0x6C,0xC7,0x81,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x70,0x1C,0x06,0x03,0x01,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x00,0x00,0x00,0x00,0x00,0x08,0x09,0x09,0x0F,0x0C,0x38,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xF0,0x1C,0x07,0x01,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x80,0x40,0x40,0x60,0x20,0x20,
0x30,0x10,0x10,0x10,0x10,0x18,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x03,0x06,0x3C,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0xFF,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xFF,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x80,0xC0,0x60,0x38,
0x0C,0x04,0x06,0x02,0x02,0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0xC0,0x78,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x01,0x07,0x0C,0x38,0x60,0x80,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x80,0x80,0xC0,0xC0,0xC0,0xE0,0xA0,0x90,0x18,0x0C,0x06,0x03,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x18,0x20,0xE0,
0xC0,0x40,0x60,0x60,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x80,0x80,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x10,0x30,0x20,0x60,0x40,0xD0,0x90,0x18,0x0C,
0x04,0x06,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xE0,0x38,0x09,0x0F,0x06,0x06,0x00,0x00,
0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0xC4,0x74,0x1E,0x02,0x02,0x02,0x03,0x01,
0x01,0xFF,0x6D,0x20,0x20,0x20,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x03,0x02,0x22,
0x32,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x92,0x92,0xB3,0xB5,0xFF,0x09,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,0x06,0x01,
0x00,0x00,0x20,0x7E,0x43,0xC2,0xC6,0xE4,0x3C,0x0C,0x07,0x07,0x82,0xC0,0xC0,0x80,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x03,0x06,
0x1C,0x70,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0xF8,0x0F,0x00,0x00,0xC0,0x00,0x00,0x40,0xC0,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x78,0x8F,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
0x80,0x8D,0xDF,0xF6,0xB2,0x82,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0xB0,0xB4,0xF4,0xFF,0xCB,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x03,0x0E,
0x08,0x18,0x10,0xD0,0x78,0x0F,0x01,0x01,0x00,0xF8,0x0E,0x0F,0x0D,0x04,0x06,0x07,
0x1E,0xF0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x01,0x07,0x7C,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x0C,0x07,0x00,0x00,0x00,0x1F,0x00,0x00,0x00,0x03,0x1E,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,
0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,
0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x07,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x01,0x03,0x00,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

#ifdef USING_UCGUI
void  GUI_DispChar  (unsigned short c); 
void OLED_printf (char *fmt, ...)
{
	char buffer[OLED_CMD_BUFFER_LEN+1]={0};  
	u8 i = 0;
	va_list arg_ptr;
	va_start(arg_ptr, fmt);  
	vsnprintf(buffer, OLED_CMD_BUFFER_LEN+1, fmt, arg_ptr);
	while ((i < OLED_CMD_BUFFER_LEN) && buffer[i])
	{
		GUI_DispChar(buffer[i++]);
	}
	va_end(arg_ptr);
}
void OLED_printfAt (int x,int y,char *fmt, ...)
{
	char buffer[OLED_CMD_BUFFER_LEN+1]={0};    
	u8 i = 0;
	va_list arg_ptr;
	va_start(arg_ptr, fmt);  
	vsnprintf(buffer, OLED_CMD_BUFFER_LEN+1, fmt, arg_ptr);
	GUI_GotoXY(x,y);
	while ((i < OLED_CMD_BUFFER_LEN) && buffer[i])
	{
		OLED_WriteChar(x,y,buffer[i++],0);
		y=y+6;
	}
	va_end(arg_ptr);
}
/*

//OLED_BUF用于UCGUI画/读点

 U32	OLED_BUF[64][4]

//坐标系统
(0,0)---------------->	X127
	|U32	U32	U32	U32
	|U32	U32	U32	U32
	|U32	U32	U32	U32
	|.		.		.		U32
	|.		.		.		U32
	|.		.		.		U32
	|.		.		.		U32
	|U32	U32	U32	U32

	Y63
//12864单列（1字节）排布规律
				 _____
				|__0__|
				|__1__|
				|__2__|
				|__3__|
				|__4__|
				|__5__|
				|__6__|
				|__7__|

			data=76543210b
*/

//UCGUI底层驱动
#define OLED_XY_U32(x,y) OLED_BUF[y][x/32]
#define OLED_BITMSAK(n)  (0x80000000>>n)
unsigned int OLED_GetPixelIndex(int x, int y)
{
	return (OLED_XY_U32(x,y)& OLED_BITMSAK(x%32))?1:0;
}
void OLED_DrawPoint(int x, int y, int PixelIndex)
{
	unsigned int oldIndex_Start,Index=0;
	int i;
 	oldIndex_Start=y/8*8;
	if(PixelIndex)
		OLED_XY_U32(x,y)|=OLED_BITMSAK(x%32);
	else
		OLED_XY_U32(x,y)&=~OLED_BITMSAK(x%32);
	for(i=7;i>=0;i--)
	{
		Index|=OLED_GetPixelIndex(x,oldIndex_Start+i);
		if(i==0)
			break;
		Index<<=1;
	}
	clr(OLED_cs);
	set_ads_lcd(y/8+1,x+1);
	transfer_data_lcd(Index);
	set(OLED_cs);	
}
#else
//不使用UCGUI时的OLED_printf
void OLED_printfAt(int x,int y,char *fmt, ...)//输出到第x行[1-8]，第y列[1-128](小字）
{
	char buffer[OLED_CMD_BUFFER_LEN+1]={0};    
	u8 i = 0;
	va_list arg_ptr;
	va_start(arg_ptr, fmt);  
	vsnprintf(buffer, OLED_CMD_BUFFER_LEN+1, fmt, arg_ptr);
	while ((i < OLED_CMD_BUFFER_LEN) && buffer[i])
	{
		OLED_WriteChar(x,y,buffer[i++],0);
		y=y+6;
	}
	va_end(arg_ptr);
}
#endif















