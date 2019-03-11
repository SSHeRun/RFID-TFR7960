/**********************************************************************************************************************************************************
* �� �� ����MAIN.C
* ��    �ܣ�RFID�Ķ���TRF7960ϵͳ�������������ʾ����
* Ӳ�����ӣ�RFID�Ķ���ϵͳ��Ӳ����ͼ������ʾ��
*                                   ����������������  
*                                  |               |
*                                  |      LCD      |
*                                  |               |
*                                   ����������������
*                                         ||   
*    ����������������    (SPI)      ����������������               �������������� 
*   |               | <----------> |               |     USB      |             |
*   |    TRF7960    |              |  MSP430F2370  | <----------> |     PC      |
*   |               | <----------> |               |              |             |
*    ����������������    (PAR)      ����������������               �������������� 
*
* ˵    ���� TRF7960��MSP430΢������֮���ͨ�ŷ�ʽΪ����SPI���߲���PARģʽ����ͨ��ģʽ��JP2����ñѡ��              
*                       
* ��    �ߣ�EMDOOR
* ��    �ڣ�2011��04��13��
**********************************************************************************************************************************************************/
#include <MSP430x23x0.h>                            //΢������ͷ�ļ�����
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


#define DBG     1                                   //��ӡ��Ϣ�궨��

char rxdata;                                        //RS232 ���������ֽ�
unsigned char buf[BUF_LENGTH];                      //����MSP430΢��������TRF7960ͨ�Ž��ջ�����
signed   char RXTXstate;                            //���巢�ͽ����ֽڼĴ�������
unsigned char flags;                                //����洢��־λ(�ڷ³�ײ��ʹ��)
unsigned char RXErrorFlag;                          //������մ����־
unsigned char RXflag;                               //����������־λ������ָʾ���������Ƿ�������
unsigned char i_reg;                                //�жϼĴ�������
unsigned char CollPoss;                             //�����ײ����λ�ñ���


/******************************************************************************************************************
* �������ƣ�main()
* ��    �ܣ���������ڡ�
* ��ڲ�������
* ���ڲ�������
* ˵    ��������Ӵ˺�����ʼ���С�
*******************************************************************************************************************/
void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;                       //�رտ��Ź�

    UARTset();                                      //��������

    /* ʹ��TRF7960оƬ */
    /*====================================================================================================*/
    EnableSet();                                    //ʹ����ѭ��������߿���������
    TRFDisable(); 
    delay_ms(1);
    TRFEnable();
    delay_ms(1);
    /*====================================================================================================*/
    
    /* ѡ��MSP430��TRF7960֮��ͨ��ģʽ */
    /*====================================================================================================*/
    if (SPIMODE)                                    //����SPIģʽ
    {                                               //Ϊ����SPIģʽ���ùܽŹ���
        EnableSet();
        IRQPinset();                                //ѡ��IRQ�жϹܽ�
        IRQEDGEset();                               //�����������ж�ģʽ

        Port1Set();                                 //MSP430 �˿�1����
        LEDallOFF();                                //����LEDЭ��ָʾ�ƾ��ر�
    }
    else                                            //����ģʽ
        PARset();                                   //Ϊ����ģʽ���ùܽŹ���
   /*====================================================================================================*/

    if (SPIMODE)                                    //����SPIģʽ������MSP430 SPI�ܽŹ��ܵ�
    {
#ifndef SPI_BITBANG
        USARTset();                                 //����USART
#else
        SlaveSelectPortSet();                       //MSP430�ܽ�P3.0 Slave Select��������
        H_SlaveSelect();                            //Slave Select��ֹ(��)
        SIMOSet();
        CLKPOUTset();
#endif
    }

    TRFDisable();
    TRFEnable();
    delay_ms(1);
    InitialSettings();                              //��ʼ�����ã�����MSP430ʱ��Ƶ��Ϊ6.78MHz��OOK����ģʽ

    OSCsel(0x00);                                   //����ѡ���ⲿ��������
    delay_ms(10);
    BeepON();
    delay_ms(10);
    BeepOFF();
    
    if (SPIMODE)                                    //��ΪSPIģʽ��ʹ���ⲿ������������USART
    {
#ifndef SPI_BITBANG
        USARTEXTCLKset();                           //����USART
#endif
    }

    EnableInterrupts();                             //ʹ���ж�
    
    /* LCMͼ����ʾģ���ʼ����ϵͳ���п���������ʾ */
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
    
    delay_ms(10);                                   //ϵͳ��ʼ����,��Э��ָʾ��ȫ��
    LEDtagitON();delay_ms(100);                     //LED����ѭ������Ϩ��
    LEDtypeBON();delay_ms(100); 
    LEDtypeAON();delay_ms(100);
    LED15693ON();delay_ms(100);
    LED15693OFF();delay_ms(100);
    LEDtypeAOFF();delay_ms(100);
    LEDtypeBOFF();delay_ms(100);
    LEDtagitOFF();delay_ms(100);

    Display_Put_Tag();                              //��ʾ��������ꡱ��ϵͳ������״̬
    
    OOKdirIN();                                     //����OOK�ܽ�Ϊ��̬��״̬
    ENABLE = 1;
    POLLING = 1;                                    //Ĭ��ΪLCD�ѻ�ģʽ
    
    while(1)                                        //�ȴ����ڽ����ж�
    {
        FindTags(0x00);                             //Ѱ�Ҹ���Э���׼��꿨Ƭ
    }
}



