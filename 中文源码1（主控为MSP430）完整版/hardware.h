/**********************************************************************************************************************************************************
* �� �� ����HARDWARE.H
* ��    �ܣ�RFID�Ķ���TRF7960��MSP430F2370΢������֮���Ӳ������ͷ�ļ���
* Ӳ�����ӣ�MSP430F2370��TRF7960��Ӳ�����ӹ�ϵ������ʾ��
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
* ��    �ߣ�EMDOOR
* ��    �ڣ�2011��04��13��
**********************************************************************************************************************************************************/
#ifndef HARDWARE_H
#define HARDWARE_H


#include <MSP430x23x0.h>            
#include <stdio.h>
#include "globals.h"

/*
#ifndef HIGHSPEED                                       //���崮��SPI����ģʽ
#define HIGHSPEED
#endif
*/

/* MSP430΢���������Ӷ��� */
/*====================================================================================================*/
/* TRF7960 ����I/O�ܽ����� - MSP430F2370�ܽ�PORT 4*/
/*  DATA[0..7]   -       PORT 4 */
/*-----------------------------------------------------------------------------*/
#define TRFWrite(x)         P4OUT = x                   //8λ�ܽ�д��������
#define TRFWriteBit(x)      P4OUT |= x                  //����λд����
#define TRFRead(x)          x = P4IN                    //������

#define TRFDirIN()          P4DIR = 0x00                //�ܽ�����
#define TRFDirOUT()         P4DIR = 0xFF                //�ܽ����
#define TRFFunc()           P4SEL = 0x00                //�ܽŹ���ѡ��
/*-----------------------------------------------------------------------------*/

/* TRF7960ʹ�ܹܽ����� - MSP430F2370�ܽ�P1.0 */
/* EN           -       P1.0 */
/*-----------------------------------------------------------------------------*/
#define EnableSet()         P1DIR |= BIT0               //���÷���Ϊ���
#define TRFEnable()         P1OUT |= BIT0               //����ߵ�ƽ
#define TRFDisable()        P1OUT &= ~BIT0              //����͵�ƽ
/*-----------------------------------------------------------------------------*/

/* TRF7960ʱ��CLK�ܽ����� - MSP430F2370�ܽ�P3.3 */
/* CLK          -       P3.3 */
/*-----------------------------------------------------------------------------*/
#define CLKGPIOset()        P3SEL &= ~BIT3              //����ΪGPIO����
#define CLKFUNset()         P3SEL |= BIT3               //����Ϊ�����ܺ���
#define CLKPOUTset()        P3DIR |= BIT3               //���÷���Ϊ���
#define CLKON()             P3OUT |= BIT3               //����ߵ�ƽ
#define CLKOFF()            P3OUT &= ~BIT3              //����͵�ƽ
/*-----------------------------------------------------------------------------*/

/* TRF7960����SPIMODE�ܽ����� - MSP430F2370�ܽ�P2.3 */
/* SPIMODE      -       P2.3 */
/*-----------------------------------------------------------------------------*/
#define SPIMODE             P2IN & BIT3             //VCC-SPI;GND-PAR


/* TRF7960����SlaveSelect�ܽ����� - MSP430F2370�ܽ�P3.0 */
/* SlaveSelect  -       P3.0 */
/*-----------------------------------------------------------------------------*/
#define SlaveSelectPin          BIT0                        
#define SlaveSelectPortSet()    P3DIR |= SlaveSelectPin     //���÷���Ϊ���
#define H_SlaveSelect()         P3OUT |= SlaveSelectPin     //����ߵ�ƽ
#define L_SlaveSelect()         P3OUT &= ~SlaveSelectPin    //����͵�ƽ
/*-----------------------------------------------------------------------------*/

/* TRF7960����SIMO�ܽ����� - MSP430F2370�ܽ�P3.1 */
/* SIMO         -       P3.1 */
/*-----------------------------------------------------------------------------*/
#define SIMOSet()           P3DIR |= BIT1               //���÷���Ϊ���
#define SIMOON()            P3OUT |= BIT1               //����ߵ�ƽ
#define SIMOOFF()           P3OUT &= ~BIT1              //����͵�ƽ
/*-----------------------------------------------------------------------------*/

/* TRF7960����SOMI�ܽ����� - MSP430F2370�ܽ�P3.2 */
/* SOMI         -       P3.2 */
/*-----------------------------------------------------------------------------*/
#define SOMISIGNAL          P3IN & BIT2
/*-----------------------------------------------------------------------------*/

/* TRF7960�ж�IRQ�ܽ����� - MSP430F2370�ܽ�P2.1 */
/* IRQ          -       P2.1 */
/*-----------------------------------------------------------------------------*/
#define IRQPin              BIT1                        //��1��
#define IRQPORT             P2IN                        //�˿�2
#define IRQPinset()         P2DIR &= ~IRQPin            //���÷���Ϊ����
#define IRQON()             P2IE |= IRQPin              //IRQ �жϿ���
#define IRQOFF()            P2IE &= ~IRQPin             //IRQ �жϹر�
#define IRQEDGEset()        P2IES &= ~IRQPin            //�������ж�
#define IRQCLR()            P2IFG = 0x00                //���жϱ�־λ
#define IRQREQreg()         P2IFG                       //�жϼĴ���                    
/*-----------------------------------------------------------------------------*/

/* TRF7960Э��ָʾ�ƹܽ����� - MSP430F2370�ܽ� */
/* Tag-it       -       P1.1 */
/* ISO14443B    -       P1.2 */
/* ISO14443A    -       P1.3 */
/* ISO15693     -       P1.4 */
/* Beep         -       P1.5 */
/*-----------------------------------------------------------------------------*/
#define Port1Set()          P1DIR |= 0xFF               //���÷���Ϊ���
#define LEDallOFF()         P1OUT &= ~0x1E              //�ر�����LED��
#define LEDallON()          P1OUT |= 0x1E               //��������LED��

#define LEDtagitON()        P1OUT |= BIT1               //Tag-itָʾ����
#define LEDtagitOFF()       P1OUT &= ~BIT1              //Tag-itָʾ����
#define LEDtypeBON()        P1OUT |= BIT2               //ISO14443Bָʾ����
#define LEDtypeBOFF()       P1OUT &= ~BIT2              //ISO14443Bָʾ����
#define LEDtypeAON()        P1OUT |= BIT3               //ISO14443Aָʾ����
#define LEDtypeAOFF()       P1OUT &= ~BIT3              //ISO14443Aָʾ����
#define LED15693ON()        P1OUT |= BIT4               //ISO15693ʾ����
#define LED15693OFF()       P1OUT &= ~BIT4              //ISO15693ʾ����

#define BeepON()            P1OUT |= BIT5//P1OUT &= ~BIT5              //��������
#define BeepOFF()           P1OUT &= ~BIT5//P1OUT |= BIT5               //��������
/*-----------------------------------------------------------------------------*/

/* LCMͼ�ε�����ʾ�� - MSP430F2370�ܽ����� */
/* LCM_SCL      -       P1.6 */
/* LCM_SID      -       P1.7 */
/* LCM_A0       -       P3.6 */
/* LCM_nCS      -       P2.4 */
/* LCM_nRST     -       P2.5 */
/*-----------------------------------------------------------------------------*/
#define H_LCM_SCL()         P1OUT |= BIT6               //����ߵ�ƽ
#define L_LCM_SCL()         P1OUT &= ~BIT6              //����͵�ƽ
#define H_LCM_SID()         P1OUT |= BIT7               //����ߵ�ƽ
#define L_LCM_SID()         P1OUT &= ~BIT7              //����͵�ƽ

#define LCMA0Set()          P3DIR |= BIT6               //���÷���Ϊ���
#define H_LCM_A0()          P3OUT |= BIT6               //����ߵ�ƽ
#define L_LCM_A0()          P3OUT &= ~BIT6              //����͵�ƽ

#define LCMnCSSet()         P2DIR |= BIT4               //���÷���Ϊ���
#define H_LCM_nCS()         P2OUT |= BIT4               //����ߵ�ƽ
#define L_LCM_nCS()         P2OUT &= ~BIT4              //����͵�ƽ

#define LCMnRSTSet()        P2DIR |= BIT5               //���÷���Ϊ���
#define H_LCM_nRST()        P2OUT |= BIT5               //����ߵ�ƽ
#define L_LCM_nRST()        P2OUT &= ~BIT5              //����͵�ƽ
/*-----------------------------------------------------------------------------*/

/* TRF7960 OOK�ܽ����� - MSP430F2370�ܽ�P2.2 */
/* OOK          -       P2.2 */
/*-----------------------------------------------------------------------------*/
#define OOKPin              BIT2                        
#define OOKdirIN()          P2DIR &= ~OOKPin            //���÷���Ϊ����     
#define OOKdirOUT()         P2DIR |= OOKPin             //���÷���Ϊ���
#define OOKoff()            P2OUT &= ~OOKPin            //OOK�ر�
#define OOKon()             P2OUT |= OOKPin             //OOK����
/*====================================================================================================*/


/* ������ʱ���������� */
/*====================================================================================================*/
#define CountValue          TACCR0                      //�������Ĵ��� TACCR0
#define StartCounter        TACTL |=  MC1               //��ʼ�Ե���ģʽ����
#define StopCounter()       TACTL &= ~(MC0 + MC1)       //ֹͣ����
/*====================================================================================================*/


/* �������ģʽģʽ */
/*====================================================================================================*/
#ifndef HIGHSPEED
    #define count1ms            847
#else
    #define count1ms            1694
#endif
/*====================================================================================================*/


/* TRF7960оƬ��ַ���� */
/*====================================================================================================*/
#define ChipStateControl        0x00                        //оƬ״̬���ƼĴ���
#define ISOControl              0x01                        //ISOЭ����ƼĴ���
#define ISO14443Boptions        0x02                        //ISO14443BЭ���ѡ�Ĵ���
#define ISO14443Aoptions        0x03                        //ISO14443AЭ���ѡ�Ĵ���
#define TXtimerEPChigh          0x04                        //TX���͸��ֽڼĴ���
#define TXtimerEPClow           0x05                        //TX���͵��ֽڼĴ���
#define TXPulseLenghtControl    0x06                        //TX�������峤�ȿ��ƼĴ���
#define RXNoResponseWaitTime    0x07                        //RX������Ӧ��ȴ�ʱ��Ĵ���
#define RXWaitTime              0x08                        //RX���յȴ�ʱ��Ĵ���
#define ModulatorControl        0x09                        //��������ϵͳʱ�ӼĴ���
#define RXSpecialSettings       0x0A                        //RX�����������üĴ���
#define RegulatorControl        0x0B                        //��ѹ������I/O���ƼĴ���
#define IRQStatus               0x0C                        //IRQ�ж�״̬�Ĵ���
#define IRQMask                 0x0D                        //��ײλ�ü��жϱ�ǼĴ���
#define CollisionPosition       0x0E                        //��ײλ�üĴ���
#define RSSILevels              0x0F                        //�źŽ���ǿ�ȼĴ���
#define RAMStartAddress         0x10                        //RAM ��ʼ��ַ����7�ֽڳ� (0x10 - 0x16)
#define NFCID                   0x17                        //��Ƭ��ʶ��
#define NFCTargetLevel          0x18                        //Ŀ�꿨ƬRSSI�ȼ��Ĵ���
#define NFCTargetProtocol       0x19                        //Ŀ�꿨ƬЭ��Ĵ���
#define TestSetting1            0x1A                        //�������üĴ���1
#define TestSetting2            0x1B                        //�������üĴ���2
#define FIFOStatus              0x1C                        //FIFO״̬�Ĵ���
#define TXLenghtByte1           0x1D                        //TX���ͳ����ֽ�1�Ĵ���
#define TXLenghtByte2           0x1E                        //TX���ͳ����ֽ�2�Ĵ���
#define FIFO                    0x1F                        //FIFO���ݼĴ���


/* TRF7960оƬ����� */
/*====================================================================================================*/
#define Idle                    0x00                        //��������
#define SoftInit                0x03                        //�����ʼ������
#define InitialRFCollision      0x04                        //��ʼ��RF��ײ����
#define ResponseRFCollisionN    0x05                        //Ӧ��RF��ײN����
#define ResponseRFCollision0    0x06                        //Ӧ��RF��ײ0����
#define Reset                   0x0F                        //��λ����
#define TransmitNoCRC           0x10                        //������CRCУ������
#define TransmitCRC             0x11                        //���ʹ�CRCУ������
#define DelayTransmitNoCRC  	0x12                        //��ʱ������CRCУ������
#define DelayTransmitCRC    	0x13                        //��ʱ���ʹ�CRCУ������
#define TransmitNextSlot        0x14                        //������һ��������
#define CloseSlotSequence       0x15                        //�رղ���������
#define StopDecoders            0x16                        //ֹͣ��a����
#define RunDecoders             0x17                        //���н�a����
#define ChectInternalRF         0x18                        //����ڲ�RF��������
#define CheckExternalRF         0x19                        //����ⲿRF��������
#define AdjustGain              0x1A                        //�����������
/*====================================================================================================*/

/******************************************************************************************************************
* �������ƣ�delay_ms()
* ��    �ܣ���ʱ������
* ��ڲ�����n_ms        ����ֵ
* ���ڲ�������
* ˵    �����ú���Ϊ��ʱ��������ڲ���ֵԽ��CPU��ʱ�ȴ���ʱ���Խ����
*           ����ʾ���򣬽���Ӧ���ڷǾ�ȷ����ʱ�ȴ����ϡ�
*           ע�⣺��ڲ��������ֵΪ 65536��           
*******************************************************************************************************************/
void delay_ms(unsigned int n_ms);

/******************************************************************************************************************
* �������ƣ�CounterSet()
* ��    �ܣ���ʱ�����ú�����
* ��ڲ�������
* ���ڲ�������
* ˵    ������ʼ�����ö�ʱ���Ĵ����� ����ʾ���򣬽���Ӧ���ھ�ȷ����ʱ�ȴ����ϡ�          
*******************************************************************************************************************/
void CounterSet(void);

/******************************************************************************************************************
* �������ƣ�OSCsel()
* ��    �ܣ���������ѡ��
* ��ڲ�����mode        ѡ���ڲ������ⲿģʽ
* ���ڲ�������
* ˵    �������þ�������ģʽ������ڲ���Ϊ0x00����ѡ���ⲿ����ģʽ��
*           ��Ϊ��0x00������ֵ����ѡ���ڲ�DCOģʽ��
*******************************************************************************************************************/
void OSCsel(unsigned char mode);

/******************************************************************************************************************
* �������ƣ�TimerAhandler()
* ��    �ܣ���ʱ���жϷ������
* ��ڲ�������
* ���ڲ�������
* ˵    ������ʱ���жϷ���������ھ�ȷ��ʱ�á�
*******************************************************************************************************************/
#pragma vector=TIMERA0_VECTOR
__interrupt void TimerAhandler(void);

/******************************************************************************************************************
* �������ƣ�Beep_Waring()
* ��    �ܣ���������������������
* ��ڲ�����n       ��������
*           t       ����Ƶ��
* ���ڲ�������
* ˵    ������������������������ڲ���t�Ĳ�ͬ����������ͬƵ�ʵ�������
*           ����nִֵ�з���������    
*******************************************************************************************************************/
void Beep_Waring(unsigned char n, unsigned char t);


#endif

