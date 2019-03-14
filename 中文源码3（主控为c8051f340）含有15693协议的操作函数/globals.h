/************************************************************************************************************************
* �� �� ����GLOBALS.H
* ��    �ܣ�����ȫ�ֱ������������塣
*           ��Щȫ�ֱ������������������е�ͷ�ļ���C�ļ��С�
* ��    ����V1.0
* ��    �ߣ�EMDOOR
* ��    �ڣ�2011-9-29
***************************************************************************************************************************/
#ifndef GLOBALS_H
#define GLOBALS_H                 
#include <c8051f340.h>

#define BUF_LENGTH                      300                 //���֡�ֽ�����
#define EnableInterrupts()              IE |= 0X80          //ʹ���ж�

extern char rxdata;                                         //RS232 ���������ֽ�
extern xdata unsigned char buf[BUF_LENGTH];                       //�ⲿ����MSP430΢��������TRF7960ͨ�Ž��ջ����� 
extern signed char RXTXstate;                               //�ⲿ�������ͽ����ֽڼĴ�������
extern unsigned char flags;                                 //�ⲿ�洢�洢��־λ(�ڷ³�ײ��ʹ��)
extern unsigned char RXErrorFlag;                           //�ⲿ�������մ����־
extern unsigned char RXflag;                                //����������־λ������ָʾ���������Ƿ�������
extern unsigned char i_reg;                                 //�ⲿ�����жϼĴ�������
extern unsigned char CollPoss;                              //�ⲿ������ײ����λ�ñ���
extern unsigned char RXdone;                                //�ⲿ���������������ݱ�־λ����������ɣ��øñ�־λ1
extern unsigned char Found_tag;                             //�����Ƿ��⵽��Ƭȫ�ֱ���
extern unsigned char rssival;                               //�����⵽�Ŀ�Ƭ�����ź�ǿ��ֵ

#endif


