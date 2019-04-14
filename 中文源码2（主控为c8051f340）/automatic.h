/****************************************************************************************************************
* 文 件 名：AUTOMATIC.H
* 功    能：侦测阅读器阅读范围内的所有卷标卡片AUTOMATIC.C头文件。
*
* 作    者：EMDOOR
* 日    期：2011-9-29
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
* 函数名称：FindTags()
* 功    能：根据指定卷标协议类型，设置TRF7960配置各相关寄存器后，进行寻卡操作。
* 入口参数：protocol       指定协议类型
* 出口参数：无
* 说    明：该函数是一个死循环函数，所有的脱机演示执行过程均在此完成。
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
