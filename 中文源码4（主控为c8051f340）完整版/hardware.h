/********************************************************************************************************
* 文 件 名：COMMUNICATE.H
* 功    能：RFID阅读器TRF7960与C8051F340微控制器之间通信方式头文件。
* 硬件连接：C8051F340与TRF7960之间通信硬件连接关系如下所示：
*                C8051F340                 TRF7960
*********************    PARALLEL INTERFACE    ******************************************         
*               P0.7   				 IRQ
*			    P0.3                 Slave_select
*               P0.2                 SIMO
*               P0.1                 SOMI
*               P0.0                 DATA_CLK
*				P4.0		       	 MOD
*				P4.2				 ASK/OOK
*				P4.3				 EN
*
* 版    本：V1.0
* 作    者：EMDOOR
* 日    期：2011-9-29
*********************************************************************************************************/
#ifndef HARDWARE_H
#define HARDWARE_H

#include <c8051f340.h>            
#include <stdio.h>
#include <globals.h>

/*-----------------------------------------------------------------------------*/

/* TRF7960使能管脚连接 - C8051F340管脚P4.3 */
/* EN           -       P4.3 */
/*-----------------------------------------------------------------------------*/
#define EnableSet()         P4MDOUT |= 0X08               //设置方向为输出
#define TRFDisable()	    P4 &= ~0X08             //输出低电平
#define TRFEnable()         P4 |= 0X08              //输出高电平
 /*-----------------------------------------------------------------------------*/
/* TRF7960时钟CLK管脚连接 - C8051F340管脚P0.0 */
/* CLK          -       P0.0*/
/*-----------------------------------------------------------------------------*/
//#define CLKGPIOset()        P0SKIP |=0x01              //设置为GPIO功能
//#define CLKFUNset()         P0SKIP   &=~0x01            //设置为主功能函数
#define CLKPOUTset()        P0MDOUT |= 0X01               //设置方向为输出
#define CLKON()             P0 |= 0X01             //输出高电平
#define CLKOFF()            P0 &= ~0X01            //输出低电平

//* TRF7960串行SlaveSelect管脚连接 - C8051F340管脚P0.3 */
/* SlaveSelect  -       P0.3 */
/*-----------------------------------------------------------------------------*/
#define SlaveSelectPin          0X08                        
#define SlaveSelectPortSet()    P0MDOUT |= SlaveSelectPin     //设置方向为输出
#define H_SlaveSelect()         P0 |= SlaveSelectPin     //输出高电平
#define L_SlaveSelect()         P0 &= ~SlaveSelectPin    //输出低电平
/*-----------------------------------------------------------------------------*/


/* TRF7960串行SIMO管脚连接 - C8051F340管脚P0.2 */
/* SIMO         -       P0.2 */
/*-----------------------------------------------------------------------------*/
#define SIMOSet()           P0MDOUT |= 0X04               //设置方向为输出
#define SIMOON()            P0 |= 0X04               //输出高电平
#define SIMOOFF()           P0 &= ~0X04             //输出低电平
/*-----------------------------------------------------------------------------*/

/* TRF7960串行SOMI管脚连接 - C8051F340管脚P0.1 */
/* SOMI         -       P0.1 */
/*-----------------------------------------------------------------------------*/
#define SOMISIGNAL          P0 & 0X02
#define SOMISIGNALSET()		P0MDIN |= 0X02
/*-----------------------------------------------------------------------------*/

/* TRF7960中断IRQ管脚连接 - C8051F340管脚P0.7 */
/* IRQ          -       P2.1 */
/*-----------------------------------------------------------------------------*/
#define IRQPin              0X80                        //第7脚
#define IRQPORT             P0                        //端口0
#define IRQPinset()         P0MDIN |= IRQPin            //设置方向为输入
#define IRQON()             IE |= 0X01              //IRQ 中断开启
#define IRQOFF()            IE &= ~0X01            //IRQ 中断关闭
//#define IRQEDGEset()        TCON |= 0X01           //上升沿中断
#define IRQCLR()            TCON  &= ~0X02              //清中断标志位
#define IRQREQreg()         TCON                       //中断寄存器     
/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/

/* TRF7960 OOK管脚连接 - C8051F340管脚P4.2 */
/* OOK          -       P2.2 */
/*-----------------------------------------------------------------------------*/
#define OOKPin              0X04                        
#define OOKdirIN()          P4MDIN |= OOKPin            //设置方向为输入     
#define OOKdirOUT()         P4MDOUT |= OOKPin            //设置方向为输出
#define OOKoff()            P4 &= ~OOKPin            //OOK关闭
#define OOKon()             P4 |= OOKPin             //OOK开启
/*====================================================================================================*/


/* 计数定时器常量定义 */
/*====================================================================================================*/
#define StartCounter()        TCON |=  0X10               //开定时器
#define StopCounter()         TCON &= ~0X10       //停止计数
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

void IRQInit(void);
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
void Timer0_Delay (int ms);
/******************************************************************************************************************
* 函数名称：OSCsel()
* 功    能：晶体振荡器选择
* 入口参数：mode        选择内部或者外部模式
* 出口参数：无
*********************************************************************************************************************/
void OSCsel(void);
/******************************************************************************************************************
* 函数名称：Timer2handler()
* 功    能：定时器中断服务程序
* 入口参数：无
* 出口参数：无
* 说    明：定时器中断服务程序，用于精确延时用。
*******************************************************************************************************************/
void Timer0handler(void);

#endif

