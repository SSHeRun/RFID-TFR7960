/**********************************************************************************************************************************************************
* �� �� ����TIRIS.C
* ��    �ܣ�TI��˾Tag-itЭ���д��������������
* 
* ��    �ߣ�EMDOOR
* ��    �ڣ�2011��04��13��
**********************************************************************************************************************************************************/
#include "tiris.h"


/******************************************************************************************************************
* �������ƣ�TIInventoryRequest()
* ��    �ܣ���������ڡ�
* ��ڲ�����*mask       ����ֵ
*           length      ���ݳ���
* ���ڲ�������
* ˵    ����Tag-it������������
*******************************************************************************************************************/
void TIInventoryRequest(unsigned char *mask, unsigned char length)                                                      	/* 010800030414(req.packet)[00ff] */
{
    unsigned char i = 1, j = 3, command, found = 0;
    unsigned char *PslotNo, slotNo[17];
    unsigned char NewMask[8], Newlength, masksize;
    int size;

    
    buf[0] = RXNoResponseWaitTime;                  // ���¶��������Ӧ��ȴ�ʱ��
    buf[1] = 0x14;
    buf[2] = ModulatorControl;                      // ���ƺ�ϵͳʱ�ӿ��ƣ�0x21 - 6.78MHz OOK(100%)
    buf[3] = 0x21;
    WriteSingle(buf, 4);

    slotNo[0] = 0x00;
    EnableSlotCounter();                            //ʹ�ܲۼ�����
    PslotNo = &slotNo[0];   	

    masksize = (((length >> 2) + 1) >> 1);
    size = masksize + 3;                            //��־ֵ+��־����+������+��־λ

    buf[0] = 0x8f;
    buf[1] = 0x91;                                  //���ʹ�CRCУ������
    buf[2] = 0x3d;                                  //д����ģʽ��0x1D/* write continous from 1D */
    buf[3] = (char) (size >> 4);
    buf[4] = (char) (size << 4);
    buf[5] = 0x00;
    buf[6] = 0x50;                                  //TIЭ���� SID��ײ����ֵ

    buf[7] = (length | 0x80);                       //��ǳ��� masklenght�������Ϣ���1
    if(length > 0)
    {
        for(i = 0; i < masksize; i++) 
        buf[i + 8] = *(mask + i);
    }   /* if */

    if(length & 0x04)                               //����в������ֽ�
    {
        buf[4] = (buf[4] | 0x09);                   //4λ
        buf[masksize + 7] = (buf[masksize + 7] << 4);       //�ƶ�4λ�������ЧλMSB
    }

    CounterSet();                                   //���ö�ʱ��
    CountValue = count1ms * 20;                     //��ʱʱ��Ϊ 20ms
    i_reg = 0x01;
    IRQCLR();                                       //���жϱ�־λ
    RAWwrite(&buf[0], masksize + 8);                //д��FIFO
    IRQON();                                        //IRQ�жϿ���
    StartCounter;                                   //��ʼ�Ե���ģʽ��ʱ
    LPM0;                                           //�ȴ�TX�жϽ���

    for(i = 1; i < 17; i++)                         //16 ��ģʽ
    {
        RXTXstate = 1;                              //���շ���״̬�Ĵ�������ֵ�������λ�洢��buf[1]��ʼλ��
        
        i_reg = 0x01;
        j = 0;
        while((i_reg == 0x01) && (j < 2))           //�ȴ�RX�����ж���
        {   				
            j++;
            CounterSet();                           //���ö�ʱ��
            CountValue = count1ms * 20;             //��ʱʱ��Ϊ 20ms
            StartCounter;                           //��ʼ�Ե���ģʽ��ʱ
            LPM0;
        }
    
        if(i_reg == 0xFF)                           //����ֽ��Ѿ�������ֽڣ�����SID���ݵ���������
        {                                   
            if(POLLING)
            {
                found = 1;                          //�ҵ�����־λ
            }
            else
            {
                kputchar('[');
                for(j = 1; j < 11; j++) Put_byte(buf[j]);
                kputchar(']');
            }
        }
        else if(i_reg == 0x02)                      //��ײ����
        {
            if(!POLLING)
            {
                PslotNo++;                          //���ײλ���ֵ�ݼ�1
                *PslotNo = i;
            }
        }
        else if(i_reg == 0x00)                      //��ʱ
        {   				
            if(!POLLING)
            {
                kputchar('[');
                kputchar(']');
            }
        }
        else
            ;
        
        command = Reset;                            //�ڽ�����һ����֮ǰ��ʹ��ֱ�����λFIFO
        DirectCommand(&command);

        if(i < 16)                                  //���δѭ��16���ۣ�����Ҫ����EOF����(�¸���)
        {   				
            command = TransmitNextSlot;
            DirectCommand(&command);
        }
        if(!POLLING)
        {
            put_crlf();   
        }
    }   /* for */

    DisableSlotCounter();                           //��ֹ�ۼ�����

    if(found)                                       //����ҵ���Ƭ����LED��ӦЭ��ָʾ����
    {
        LEDtagitON();
        
    }
    else
    {
        LEDtagitOFF();                              //���δ�ҵ���Ƭ����LEDϨ�𡢷�����������
       
    }

    Newlength = length + 4;                         //��ǳ���Ϊ4����λ����
    masksize = (((Newlength >> 2) + 1) >> 1) - 1;

    while(*PslotNo != 0x00)
    {
        *PslotNo = *PslotNo - 1;                    //�ۼ�������1��16����

        for(i = 0; i < 4; i++) 
            NewMask[i] = *(mask + i);               //���Ƚ����ֵ�������±��������

        if((Newlength & BIT2) == 0x00) 
            *PslotNo = *PslotNo << 4;

        NewMask[masksize] |= *PslotNo;              //���ֵ���ı�/* the mask is changed */
        TIInventoryRequest(&NewMask[0], Newlength); //�ݹ���� TIInventoryRequest ����

        PslotNo--;                                  //�۵ݼ�
    }   /* while */

    IRQOFF();                                       //�³�ײ���̽������ر��ж�
}   /* TIInventoryRequest */
