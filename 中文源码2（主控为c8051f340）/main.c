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
#include <crc.h>

char     rxdata;                                    //RS232 接收数据字节
xdata    unsigned char buf[BUF_LENGTH];             //定义MSP430微控制器与TRF7960通信接收缓冲区
signed   char RXTXstate;                            //定义发送接收字节寄存器变量
unsigned char flags;                                //定义存储标志位(在仿冲撞中使用)
unsigned char RXErrorFlag;                          //定义接收错误标志
unsigned char RXflag;                               //定义接收书标志位，用来指示缓冲区中是否有数据
unsigned char i_reg;                                //中断寄存器变量
unsigned char CollPoss;                             //定义冲撞发生位置变量

extern unsigned char frames[8];  
extern unsigned char num;
extern bit receiveOver;

extern bit flag1=0;

void Uart0_Transmit(unsigned char tmp);
/**
  * @file   main.c
	* @brief  Analysis frames 
  * @param  None
  * @retval None
  */
void analysisFrames()
{
		unsigned char arr[4];  
		unsigned char numOfTags = 0;
	  unsigned char datas[2];
		arr[0] = arr[1] = arr[2] = arr[3] = num;  
		receiveOver = 0;
	//	if(checkCRC(frames,num))       //data is right
	//	{
	       
				do
				{
					FindTags();
	      switch(frames[2])
				{
					case 0x11:
						flag1=1;
					  break;
					case 0x12:
						flag1=0;
						Read_Block_Command(frames[3]);
					  
					  break;
					case 0x13:
						flag1=0;
						Get_Info_Command();
					  break;
					case 0x14:
						flag1=0;
					  datas[0]=frames[4];
					  datas[1]=frames[5];
						Write_Block_Command(frames[3],datas);
					  break;
					case 0x15:
						flag1=0;
						Write_AFI_Command(frames[3]);
					  break;
					case 0x16:
						flag1=0;
						Write_DSFID_Command(frames[3]);
					  break;
					case 0x17:
						flag1=0;
						Get_sec_Command(frames[3],frames[4]);
					  break;
				}
//						if(frames[2] == 0x11)  //13.56M 15693协议读
//						{
//							 FindTags();
//								//???
//								//numOfTags = readBlock(frames[3]);  
//						}
//						else if(frames[2] == 0x12) //13.56M 15693协议写
//						{
//								//???
//								//writeBuf[0] = frames[4];
//								//writeBuf[1] = frames[5];
//								//numOfTags = writeBlock(frames[3]);	
//						}
					}	while(!numOfTags && !receiveOver);   //Waiting for a tag or next data frame
		//	}
	//	else
	//	{ 
				//PrintData(arr,4);  
	//	}
}

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
	  SYSCLK_Init();//初始化系统时钟
    UART0_Init();  //初始化串口
    EA = 1;
	  ES0 = 1;
	
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
		// FindTags(); 
	  while(!receiveOver);  //等待数据接收完毕 receiveOver = 1 退出
	  send_cstring(frames);
			analysisFrames();
    }
}

// 串口UART0中断
//-----------------------------------------------------------
//中断向量0x0023
//void UATR0_ISR(void)interrupt 4
//{
//	unsigned char temp;
//    //Rx、Tx共用中断
//    //接收中断
//    if(!TI0)
//    {
//        RI0=0 ;
//        temp=SBUF0 ;
//        Uart0_Transmit(temp);
//    }
//    //发送中断
//    else TI0=0;
//}
////-----------------------------------------------------------
//// 串口UART0发送
////-----------------------------------------------------------
//void Uart0_Transmit(unsigned char tmp)
//{
//    ES0 = 0 ;		//关UART0中断
//    EA = 0 ;		//关全局中断
//    SBUF0 = tmp ;
//    while(TI0 == 0);
//    TI0 = 0 ;
//    ES0 = 1 ;		//开UART0中断
//    EA = 1 ;		//开全局中断
//   
//}



