/**********************************************************************************************************************************************************
* �� �� ����GLOBALS.H
* ��    �ܣ�����ȫ�ֱ������������塣
*           ��Щȫ�ֱ������������������е�ͷ�ļ���C�ļ��С�
* ��    ����V1.0
* ��    �ߣ�POWER
* ��    �ڣ�2009��05��18��
**********************************************************************************************************************************************************/
#ifndef GLOBALS_H
#define GLOBALS_H


#define Beep15693 	                20                  //��������������Ƶ��
#define BeeptypeA   	                80
#define BeeptypeB  	                160
#define Tagit                           250

#define BUF_LENGTH                      300                 //���֡�ֽ�����
#define EnableInterrupts()              _EINT()             //ʹ���ж�

extern char rxdata;                                         //RS232 ���������ֽ�
extern unsigned char buf[BUF_LENGTH];                       //�ⲿ����MSP430΢��������TRF7960ͨ�Ž��ջ�����
extern signed char RXTXstate;                               //�ⲿ�������ͽ����ֽڼĴ�������
extern unsigned char flags;                                 //�ⲿ�洢�洢��־λ(�ڷ³�ײ��ʹ��)
extern unsigned char RXErrorFlag;                           //�ⲿ�������մ����־
extern unsigned char RXflag;                                //����������־λ������ָʾ���������Ƿ�������
extern unsigned char i_reg;                                 //�ⲿ�����жϼĴ�������
extern unsigned char CollPoss;                              //�ⲿ������ײ����λ�ñ���

extern unsigned char RXdone;                                //�ⲿ���������������ݱ�־λ����������ɣ��øñ�־λ1
extern unsigned char ENABLE;                                //�ⲿ����TRF7960ʹ�ܱ���
extern unsigned char LCDcon;                                //LCD������־λ
extern unsigned char POLLING;                               //�����ѻ�����������־λ:0-Ϊ����USBģʽ;1-Ϊ�ѻ�LCDģʽ
extern unsigned char Found_tag;                             //�����Ƿ��⵽��Ƭȫ�ֱ���
extern unsigned char test_no;                               //����RFID��Ƭ������Ŀ���к�
extern unsigned char rssival;                               //�����⵽�Ŀ�Ƭ�����ź�ǿ��ֵ


#endif


