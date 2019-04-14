/*************************************************************************************************
* �� �� ����MAIN.C
* ��    �ܣ�RFID�Ķ���TRF7960ϵͳ�������������ʾ����
*
* ˵    ���� TRF7960��C8051F340΢������֮���ͨ�ŷ�ʽΪ����SPI���߲���PARģʽ��              
*                       
* ��    �ߣ�EMDOOR
* ��    �ڣ�2011-9-29
***************************************************************************************************/
#include <c8051f340.h>                            //΢������ͷ�ļ�����
#include <host.h>
#include <hardware.h>
#include <automatic.h>
#include <communicate.h>
#include <globals.h>   
#include <crc.h>

char     rxdata;                                    //RS232 ���������ֽ�
xdata    unsigned char buf[BUF_LENGTH];             //����MSP430΢��������TRF7960ͨ�Ž��ջ�����
signed   char RXTXstate;                            //���巢�ͽ����ֽڼĴ�������
unsigned char flags;                                //����洢��־λ(�ڷ³�ײ��ʹ��)
unsigned char RXErrorFlag;                          //������մ����־
unsigned char RXflag;                               //����������־λ������ָʾ���������Ƿ�������
unsigned char i_reg;                                //�жϼĴ�������
unsigned char CollPoss;                             //�����ײ����λ�ñ���

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
//						if(frames[2] == 0x11)  //13.56M 15693Э���
//						{
//							 FindTags();
//								//???
//								//numOfTags = readBlock(frames[3]);  
//						}
//						else if(frames[2] == 0x12) //13.56M 15693Э��д
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
* �������ƣ�main()
* ��    �ܣ���������ڡ�
* ��ڲ�������
* ���ڲ�������
* ˵    ��������Ӵ˺�����ʼ���С�
****************************************************************************************************/
void main(void)
{
//==================================================================================================
    PCA0MD   &= ~0x40;
    PCA0MD    = 0x00;   //�رտ��Ź�
	  
//==================================================================================================  
    OSCsel();  //ѡ��������
  	PORT_Init();//��ʼ�����ڶ˿�
	  SYSCLK_Init();//��ʼ��ϵͳʱ��
    UART0_Init();  //��ʼ������
    EA = 1;
	  ES0 = 1;
	
 //ʹ����ѭ��������߿���������
	
/*=================================================================================================*/
    EnableSet(); //����Ϊ���                                      
		TRFDisable(); //��ֹ������  
		delay_ms(1);  
		TRFEnable();//ʹ�ܶ�����   
		delay_ms(1);  
		IRQPinset();                //ѡ��IRQ�жϹܽ�
    IRQInit();
  
    SlaveSelectPortSet();       //c8051f340P0.3Slave Select��������
    H_SlaveSelect();            //Slave Select��ֹ(��)
    SIMOSet();					//����SIMOΪ���
    SOMISIGNALSET();
		CLKPOUTset();				//����ʱ������Ϊ���

    TRFDisable();
		delay_ms(1);
    TRFEnable();
    delay_ms(1);
	
    InitialSettings();           //��ʼ�����ã�����MSP430ʱ��Ƶ��Ϊ6.78MHz��OOK����ģʽ
    EnableInterrupts();          //ʹ�����ж�
    
    OOKdirIN();                  //����OOK�ܽ�Ϊ��̬��״̬
    
    while(1)                                      
    { 
                                 //Ѱ�Ҹ���Э���׼��꿨Ƭ
		// FindTags(); 
	  while(!receiveOver);  //�ȴ����ݽ������ receiveOver = 1 �˳�
	  send_cstring(frames);
			analysisFrames();
    }
}

// ����UART0�ж�
//-----------------------------------------------------------
//�ж�����0x0023
//void UATR0_ISR(void)interrupt 4
//{
//	unsigned char temp;
//    //Rx��Tx�����ж�
//    //�����ж�
//    if(!TI0)
//    {
//        RI0=0 ;
//        temp=SBUF0 ;
//        Uart0_Transmit(temp);
//    }
//    //�����ж�
//    else TI0=0;
//}
////-----------------------------------------------------------
//// ����UART0����
////-----------------------------------------------------------
//void Uart0_Transmit(unsigned char tmp)
//{
//    ES0 = 0 ;		//��UART0�ж�
//    EA = 0 ;		//��ȫ���ж�
//    SBUF0 = tmp ;
//    while(TI0 == 0);
//    TI0 = 0 ;
//    ES0 = 1 ;		//��UART0�ж�
//    EA = 1 ;		//��ȫ���ж�
//   
//}



