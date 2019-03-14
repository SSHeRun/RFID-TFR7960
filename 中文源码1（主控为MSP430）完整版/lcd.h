/**********************************************************************************************************************************************************
* 文 件 名：LCD.H
* 功    能：HO12864FPD-14CSBE（3S）位图形液晶模块（ST7565P）驱动头文件。
* 硬件连接：HO12864FPD-14CSBE（3S）位图形液晶模块（ST7565P）与MSP430F2370的硬件连接关系如下所示：
*           HO12864FPD-14CSBE(3S)               MSP430F2370
*
*               SCL(PIN7)                         P1.6
*               SID(PIN8)                         P1.7
*               A0(PIN14)                         P3.6
*               CSn(PIN12)                        P2.4
*               RESETn(PIN13)                     P2.5
*               VDD(PIN9)
*               VSS(PIN10)
*               LED+(PIN11)
* 作    者：EMDOOR
* 日    期：2011年04月13号
**********************************************************************************************************************************************************/
#ifndef LCD_H
#define LCD_H


#include <MSP430x23x0.h>     	
#include <stdlib.h>		
#include <stdio.h>
#include "hardware.h"
#include "communicate.h"
#include "anticollision.h"
#include "globals.h"
#include "tiris.h"
#include "ISO14443.h"
#include "host.h"

/* 声明点阵字模数据定义为外部变量 */
/*====================================================================================================*/
extern unsigned char const logo[1024];
extern unsigned char const emdoor[1024];
extern unsigned char const usb[1024];

extern unsigned char const Num[256];
extern unsigned char const CG[16];
extern unsigned char const CH[16];
extern unsigned char const CI[16];
extern unsigned char const CJ[16];
extern unsigned char const CK[16];
extern unsigned char const CL[16];
extern unsigned char const CM[16];
extern unsigned char const CN[16];
extern unsigned char const CO[16];
extern unsigned char const CP[16];
extern unsigned char const CQ[16];
extern unsigned char const CR[16];
extern unsigned char const CS[16];
extern unsigned char const CT[16];
extern unsigned char const CU[16];
extern unsigned char const CV[16];
extern unsigned char const CW[16];
extern unsigned char const CX[16];
extern unsigned char const CY[16];
extern unsigned char const CZ[16];
extern unsigned char const Cz[16];

extern unsigned char const pro[16];
extern unsigned char const ZK[16];
extern unsigned char const YK[16];
extern unsigned char const ant[16];
extern unsigned char const ant1[16];
extern unsigned char const ant2[16];
extern unsigned char const ant3[16];
extern unsigned char const lkuo[16];
extern unsigned char const rkuo[16];

extern unsigned char const huan[32];
extern unsigned char const ying[32];
extern unsigned char const shi[32];
extern unsigned char const yong[32];
extern unsigned char const jiao[32];
extern unsigned char const xue[32];
extern unsigned char const ping[32];
extern unsigned char const tai2[32];
extern unsigned char const xi[32];
extern unsigned char const tong[32];

extern unsigned char const xie[32];
extern unsigned char const du[32];
extern unsigned char const yi[32];
extern unsigned char const ge[32];
extern unsigned char const kuai[32];
extern unsigned char const xin[32];
extern unsigned char const xi2[32];
extern unsigned char const an[32];
extern unsigned char const quan[32];
extern unsigned char const zhuang[32];
extern unsigned char const tai[32];

extern unsigned char const yi[32];
extern unsigned char const dao[32];
extern unsigned char const dian[32];
extern unsigned char const zi2[32];
extern unsigned char const lian[32];
extern unsigned char const ji[32];
extern unsigned char const mo[32];
extern unsigned char const shi2[32];
extern unsigned char const tong2[32];

/******************************************************************************************************************
* 函数名称：Delay_lcm()
* 功    能：软件延时
* 入口参数：count    延时参数，值越大，延时越长
* 出口参数：无
* 说    明：延时等待用。
******************************************************************************************************************/
void Delay_lcm(unsigned int count);

/******************************************************************************************************************
* 函数名称：Data_Send()
* 功    能：LCD串行模式发送数据
* 入口参数：data    要发送的数据
* 出口参数：无
* 说    明：MSP430微控制器向LCD缓冲区发送数据
******************************************************************************************************************/
void Data_Send(unsigned char data);

/******************************************************************************************************************
* 函数名称：Write_CMD()
* 功    能：LCD串行模式发送命令
* 入口参数：command    命令值
* 出口参数：无
* 说    明：MSP430微控制器向LCD控制器发送命令
******************************************************************************************************************/
void Write_CMD(unsigned char command);

/******************************************************************************************************************
* 函数名称：Write_Data()
* 功    能：写数据
* 入口参数：data    数据
* 出口参数：无
* 说    明：MSP430微控制器向LCD控制器发送数据
******************************************************************************************************************/
void Write_Data(unsigned char data);

/******************************************************************************************************************
* 函数名称：Display_Clear()
* 功    能：清空LCD显示
* 入口参数：无
* 出口参数：无
* 说    明：执行该函数，可以清除屏幕任何显示内容
******************************************************************************************************************/
void Display_Clear(void);

/******************************************************************************************************************
* 函数名称：Display_Picture()
* 功    能：显示一张128*64整屏图片
* 入口参数：*p       图片数据的首地址指针
* 出口参数：无
* 说    明：执行该函数，首先需指定欲显示的图片首地址。
******************************************************************************************************************/
void Display_Picture(unsigned char *p);

/******************************************************************************************************************
* 函数名称：Display_Chinese()
* 功    能：在指定的位置上，显示一个汉字
* 入口参数：pag       显示汉字的页地址
*           col       显示汉字的列地址
*           *hzk      显示汉字的地址指针
* 出口参数：无
* 说    明：若该函数操作不成功，是由于输入错误坐标造成
******************************************************************************************************************/
void Display_Chinese(unsigned char pag,unsigned char col, unsigned char const *hzk);

/******************************************************************************************************************
* 函数名称：Display_Char()
* 功    能：在指定的位置上，显示一个数字或者字母
* 入口参数：pag       显示汉字的页地址
*           col       显示汉字的列地址
*           *hzk      显示汉字的地址指针
* 出口参数：无
* 说    明：若该函数操作不成功，是由于输入错误坐标造成
******************************************************************************************************************/
void Display_Char(unsigned char pag,unsigned char col, unsigned char const *hzk);

/******************************************************************************************************************
* 函数名称：LCM12864_Init()
* 功    能：LCM12864模块初始化
* 入口参数：无
* 出口参数：无
* 说    明：初始化LCD显示器
******************************************************************************************************************/
void LCM12864_Init(void);

/******************************************************************************************************************
* 函数名称：Display_Put_Tag()
* 功    能：LCM12864模块显示请放入卡片信息
* 入口参数：无
* 出口参数：无
* 说    明：显示提示信息
******************************************************************************************************************/
void Display_Put_Tag(void);

/******************************************************************************************************************
* 函数名称：Display_Start()
* 功    能：LCM12864模块显示系统启动信息
* 入口参数：无
* 出口参数：无
* 说    明：显示启动信息
******************************************************************************************************************/
void Display_Start(void);

/******************************************************************************************************************
* 函数名称：Display_Connect()
* 功    能：连机模式下LCM12864模块显示系统信息
* 入口参数：无
* 出口参数：无
* 说    明：显示连机模式信息
******************************************************************************************************************/
void Display_Connect(void);

/******************************************************************************************************************
* 函数名称：Display_find_tag()
* 功    能：LCM12864模块显示系统启动信息
* 入口参数：pro     ISO协议的设定值
*                   0为ISO15693；1为ISO14443A；2为ISO14443B
* 出口参数：无
* 说    明：显示启动信息
******************************************************************************************************************/
void Display_find_tag(unsigned char pro);

/******************************************************************************************************************
* 函数名称：Display_Rssi()
* 功    能：LCM12864模块显示检测到的卷标卡片信号强度图案
* 入口参数：M_rssi    主信道信号强度
* 出口参数：无
* 说    明：显示启动信息
******************************************************************************************************************/
void Display_Rssi(unsigned char M_rssi);

/******************************************************************************************************************
* 函数名称：Display_pro()
* 功    能：LCM12864模块显示读写操作时进度条
* 入口参数：无
* 出口参数：无
* 说    明：显示启动信息
******************************************************************************************************************/
void Display_pro(void);


#endif

