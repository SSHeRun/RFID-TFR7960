/**********************************************************************************************************************************************************
* �� �� ����LCD.H
* ��    �ܣ�HO12864FPD-14CSBE��3S��λͼ��Һ��ģ�飨ST7565P������ͷ�ļ���
* Ӳ�����ӣ�HO12864FPD-14CSBE��3S��λͼ��Һ��ģ�飨ST7565P����MSP430F2370��Ӳ�����ӹ�ϵ������ʾ��
*           HO12864FPD-14CSBE(3S)               MSP430F2370
*
*               SCL(PIN7)                         P1.6
*               SID(PIN8)                         P1.7
*               A0(PIN14)                         P3.6
*               CSn(PIN12)                        P2.4
*               RESETn(PIN13)                     P2.5
*               VDD(PIN9)
*               VSS(PIN10)
*               LED+(PIN11)
* ��    �ߣ�EMDOOR
* ��    �ڣ�2011��04��13��
**********************************************************************************************************************************************************/
#ifndef LCD_H
#define LCD_H


#include <MSP430x23x0.h>     	
#include <stdlib.h>		
#include <stdio.h>
#include "hardware.h"
#include "communicate.h"
#include "anticollision.h"
#include "globals.h"
#include "tiris.h"
#include "ISO14443.h"
#include "host.h"

/* ����������ģ���ݶ���Ϊ�ⲿ���� */
/*====================================================================================================*/
extern unsigned char const logo[1024];
extern unsigned char const emdoor[1024];
extern unsigned char const usb[1024];

extern unsigned char const Num[256];
extern unsigned char const CG[16];
extern unsigned char const CH[16];
extern unsigned char const CI[16];
extern unsigned char const CJ[16];
extern unsigned char const CK[16];
extern unsigned char const CL[16];
extern unsigned char const CM[16];
extern unsigned char const CN[16];
extern unsigned char const CO[16];
extern unsigned char const CP[16];
extern unsigned char const CQ[16];
extern unsigned char const CR[16];
extern unsigned char const CS[16];
extern unsigned char const CT[16];
extern unsigned char const CU[16];
extern unsigned char const CV[16];
extern unsigned char const CW[16];
extern unsigned char const CX[16];
extern unsigned char const CY[16];
extern unsigned char const CZ[16];
extern unsigned char const Cz[16];

extern unsigned char const pro[16];
extern unsigned char const ZK[16];
extern unsigned char const YK[16];
extern unsigned char const ant[16];
extern unsigned char const ant1[16];
extern unsigned char const ant2[16];
extern unsigned char const ant3[16];
extern unsigned char const lkuo[16];
extern unsigned char const rkuo[16];

extern unsigned char const huan[32];
extern unsigned char const ying[32];
extern unsigned char const shi[32];
extern unsigned char const yong[32];
extern unsigned char const jiao[32];
extern unsigned char const xue[32];
extern unsigned char const ping[32];
extern unsigned char const tai2[32];
extern unsigned char const xi[32];
extern unsigned char const tong[32];

extern unsigned char const xie[32];
extern unsigned char const du[32];
extern unsigned char const yi[32];
extern unsigned char const ge[32];
extern unsigned char const kuai[32];
extern unsigned char const xin[32];
extern unsigned char const xi2[32];
extern unsigned char const an[32];
extern unsigned char const quan[32];
extern unsigned char const zhuang[32];
extern unsigned char const tai[32];

extern unsigned char const yi[32];
extern unsigned char const dao[32];
extern unsigned char const dian[32];
extern unsigned char const zi2[32];
extern unsigned char const lian[32];
extern unsigned char const ji[32];
extern unsigned char const mo[32];
extern unsigned char const shi2[32];
extern unsigned char const tong2[32];

/******************************************************************************************************************
* �������ƣ�Delay_lcm()
* ��    �ܣ������ʱ
* ��ڲ�����count    ��ʱ������ֵԽ����ʱԽ��
* ���ڲ�������
* ˵    ������ʱ�ȴ��á�
******************************************************************************************************************/
void Delay_lcm(unsigned int count);

/******************************************************************************************************************
* �������ƣ�Data_Send()
* ��    �ܣ�LCD����ģʽ��������
* ��ڲ�����data    Ҫ���͵�����
* ���ڲ�������
* ˵    ����MSP430΢��������LCD��������������
******************************************************************************************************************/
void Data_Send(unsigned char data);

/******************************************************************************************************************
* �������ƣ�Write_CMD()
* ��    �ܣ�LCD����ģʽ��������
* ��ڲ�����command    ����ֵ
* ���ڲ�������
* ˵    ����MSP430΢��������LCD��������������
******************************************************************************************************************/
void Write_CMD(unsigned char command);

/******************************************************************************************************************
* �������ƣ�Write_Data()
* ��    �ܣ�д����
* ��ڲ�����data    ����
* ���ڲ�������
* ˵    ����MSP430΢��������LCD��������������
******************************************************************************************************************/
void Write_Data(unsigned char data);

/******************************************************************************************************************
* �������ƣ�Display_Clear()
* ��    �ܣ����LCD��ʾ
* ��ڲ�������
* ���ڲ�������
* ˵    ����ִ�иú��������������Ļ�κ���ʾ����
******************************************************************************************************************/
void Display_Clear(void);

/******************************************************************************************************************
* �������ƣ�Display_Picture()
* ��    �ܣ���ʾһ��128*64����ͼƬ
* ��ڲ�����*p       ͼƬ���ݵ��׵�ַָ��
* ���ڲ�������
* ˵    ����ִ�иú�����������ָ������ʾ��ͼƬ�׵�ַ��
******************************************************************************************************************/
void Display_Picture(unsigned char *p);

/******************************************************************************************************************
* �������ƣ�Display_Chinese()
* ��    �ܣ���ָ����λ���ϣ���ʾһ������
* ��ڲ�����pag       ��ʾ���ֵ�ҳ��ַ
*           col       ��ʾ���ֵ��е�ַ
*           *hzk      ��ʾ���ֵĵ�ַָ��
* ���ڲ�������
* ˵    �������ú����������ɹ�����������������������
******************************************************************************************************************/
void Display_Chinese(unsigned char pag,unsigned char col, unsigned char const *hzk);

/******************************************************************************************************************
* �������ƣ�Display_Char()
* ��    �ܣ���ָ����λ���ϣ���ʾһ�����ֻ�����ĸ
* ��ڲ�����pag       ��ʾ���ֵ�ҳ��ַ
*           col       ��ʾ���ֵ��е�ַ
*           *hzk      ��ʾ���ֵĵ�ַָ��
* ���ڲ�������
* ˵    �������ú����������ɹ�����������������������
******************************************************************************************************************/
void Display_Char(unsigned char pag,unsigned char col, unsigned char const *hzk);

/******************************************************************************************************************
* �������ƣ�LCM12864_Init()
* ��    �ܣ�LCM12864ģ���ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵    ������ʼ��LCD��ʾ��
******************************************************************************************************************/
void LCM12864_Init(void);

/******************************************************************************************************************
* �������ƣ�Display_Put_Tag()
* ��    �ܣ�LCM12864ģ����ʾ����뿨Ƭ��Ϣ
* ��ڲ�������
* ���ڲ�������
* ˵    ������ʾ��ʾ��Ϣ
******************************************************************************************************************/
void Display_Put_Tag(void);

/******************************************************************************************************************
* �������ƣ�Display_Start()
* ��    �ܣ�LCM12864ģ����ʾϵͳ������Ϣ
* ��ڲ�������
* ���ڲ�������
* ˵    ������ʾ������Ϣ
******************************************************************************************************************/
void Display_Start(void);

/******************************************************************************************************************
* �������ƣ�Display_Connect()
* ��    �ܣ�����ģʽ��LCM12864ģ����ʾϵͳ��Ϣ
* ��ڲ�������
* ���ڲ�������
* ˵    ������ʾ����ģʽ��Ϣ
******************************************************************************************************************/
void Display_Connect(void);

/******************************************************************************************************************
* �������ƣ�Display_find_tag()
* ��    �ܣ�LCM12864ģ����ʾϵͳ������Ϣ
* ��ڲ�����pro     ISOЭ����趨ֵ
*                   0ΪISO15693��1ΪISO14443A��2ΪISO14443B
* ���ڲ�������
* ˵    ������ʾ������Ϣ
******************************************************************************************************************/
void Display_find_tag(unsigned char pro);

/******************************************************************************************************************
* �������ƣ�Display_Rssi()
* ��    �ܣ�LCM12864ģ����ʾ��⵽�ľ�꿨Ƭ�ź�ǿ��ͼ��
* ��ڲ�����M_rssi    ���ŵ��ź�ǿ��
* ���ڲ�������
* ˵    ������ʾ������Ϣ
******************************************************************************************************************/
void Display_Rssi(unsigned char M_rssi);

/******************************************************************************************************************
* �������ƣ�Display_pro()
* ��    �ܣ�LCM12864ģ����ʾ��д����ʱ������
* ��ڲ�������
* ���ڲ�������
* ˵    ������ʾ������Ϣ
******************************************************************************************************************/
void Display_pro(void);


#endif

