/****************************************************************************************************************
* �� �� ����AUTOMATIC.H
* ��    �ܣ�����Ķ����Ķ���Χ�ڵ����о�꿨ƬAUTOMATIC.Cͷ�ļ���
*
* ��    �ߣ�EMDOOR
* ��    �ڣ�2011-9-29
******************************************************************************************************************/
#ifndef AUTOMATIC_H
#define AUTOMATIC_H


#include <C8051F340.h> 	
#include <stdio.h>
#include <hardware.h>
#include <globals.h>
#include <host.h>
#include <communicate.h>
#include <anticollision.h>
#include <ISO14443.h>


/******************************************************************************************************************
* �������ƣ�FindTags()
* ��    �ܣ�����ָ�����Э�����ͣ�����TRF7960���ø���ؼĴ����󣬽���Ѱ��������
* ��ڲ�����protocol       ָ��Э������
* ���ڲ�������
* ˵    �����ú�����һ����ѭ�����������е��ѻ���ʾִ�й��̾��ڴ���ɡ�
*******************************************************************************************************************/
void FindTags(void);

void Set_Pro();
void Write_AFI_Command(unsigned char datas);
void Write_DSFID_Command(unsigned char datas);
void Get_Info_Command();
void Get_sec_Command(unsigned char block,unsigned char datas);
void Write_Block_Command(unsigned char block,unsigned char datas[2]);
void Read_Block_Command(unsigned char block);


#endif
