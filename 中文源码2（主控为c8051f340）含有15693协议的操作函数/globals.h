/************************************************************************************************************************
* 文 件 名：GLOBALS.H
* 功    能：程序全局变量和声明定义。
*           这些全局变量和声明被用于所有的头文件和C文件中。
* 版    本：V1.0
* 作    者：EMDOOR
* 日    期：2011-9-29
***************************************************************************************************************************/
#ifndef GLOBALS_H
#define GLOBALS_H                 
#include <c8051f340.h>

#define BUF_LENGTH                      300                 //最大帧字节数量
#define EnableInterrupts()              IE |= 0X80          //使能中断

extern char rxdata;                                         //RS232 接收数据字节
extern xdata unsigned char buf[BUF_LENGTH];                       //外部声明MSP430微控制器与TRF7960通信接收缓冲区 
extern signed char RXTXstate;                               //外部声明发送接收字节寄存器变量
extern unsigned char flags;                                 //外部存储存储标志位(在仿冲撞中使用)
extern unsigned char RXErrorFlag;                           //外部声明接收错误标志
extern unsigned char RXflag;                                //定义接收书标志位，用来指示缓冲区中是否有数据
extern unsigned char i_reg;                                 //外部声明中断寄存器变量
extern unsigned char CollPoss;                              //外部声明冲撞发生位置变量
extern unsigned char RXdone;                                //外部声明接收完整数据标志位，若接收完成，置该标志位1
extern unsigned char Found_tag;                             //定义是否检测到卡片全局变量
extern unsigned char rssival;                               //定义检测到的卡片接收信号强度值

#endif


