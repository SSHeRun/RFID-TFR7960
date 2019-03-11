/*************************************************************************************************
* 文 件 名：MAIN.C
* 功    能：RFID阅读器TRF7960系统与计算器连机演示程序。
*
* 说    明： TRF7960与C8051F340微控制器之间的通信方式为串行SPI或者并行PAR模式。              
*                       
* 作    者：EMDOOR
* 日    期：2011-9-29
***************************************************************************************************/
#include <c8051f340.h>                            //微控制器头文件定义
#include <host.h>
#include <hardware.h>
#include <automatic.h>
#include <communicate.h>
#include <globals.h>                 

char     rxdata;                                    //RS232 接收数据字节
xdata    unsigned char buf[BUF_LENGTH];             //定义MSP430微控制器与TRF7960通信接收缓冲区
signed   char RXTXstate;                            //定义发送接收字节寄存器变量
unsigned char flags;                                //定义存储标志位(在仿冲撞中使用)
unsigned char RXErrorFlag;                          //定义接收错误标志
unsigned char RXflag;                               //定义接收书标志位，用来指示缓冲区中是否有数据
unsigned char i_reg;                                //中断寄存器变量
unsigned char CollPoss;                             //定义冲撞发生位置变量

/**************************************************************************************************
* 函数名称：main()
* 功    能：主函数入口。
* 入口参数：无
* 出口参数：无
* 说    明：程序从此函数开始运行。
****************************************************************************************************/
void main(void)
{
//==================================================================================================
    PCA0MD   &= ~0x40;
    PCA0MD    = 0x00;   //关闭看门狗
//==================================================================================================  
    OSCsel();  //选择晶体振荡器
  	PORT_Init();//初始化串口端口
    UART0_Init();  //初始化串口
   
 //使用轮循方法来提高抗干扰能力
/*=================================================================================================*/
    EnableSet(); //设置为输出                                      
		TRFDisable(); //禁止读卡器  
		delay_ms(1);  
		TRFEnable();//使能读卡器   
		delay_ms(1);  
		IRQPinset();                //选择IRQ中断管脚
    IRQInit();
  
    SlaveSelectPortSet();       //c8051f340P0.3Slave Select功能设置
    H_SlaveSelect();            //Slave Select禁止(高)
    SIMOSet();					//设置SIMO为输出
    SOMISIGNALSET();
		CLKPOUTset();				//设置时钟引脚为输出

    TRFDisable();
		delay_ms(1);
    TRFEnable();
    delay_ms(1);
	
    InitialSettings();           //初始化设置：设置MSP430时钟频率为6.78MHz及OOK调制模式
    EnableInterrupts();          //使能总中断
    
    OOKdirIN();                  //设置OOK管脚为三态门状态
    
    while(1)                                      
    { 
                                 //寻找各种协议标准卷标卡片
		 FindTags();
	  
    }
}



