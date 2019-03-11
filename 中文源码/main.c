/**********************************************************************************************************************************************************
* 文 件 名：MAIN.C
* 功    能：RFID阅读器TRF7960系统与计算器连机演示程序。
* 硬件连接：RFID阅读器系统的硬件框图如下所示：
*                                   ————————  
*                                  |               |
*                                  |      LCD      |
*                                  |               |
*                                   ————————
*                                         ||   
*    ————————    (SPI)      ————————               ——————— 
*   |               | <----------> |               |     USB      |             |
*   |    TRF7960    |              |  MSP430F2370  | <----------> |     PC      |
*   |               | <----------> |               |              |             |
*    ————————    (PAR)      ————————               ——————— 
*
* 说    明： TRF7960与MSP430微控制器之间的通信方式为串行SPI或者并行PAR模式，该通信模式由JP2跳线帽选择。              
*                       
* 作    者：EMDOOR
* 日    期：2011年04月13号
**********************************************************************************************************************************************************/
#include <MSP430x23x0.h>                            //微控制器头文件定义
#include <stdlib.h>     
#include <stdio.h>
#include "hardware.h"
#include "communicate.h"
#include "anticollision.h"
#include "automatic.h"
#include "globals.h"
#include "tiris.h"
#include "ISO14443.h"
#include "host.h"
#include "lcd.h"


#define DBG     1                                   //打印信息宏定义

char rxdata;                                        //RS232 接收数据字节
unsigned char buf[BUF_LENGTH];                      //定义MSP430微控制器与TRF7960通信接收缓冲区
signed   char RXTXstate;                            //定义发送接收字节寄存器变量
unsigned char flags;                                //定义存储标志位(在仿冲撞中使用)
unsigned char RXErrorFlag;                          //定义接收错误标志
unsigned char RXflag;                               //定义接收书标志位，用来指示缓冲区中是否有数据
unsigned char i_reg;                                //中断寄存器变量
unsigned char CollPoss;                             //定义冲撞发生位置变量


/******************************************************************************************************************
* 函数名称：main()
* 功    能：主函数入口。
* 入口参数：无
* 出口参数：无
* 说    明：程序从此函数开始运行。
*******************************************************************************************************************/
void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;                       //关闭看门狗

    UARTset();                                      //串口设置

    /* 使能TRF7960芯片 */
    /*====================================================================================================*/
    EnableSet();                                    //使用轮循方法来提高抗干扰能力
    TRFDisable(); 
    delay_ms(1);
    TRFEnable();
    delay_ms(1);
    /*====================================================================================================*/
    
    /* 选择MSP430与TRF7960之间通信模式 */
    /*====================================================================================================*/
    if (SPIMODE)                                    //串行SPI模式
    {                                               //为串行SPI模式设置管脚功能
        EnableSet();
        IRQPinset();                                //选择IRQ中断管脚
        IRQEDGEset();                               //配置上升沿中断模式

        Port1Set();                                 //MSP430 端口1设置
        LEDallOFF();                                //所有LED协议指示灯均关闭
    }
    else                                            //并行模式
        PARset();                                   //为并行模式设置管脚功能
   /*====================================================================================================*/

    if (SPIMODE)                                    //串行SPI模式下配置MSP430 SPI管脚功能等
    {
#ifndef SPI_BITBANG
        USARTset();                                 //设置USART
#else
        SlaveSelectPortSet();                       //MSP430管脚P3.0 Slave Select功能设置
        H_SlaveSelect();                            //Slave Select禁止(高)
        SIMOSet();
        CLKPOUTset();
#endif
    }

    TRFDisable();
    TRFEnable();
    delay_ms(1);
    InitialSettings();                              //初始化设置：设置MSP430时钟频率为6.78MHz及OOK调制模式

    OSCsel(0x00);                                   //设置选择外部晶体振荡器
    delay_ms(10);
    BeepON();
    delay_ms(10);
    BeepOFF();
    
    if (SPIMODE)                                    //若为SPI模式需使用外部晶体重新配置USART
    {
#ifndef SPI_BITBANG
        USARTEXTCLKset();                           //配置USART
#endif
    }

    EnableInterrupts();                             //使能中断
    
    /* LCM图形显示模块初始化及系统运行开机画面显示 */
    /*====================================================================================================*/
    LCM12864_Init();
    Display_Clear();
    Display_Picture((unsigned char *)logo);
    delay_ms(500);
    Display_Picture((unsigned char *)emdoor);
    delay_ms(1000); 
    Display_Clear();
    Display_Start();
    delay_ms(500);
    /*====================================================================================================*/
    
    delay_ms(10);                                   //系统开始运行,各协议指示灯全亮
    LEDtagitON();delay_ms(100);                     //LED依次循环点亮熄灭
    LEDtypeBON();delay_ms(100); 
    LEDtypeAON();delay_ms(100);
    LED15693ON();delay_ms(100);
    LED15693OFF();delay_ms(100);
    LEDtypeAOFF();delay_ms(100);
    LEDtypeBOFF();delay_ms(100);
    LEDtagitOFF();delay_ms(100);

    Display_Put_Tag();                              //显示“请放入卷标”，系统进入检测状态
    
    OOKdirIN();                                     //设置OOK管脚为三态门状态
    ENABLE = 1;
    POLLING = 1;                                    //默认为LCD脱机模式
    
    while(1)                                        //等待串口接收中断
    {
        FindTags(0x00);                             //寻找各种协议标准卷标卡片
    }
}



