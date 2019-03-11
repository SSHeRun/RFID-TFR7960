/********************************************************************************************************************
* 文 件 名：HOST.C
* 功    能：串口相关的功能函数以及设置
*
* 作    者：EMDOOR
* 日    期：2011-9-29
*********************************************************************************************************************/
#include <C8051F340.h>
#include <hardware.h>
#include <communicate.h>
#include <anticollision.h>
#include <globals.h>
#include <host.h>

unsigned char   RXdone;                             //接收完整数据标志位，若接收完成，置该标志位1

unsigned char   Firstdata = 1;                      //设置串口同步标志位
unsigned char   ENABLE;                             //TRF7960??????????,1??;0????
#define BAUDRATE           115200      // UART波特率设置
#define SYSCLK             12000000    // 内部的晶振频率

/******************************************************************************************************************
* 函数名称：PORT_Init()
* 功    能：串口初始化设置函数
* 入口参数：无
* 出口参数：无   
*****************************0.**************************************************************************************/
void PORT_Init (void)
{  
   
   P0MDOUT   = 0x10;
   XBR0      = 0x01;
   XBR1      = 0x40;
}


void UART0_Init(void)
{	
    SCON0 = 0x10;   //8位数据位，1位停止位，使能接收                   
 
   if (SYSCLK/BAUDRATE/2/256 < 1)
   {
      TH1 = -(SYSCLK/BAUDRATE/2);
      CKCON &= ~0x0B;                  // T1M = 1; SCA1:0 = xx
      CKCON |=  0x08;
   } 
   else if (SYSCLK/BAUDRATE/2/256 < 4)
   {
      TH1 = -(SYSCLK/BAUDRATE/2/4);
      CKCON &= ~0x0B;                  // T1M = 0; SCA1:0 = 01
      CKCON |=  0x09;
   }
   else if (SYSCLK/BAUDRATE/2/256 < 12)
   {
      TH1 = -(SYSCLK/BAUDRATE/2/12);
      CKCON &= ~0x0B;                  // T1M = 0; SCA1:0 = 00
   } else
   {
      TH1 = -(SYSCLK/BAUDRATE/2/48);
      CKCON &= ~0x0B;                  // T1M = 0; SCA1:0 = 10
      CKCON |=  0x02;
   }

   TL1 = TH1;                          // init Timer1
   TMOD &= ~0xf0;                      // TMOD: timer 1 in 8-bit autoreload
   TMOD |=  0x20;
   TR1 = 1;                            // START Timer1
   TI0 = 1;                            // Indicate TX0 ready
}



/******************************************************************************************************************
* 函数名称：sendchar()
* 功    能：向计算器上位机发送一个字符函数
* 入口参数：TXchar    将要被发送的字符
* 出口参数：无
* 说    明：无   
*******************************************************************************************************************/
sendchar(char TXchar)
{
     
      if (TXchar == '\n')  {                // check for newline character
         while (!TI0);                 // wait until UART0 is ready to transmit
         TI0 = 0;                      // clear interrupt flag
         SBUF0 = 0x0d;                 // output carriage return command
      }
      while (!TI0);                    // wait until UART0 is ready to transmit
      TI0 = 0;
	  SBUF0 = TXchar;                         // clear interrupt flag
      return (SBUF0);              // output <c> using UART 0
}


/******************************************************************************************************************
* 函数名称：send_crlf()
* 功    能：向计算器上位机发送一个回车+换行符号
* 入口参数：无
* 出口参数：无
* 说    明：无   
*******************************************************************************************************************/
void send_crlf(void)
{
    sendchar('\r');                                 //发送回车符号
    sendchar('\n');                                 //发送换行符号
}

/******************************************************************************************************************
* 函数名称：send_cstring()
* 功    能：向计算器上位机发送一字符串
* 入口参数：*pstr       将要被发送的字符串
* 出口参数：无
* 说    明：无   
*******************************************************************************************************************/
void send_cstring(char *pstr)
{
    while(*pstr != '\0')                            //查询是否到达字符串尾
    {
        sendchar(*pstr++);                          //发送字符
    }
}

/******************************************************************************************************************
* 函数名称：Nibble2Ascii()
* 功    能：半位字节转化成ASCII十六进制码
* 入口参数：anibble         将要被转换字节
* 出口参数：AsciiOut        转换后的ASCII十六进制码值
* 说    明：无   
*******************************************************************************************************************/
unsigned char Nibble2Ascii(unsigned char anibble)
{
    unsigned char AsciiOut = anibble;               //定义转换后的变量AsciiOut，并赋值

    if(anibble > 9)                                 //如果被转换的半位字节为A-F，则需要加0x07
        AsciiOut = AsciiOut + 0x07;

    AsciiOut = AsciiOut + 0x30;                     //其他情况则在转换结果基础上增加偏移量0x30

    return(AsciiOut);                               //返回转换后的值
}

/******************************************************************************************************************
* 函数名称：Put_byte()
* 功    能：发送字节函数半位字节转化成ASCII十六进制码
* 入口参数：abyte         将要被发送字节
* 出口参数：无      
* 说    明：该函数调用两次Nibble2Ascii，将一个字节拆分成高低四位，先转换再次序发送。
*******************************************************************************************************************/
void send_byte(unsigned char abyte)
{
    unsigned char temp1, temp2;

    temp1 = (abyte >> 4) & 0x0F;                    //获取高四位字节
    temp2 = Nibble2Ascii(temp1);                    //转换成ASCII码
    sendchar(temp2);                                //发送之

    temp1 = abyte & 0x0F;                           //获取低四位字节
    temp2 = Nibble2Ascii(temp1);                    //转换成ASCII码
    sendchar(temp2);                                //发送之
}
unsigned char Get_nibble(void)
{
    unsigned char reading;                          //????
    //unsigned char rxdata;
		send_cstring("Get_nibble");
    reading = 1;                                    //????1 ????????
    while(reading)                                  //??????
    {                   		
        //LPM0; 
				PCON |= 0x01;				//???????,????
        if(rxdata >= 'a')                           //???????
        {
            rxdata -= 32;
        }

        /* ???????,???? */
        /*====================================================================================================*/
        if(((rxdata >= '0') && (rxdata <= '9')) || ((rxdata >= 'A') && (rxdata <= 'F')))
        {
            reading = 0;
            sendchar(rxdata);                       //??????
        
            if(rxdata > '9')                        //??ASCII?????A-F,??9
            {       
                rxdata = (rxdata & 0x0F) + 9;
            }
        }
        /*====================================================================================================*/
    }
    
    return(rxdata);                                 //??????                             
}

void RXhandler (void) interrupt 4
{
    if(RI0==1)                            //?????????
    {   
        rxdata = SBUF0;                         //??????UCA0RXBUF?????rxdata
        RXdone = 1;                                 //???????
        if(ENABLE == 0)                             //TRF7960??????
        {
            TRFEnable();                            //??TRF790
            //BaudSet(0x00);                          //?????
            OSCsel();                           //????????? 
               
            InitialSettings();                      //???TRF7960
            send_cstring("Reader enabled.");        //????????
            ENABLE = 1;                             //??TRF7960???
        }
       PCON &= ~0X02;

        if(Firstdata)                               //????1??????
        {
            
            IRQOFF();                               //??IRQ??
            StopCounter();                          //?????
         
        }
    }
}
