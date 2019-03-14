/********************************************************************************************************************
* �� �� ����HOST.C
* ��    �ܣ�������صĹ��ܺ����Լ�����
*
* ��    �ߣ�EMDOOR
* ��    �ڣ�2011-9-29
*********************************************************************************************************************/
#include <C8051F340.h>
#include <hardware.h>
#include <communicate.h>
#include <anticollision.h>
#include <globals.h>
#include <host.h>

unsigned char   RXdone;                             //�����������ݱ�־λ����������ɣ��øñ�־λ1

unsigned char   Firstdata = 1;                      //���ô���ͬ����־λ
unsigned char   ENABLE;                             //TRF7960??????????,1??;0????
#define BAUDRATE           115200      // UART����������
#define SYSCLK             12000000    // �ڲ��ľ���Ƶ��

/******************************************************************************************************************
* �������ƣ�PORT_Init()
* ��    �ܣ����ڳ�ʼ�����ú���
* ��ڲ�������
* ���ڲ�������   
*****************************0.**************************************************************************************/
void PORT_Init (void)
{  
   
   P0MDOUT   = 0x10;
   XBR0      = 0x01;
   XBR1      = 0x40;
}


void UART0_Init(void)
{	
    SCON0 = 0x10;   //8λ����λ��1λֹͣλ��ʹ�ܽ���                   
 
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
* �������ƣ�sendchar()
* ��    �ܣ����������λ������һ���ַ�����
* ��ڲ�����TXchar    ��Ҫ�����͵��ַ�
* ���ڲ�������
* ˵    ������   
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
* �������ƣ�send_crlf()
* ��    �ܣ����������λ������һ���س�+���з���
* ��ڲ�������
* ���ڲ�������
* ˵    ������   
*******************************************************************************************************************/
void send_crlf(void)
{
    sendchar('\r');                                 //���ͻس�����
    sendchar('\n');                                 //���ͻ��з���
}

/******************************************************************************************************************
* �������ƣ�send_cstring()
* ��    �ܣ����������λ������һ�ַ���
* ��ڲ�����*pstr       ��Ҫ�����͵��ַ���
* ���ڲ�������
* ˵    ������   
*******************************************************************************************************************/
void send_cstring(char *pstr)
{
    while(*pstr != '\0')                            //��ѯ�Ƿ񵽴��ַ���β
    {
        sendchar(*pstr++);                          //�����ַ�
    }
}

/******************************************************************************************************************
* �������ƣ�Nibble2Ascii()
* ��    �ܣ���λ�ֽ�ת����ASCIIʮ��������
* ��ڲ�����anibble         ��Ҫ��ת���ֽ�
* ���ڲ�����AsciiOut        ת�����ASCIIʮ��������ֵ
* ˵    ������   
*******************************************************************************************************************/
unsigned char Nibble2Ascii(unsigned char anibble)
{
    unsigned char AsciiOut = anibble;               //����ת����ı���AsciiOut������ֵ

    if(anibble > 9)                                 //�����ת���İ�λ�ֽ�ΪA-F������Ҫ��0x07
        AsciiOut = AsciiOut + 0x07;

    AsciiOut = AsciiOut + 0x30;                     //�����������ת���������������ƫ����0x30

    return(AsciiOut);                               //����ת�����ֵ
}

/******************************************************************************************************************
* �������ƣ�Put_byte()
* ��    �ܣ������ֽں�����λ�ֽ�ת����ASCIIʮ��������
* ��ڲ�����abyte         ��Ҫ�������ֽ�
* ���ڲ�������      
* ˵    �����ú�����������Nibble2Ascii����һ���ֽڲ�ֳɸߵ���λ����ת���ٴ����͡�
*******************************************************************************************************************/
void send_byte(unsigned char abyte)
{
    unsigned char temp1, temp2;

    temp1 = (abyte >> 4) & 0x0F;                    //��ȡ����λ�ֽ�
    temp2 = Nibble2Ascii(temp1);                    //ת����ASCII��
    sendchar(temp2);                                //����֮

    temp1 = abyte & 0x0F;                           //��ȡ����λ�ֽ�
    temp2 = Nibble2Ascii(temp1);                    //ת����ASCII��
    sendchar(temp2);                                //����֮
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
