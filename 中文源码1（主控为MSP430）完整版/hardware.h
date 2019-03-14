/**********************************************************************************************************************************************************
* 文 件 名：HARDWARE.H
* 功    能：RFID阅读器TRF7960与MSP430F2370微控制器之间的硬件连接头文件。
* 硬件连接：MSP430F2370与TRF7960的硬件连接关系如下所示：
*               MSP430F2370                 TRF7960
*********************    PARALLEL INTERFACE    ******************************************
*               P4.0(Pin26)                 DATA0(Pin17)
*               P4.1(Pin27)                 DATA1(Pin18)
*               P4.2(Pin28)                 DATA2(Pin19)
*               P4.3(Pin29)                 DATA3(Pin20)
*               P4.4(Pin30)                 DATA4(Pin21)
*               P4.5(Pin31)                 DATA5(Pin22)
*               P4.6(Pin32)                 DATA6(Pin23)
*               P4.7(Pin33)                 DATA7(Pin24)
*********************    FUNCTION INTERFACE    ******************************************
*               P2.0(Pin12)                 MOD(Pin14)
*               P2.1(Pin13)                 IRQ(Pin13)
*               P2.2(Pin14)                 ASK/OOK(Pin12)
*
*               P1.0(Pin4)                  EN(Pin28)
*               XIN/P2.6(Pin2)              SYS_CLK(Pin24)
*               P3.3(Pin21)                 DATA_CLK(Pin26)
*********************    SERIAL(SPI) INTERFACE    ***************************************           
*               P3.2(Pin20)                 SOMI(Pin23)
*               P3.1(Pin19)                 SIMO(Pin24)
*               P3.0(Pin18)                 Slave_select(Pin21)
*
* 作    者：EMDOOR
* 日    期：2011年04月13号
**********************************************************************************************************************************************************/
#ifndef HARDWARE_H
#define HARDWARE_H


#include <MSP430x23x0.h>            
#include <stdio.h>
#include "globals.h"

/*
#ifndef HIGHSPEED                                       //定义串行SPI高速模式
#define HIGHSPEED
#endif
*/

/* MSP430微控制器连接定义 */
/*====================================================================================================*/
/* TRF7960 数据I/O管脚连接 - MSP430F2370管脚PORT 4*/
/*  DATA[0..7]   -       PORT 4 */
/*-----------------------------------------------------------------------------*/
#define TRFWrite(x)         P4OUT = x                   //8位管脚写操作定义
#define TRFWriteBit(x)      P4OUT |= x                  //单独位写定义
#define TRFRead(x)          x = P4IN                    //读操作

#define TRFDirIN()          P4DIR = 0x00                //管脚输入
#define TRFDirOUT()         P4DIR = 0xFF                //管脚输出
#define TRFFunc()           P4SEL = 0x00                //管脚功能选择
/*-----------------------------------------------------------------------------*/

/* TRF7960使能管脚连接 - MSP430F2370管脚P1.0 */
/* EN           -       P1.0 */
/*-----------------------------------------------------------------------------*/
#define EnableSet()         P1DIR |= BIT0               //设置方向为输出
#define TRFEnable()         P1OUT |= BIT0               //输出高电平
#define TRFDisable()        P1OUT &= ~BIT0              //输出低电平
/*-----------------------------------------------------------------------------*/

/* TRF7960时钟CLK管脚连接 - MSP430F2370管脚P3.3 */
/* CLK          -       P3.3 */
/*-----------------------------------------------------------------------------*/
#define CLKGPIOset()        P3SEL &= ~BIT3              //设置为GPIO功能
#define CLKFUNset()         P3SEL |= BIT3               //设置为主功能函数
#define CLKPOUTset()        P3DIR |= BIT3               //设置方向为输出
#define CLKON()             P3OUT |= BIT3               //输出高电平
#define CLKOFF()            P3OUT &= ~BIT3              //输出低电平
/*-----------------------------------------------------------------------------*/

/* TRF7960串行SPIMODE管脚连接 - MSP430F2370管脚P2.3 */
/* SPIMODE      -       P2.3 */
/*-----------------------------------------------------------------------------*/
#define SPIMODE             P2IN & BIT3             //VCC-SPI;GND-PAR


/* TRF7960串行SlaveSelect管脚连接 - MSP430F2370管脚P3.0 */
/* SlaveSelect  -       P3.0 */
/*-----------------------------------------------------------------------------*/
#define SlaveSelectPin          BIT0                        
#define SlaveSelectPortSet()    P3DIR |= SlaveSelectPin     //设置方向为输出
#define H_SlaveSelect()         P3OUT |= SlaveSelectPin     //输出高电平
#define L_SlaveSelect()         P3OUT &= ~SlaveSelectPin    //输出低电平
/*-----------------------------------------------------------------------------*/

/* TRF7960串行SIMO管脚连接 - MSP430F2370管脚P3.1 */
/* SIMO         -       P3.1 */
/*-----------------------------------------------------------------------------*/
#define SIMOSet()           P3DIR |= BIT1               //设置方向为输出
#define SIMOON()            P3OUT |= BIT1               //输出高电平
#define SIMOOFF()           P3OUT &= ~BIT1              //输出低电平
/*-----------------------------------------------------------------------------*/

/* TRF7960串行SOMI管脚连接 - MSP430F2370管脚P3.2 */
/* SOMI         -       P3.2 */
/*-----------------------------------------------------------------------------*/
#define SOMISIGNAL          P3IN & BIT2
/*-----------------------------------------------------------------------------*/

/* TRF7960中断IRQ管脚连接 - MSP430F2370管脚P2.1 */
/* IRQ          -       P2.1 */
/*-----------------------------------------------------------------------------*/
#define IRQPin              BIT1                        //第1脚
#define IRQPORT             P2IN                        //端口2
#define IRQPinset()         P2DIR &= ~IRQPin            //设置方向为输入
#define IRQON()             P2IE |= IRQPin              //IRQ 中断开启
#define IRQOFF()            P2IE &= ~IRQPin             //IRQ 中断关闭
#define IRQEDGEset()        P2IES &= ~IRQPin            //上升沿中断
#define IRQCLR()            P2IFG = 0x00                //清中断标志位
#define IRQREQreg()         P2IFG                       //中断寄存器                    
/*-----------------------------------------------------------------------------*/

/* TRF7960协议指示灯管脚连接 - MSP430F2370管脚 */
/* Tag-it       -       P1.1 */
/* ISO14443B    -       P1.2 */
/* ISO14443A    -       P1.3 */
/* ISO15693     -       P1.4 */
/* Beep         -       P1.5 */
/*-----------------------------------------------------------------------------*/
#define Port1Set()          P1DIR |= 0xFF               //设置方向为输出
#define LEDallOFF()         P1OUT &= ~0x1E              //关闭所有LED灯
#define LEDallON()          P1OUT |= 0x1E               //开启所有LED灯

#define LEDtagitON()        P1OUT |= BIT1               //Tag-it指示灯亮
#define LEDtagitOFF()       P1OUT &= ~BIT1              //Tag-it指示灯灭
#define LEDtypeBON()        P1OUT |= BIT2               //ISO14443B指示灯亮
#define LEDtypeBOFF()       P1OUT &= ~BIT2              //ISO14443B指示灯灭
#define LEDtypeAON()        P1OUT |= BIT3               //ISO14443A指示灯亮
#define LEDtypeAOFF()       P1OUT &= ~BIT3              //ISO14443A指示灯灭
#define LED15693ON()        P1OUT |= BIT4               //ISO15693示灯亮
#define LED15693OFF()       P1OUT &= ~BIT4              //ISO15693示灯灭

#define BeepON()            P1OUT |= BIT5//P1OUT &= ~BIT5              //蜂鸣器开
#define BeepOFF()           P1OUT &= ~BIT5//P1OUT |= BIT5               //蜂鸣器关
/*-----------------------------------------------------------------------------*/

/* LCM图形点阵显示屏 - MSP430F2370管脚连接 */
/* LCM_SCL      -       P1.6 */
/* LCM_SID      -       P1.7 */
/* LCM_A0       -       P3.6 */
/* LCM_nCS      -       P2.4 */
/* LCM_nRST     -       P2.5 */
/*-----------------------------------------------------------------------------*/
#define H_LCM_SCL()         P1OUT |= BIT6               //输出高电平
#define L_LCM_SCL()         P1OUT &= ~BIT6              //输出低电平
#define H_LCM_SID()         P1OUT |= BIT7               //输出高电平
#define L_LCM_SID()         P1OUT &= ~BIT7              //输出低电平

#define LCMA0Set()          P3DIR |= BIT6               //设置方向为输出
#define H_LCM_A0()          P3OUT |= BIT6               //输出高电平
#define L_LCM_A0()          P3OUT &= ~BIT6              //输出低电平

#define LCMnCSSet()         P2DIR |= BIT4               //设置方向为输出
#define H_LCM_nCS()         P2OUT |= BIT4               //输出高电平
#define L_LCM_nCS()         P2OUT &= ~BIT4              //输出低电平

#define LCMnRSTSet()        P2DIR |= BIT5               //设置方向为输出
#define H_LCM_nRST()        P2OUT |= BIT5               //输出高电平
#define L_LCM_nRST()        P2OUT &= ~BIT5              //输出低电平
/*-----------------------------------------------------------------------------*/

/* TRF7960 OOK管脚连接 - MSP430F2370管脚P2.2 */
/* OOK          -       P2.2 */
/*-----------------------------------------------------------------------------*/
#define OOKPin              BIT2                        
#define OOKdirIN()          P2DIR &= ~OOKPin            //设置方向为输入     
#define OOKdirOUT()         P2DIR |= OOKPin             //设置方向为输出
#define OOKoff()            P2OUT &= ~OOKPin            //OOK关闭
#define OOKon()             P2OUT |= OOKPin             //OOK开启
/*====================================================================================================*/


/* 计数定时器常量定义 */
/*====================================================================================================*/
#define CountValue          TACCR0                      //计数器寄存器 TACCR0
#define StartCounter        TACTL |=  MC1               //开始以递增模式计数
#define StopCounter()       TACTL &= ~(MC0 + MC1)       //停止计数
/*====================================================================================================*/


/* 定义高速模式模式 */
/*====================================================================================================*/
#ifndef HIGHSPEED
    #define count1ms            847
#else
    #define count1ms            1694
#endif
/*====================================================================================================*/


/* TRF7960芯片地址定义 */
/*====================================================================================================*/
#define ChipStateControl        0x00                        //芯片状态控制寄存器
#define ISOControl              0x01                        //ISO协议控制寄存器
#define ISO14443Boptions        0x02                        //ISO14443B协议可选寄存器
#define ISO14443Aoptions        0x03                        //ISO14443A协议可选寄存器
#define TXtimerEPChigh          0x04                        //TX发送高字节寄存器
#define TXtimerEPClow           0x05                        //TX发送低字节寄存器
#define TXPulseLenghtControl    0x06                        //TX发送脉冲长度控制寄存器
#define RXNoResponseWaitTime    0x07                        //RX接收无应答等待时间寄存器
#define RXWaitTime              0x08                        //RX接收等待时间寄存器
#define ModulatorControl        0x09                        //调制器及系统时钟寄存器
#define RXSpecialSettings       0x0A                        //RX接收特殊设置寄存器
#define RegulatorControl        0x0B                        //电压调整和I/O控制寄存器
#define IRQStatus               0x0C                        //IRQ中断状态寄存器
#define IRQMask                 0x0D                        //冲撞位置及中断标记寄存器
#define CollisionPosition       0x0E                        //冲撞位置寄存器
#define RSSILevels              0x0F                        //信号接收强度寄存器
#define RAMStartAddress         0x10                        //RAM 开始地址，共7字节长 (0x10 - 0x16)
#define NFCID                   0x17                        //卡片标识符
#define NFCTargetLevel          0x18                        //目标卡片RSSI等级寄存器
#define NFCTargetProtocol       0x19                        //目标卡片协议寄存器
#define TestSetting1            0x1A                        //测试设置寄存器1
#define TestSetting2            0x1B                        //测试设置寄存器2
#define FIFOStatus              0x1C                        //FIFO状态寄存器
#define TXLenghtByte1           0x1D                        //TX发送长度字节1寄存器
#define TXLenghtByte2           0x1E                        //TX发送长度字节2寄存器
#define FIFO                    0x1F                        //FIFO数据寄存器


/* TRF7960芯片命令定义 */
/*====================================================================================================*/
#define Idle                    0x00                        //空闲命令
#define SoftInit                0x03                        //软件初始化命令
#define InitialRFCollision      0x04                        //初始化RF冲撞命令
#define ResponseRFCollisionN    0x05                        //应答RF冲撞N命令
#define ResponseRFCollision0    0x06                        //应答RF冲撞0命令
#define Reset                   0x0F                        //复位命令
#define TransmitNoCRC           0x10                        //传送无CRC校验命令
#define TransmitCRC             0x11                        //传送带CRC校验命令
#define DelayTransmitNoCRC  	0x12                        //定时传送无CRC校验命令
#define DelayTransmitCRC    	0x13                        //定时传送带CRC校验命令
#define TransmitNextSlot        0x14                        //传送下一个槽命令
#define CloseSlotSequence       0x15                        //关闭槽序列命令
#define StopDecoders            0x16                        //停止解碼命令
#define RunDecoders             0x17                        //运行解碼命令
#define ChectInternalRF         0x18                        //清空内部RF接收命令
#define CheckExternalRF         0x19                        //清空外部RF接收命令
#define AdjustGain              0x1A                        //增益调节命令
/*====================================================================================================*/

/******************************************************************************************************************
* 函数名称：delay_ms()
* 功    能：延时函数。
* 入口参数：n_ms        毫秒值
* 出口参数：无
* 说    明：该函数为延时函数，入口参数值越大，CPU延时等待的时间就越长。
*           本演示程序，将其应用于非精确的延时等待场合。
*           注意：入口参数的最大值为 65536。           
*******************************************************************************************************************/
void delay_ms(unsigned int n_ms);

/******************************************************************************************************************
* 函数名称：CounterSet()
* 功    能：定时器设置函数。
* 入口参数：无
* 出口参数：无
* 说    明：初始化设置定时器寄存器。 本演示程序，将其应用于精确的延时等待场合。          
*******************************************************************************************************************/
void CounterSet(void);

/******************************************************************************************************************
* 函数名称：OSCsel()
* 功    能：晶体振荡器选择
* 入口参数：mode        选择内部或者外部模式
* 出口参数：无
* 说    明：配置晶体振荡器模式。若入口参数为0x00，则选择外部晶体模式。
*           若为非0x00的任意值，则选择内部DCO模式。
*******************************************************************************************************************/
void OSCsel(unsigned char mode);

/******************************************************************************************************************
* 函数名称：TimerAhandler()
* 功    能：定时器中断服务程序
* 入口参数：无
* 出口参数：无
* 说    明：定时器中断服务程序，用于精确延时用。
*******************************************************************************************************************/
#pragma vector=TIMERA0_VECTOR
__interrupt void TimerAhandler(void);

/******************************************************************************************************************
* 函数名称：Beep_Waring()
* 功    能：蜂鸣器报警发声函数。
* 入口参数：n       发声次数
*           t       发声频率
* 出口参数：无
* 说    明：蜂鸣器发声函数根据入口参数t的不同，而发出不同频率的响声。
*           根据n值执行发声次数。    
*******************************************************************************************************************/
void Beep_Waring(unsigned char n, unsigned char t);


#endif

