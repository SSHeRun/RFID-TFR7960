/**********************************************************************************************************************************************************
* �� �� ����ISO14443.C
* ��    �ܣ�ISO14443A��ISO14443BЭ�鿨Ƭ���������������³�ײ����ȡ�
*           ���ļ�������ISO14443Э���ISO14443BЭ�鿨Ƭ����ʾ������
*           ע�⣺�ڴ���ͽ��չ����У�����Ҫͨ��FIFO������
* ��    �ߣ�EMDOOR
* ��    �ڣ�2011��04��13��
**********************************************************************************************************************************************************/
#include "ISO14443.h"
#include "lcd.h"

unsigned char completeUID[14];                      //����������ISO14443Э��UID�����

/******************************************************************************************************************
* �������ƣ�SelectCommand()
* ��    �ܣ�ѡ�������ڲ���ֵselectд�뵽TRF7960��FIFO�������С�
* ��ڲ�����select       Э�鴮������
*           *UID         Ψһ��ʶ��UID�ַ�������
* ���ڲ�����ret     
* ˵    �������ú���������ֵ0�����ʾд������ɹ���ɡ�
*******************************************************************************************************************/
char SelectCommand(unsigned char select, unsigned char *UID)
{
    unsigned char j;                                //�������
    char ret = 0;                                   //���巵��ֵ����������ֵΪ0
    
    buf[50] = ISOControl;                           //����ѡ��ISO14443A����ģʽΪ:������106kbps����ʹ��CRCУ��
    buf[51] = 0x08;
    WriteSingle(&buf[50], 2);                       //д����

    /* ��buf�Ĵ���������ֵ */
    /*====================================================================================================*/
    for(j = 0; j < 5; j++) 
    {
        buf[j + 7] = *(UID + j);
    }
    /*====================================================================================================*/
    
    buf[0] = 0x8f;                                  //���ý�Ҫд��FIFO��ֵ
    buf[1] = 0x91;          
    buf[2] = 0x3d;
    buf[3] = 0x00;
    buf[4] = 0x70;
    buf[5] = select;
    buf[6] = 0x70;

    RAWwrite(buf, 12);                              //ʹ��ֱ��д����д��12�ֽ�������������

    i_reg = 0x01;
    RXTXstate = 1;                                  //���ñ�־λ�������λ�洢��buf[1]��ʼλ��

    while(i_reg == 0x01)                            //�ȴ��жϽ������
    {
    }

    i_reg = 0x01;                                   //�ָ���־λ
    CounterSet();                                   //��ʱ����ʼ����
    CountValue = 0x2000;                            //��ʱ10ms
    StartCounter;                                   //��ʼ��ʱ

    while(i_reg == 0x01)                            //�ȴ��жϽ������
    {
    }                   	
    
    if(!POLLING)
    {
        if(i_reg == 0xFF)                           //���ܵ�Ӧ��
        {                 
            if((buf[1] & BIT2) == BIT2)             //UIDδ��������
            {           
                kputchar('(');
                for(j = 1; j < RXTXstate; j++)
                {
                    Put_byte(buf[j]);
                }/* for */

                kputchar(')');
                ret = 1;
                goto FINISH;
            }
            else                                    //UID�������
            {               
                kputchar('[');
                for(j = 1; j < RXTXstate; j++)
                {
                    Put_byte(buf[j]);
                }/* for */

                kputchar(']');
                ret = 0;
                goto FINISH;
            }
        }
        else if(i_reg == 0x02)                      //��ײ����
        {                
            kputchar('[');
            kputchar('z');                          //����[z]
            kputchar(']');
        }
        else if(i_reg == 0x00)                      //��ʱ���ж�
        {             
            kputchar('[');
            kputchar(']');
        }
        else
            ;
    }
    
FINISH:
    return(ret);                                    //����0����ʾ�ú����ɹ���ִ�С�
}   /* SelectCommand */

/******************************************************************************************************************
* �������ƣ�AnticollisionLoopA()
* ��    �ܣ�ISO14443A�³�ײѭ����
* ��ڲ�����select       Э�鴮������
*           NVB          ��Ч�ֽ�����
*           *UID         Ψһ��ʶ��UID�ַ�������
* ���ڲ�������    
* ˵    �����ú����ݹ麯��������ISO14443A��ƬUID�봮��������ͬ���ݹ���ô�����ͬ��
*******************************************************************************************************************/
void AnticollisionLoopA(unsigned char select, unsigned char NVB, unsigned char *UID)
{
    unsigned char i, lenght, newUID[4], more = 0;
    unsigned char NvBytes = 0, NvBits = 0, Xbits, found = 0;
    unsigned char temp1, temp2;

    buf[50] = ISOControl;                           //��ֹ����CRCУ��
    buf[51] = 0x88;
    WriteSingle(&buf[50], 2);                       //д������ֵ

    RXErrorFlag = 0;                                //����մ����־
    CollPoss = 0;                                   //���ײλ��

    lenght = 5 + (NVB >> 4);                        //�õ���Ч�ֽ���������
    if((NVB & 0x0f) != 0x00)
    {
        lenght++;
        NvBytes = (NVB >> 4) - 2;                   //��ȡ��Ч�ֽ�����
        Xbits = NVB & 0x07;                         //��ȡ��Чλ����
        for(i = 0; i < Xbits; i++)
        {
            NvBits = NvBits << 1;
            NvBits = NvBits + 1;                    //�ɴ˼������Чλ����
        }
    }   /* if */

    buf[0] = 0x8f;                                  //׼������ѡ�������λFIFO������
    if(NVB == 0x70)                                 //�ж���ѡ�������CRCУ��
        buf[1] = 0x91;                         
    else                                            //����Ϊ�Ƿ³�ײ����
        buf[1] = 0x90;
    
    buf[2] = 0x3d;
    buf[3] = 0x00;
    buf[4] = NVB & 0xf0;                            //�����ֽ�����
    if((NVB & 0x07) != 0x00)                        //������λ����
        buf[4] |= ((NVB & 0x07) << 1) + 1;
    buf[5] = select;                                //selectֵΪ�������ֵ����ȡ0x93,0x95����0x97
    buf[6] = NVB;                                   //��Чλ����
    buf[7] = *UID;
    buf[8] = *(UID + 1);
    buf[9] = *(UID + 2);
    buf[10] = *(UID + 3);

    RAWwrite(&buf[0], lenght);                      //ֱ��д���FIFO������,����Ϊlenth

    RXTXstate = 1;                                  //���ñ�־λ�������λ�洢��buf[1]��ʼλ��

    i_reg = 0x01;
    while(i_reg != 0x00)                            //�ȴ��������
    {
        CounterSet();
        CountValue = 0x2710;                        //��ʱ 10ms 
        StartCounter;                               //�����ϼ���ģʽ��ʱ
        LPM0;
    }

    i_reg = 0x01;
    i = 0;
    while((i_reg == 0x01) && (i < 2))               //�ȴ�������ϣ�������ʱʱ�䵽
    {   
        i++;
        CounterSet();
        CountValue = 0x2710;                        //��ʱ 10ms
        StartCounter;                               //�����ϼ���ģʽ��ʱ
        LPM0;
    }

    if(RXErrorFlag == 0x02)                         //������մ����������жϱ�־λ
    {
        i_reg = 0x02;
    }

    if(i_reg == 0xff)                               //����жϴ��ͽ������
    {
        if(!POLLING)
        {
            kputchar('(');
            for(i = 1; i < 6; i++) Put_byte(buf[i]);
            kputchar(')');
        }
        
        switch(select)                              //���ݴ���ֵ��ѡ��ִ��
        {
            case 0x93:                              //�����ȼ�1
            if((buf[1] == 0x88) || (*UID == 0x88))  //UID��δ��������
            {
                if(NvBytes > 0)
                {
                    for(i = 0; i < 4; i++)
                    {
                        if(i < (NvBytes - 1))       //����֪���ֽںͽ��յ����ֽںϲ���һ��������UID
                            completeUID[i] = *(UID + i + 1);
                        else if(i == (NvBytes - 1)) //����������λ�ϲ�������UID��
                            completeUID[i] = (buf[i + 2 - NvBytes] &~NvBits) | (*(UID + i + 1) & NvBits);
                        else                        //�����յ����ֽ���ӵ�UID��
                            completeUID[i] = buf[i + 2 - NvBytes];
                    }
                }   /*if(NvBytes > 0)*/
                else                                //�����Ч�ֽ�Ϊ0������Чλ�ϲ���UID��
                {
                    completeUID[0] = (buf[2] &~NvBits) | (*UID & NvBits);
                    for(i = 0; i < 4; i++)
                    {
                        completeUID[i + 1] = buf[i + 3];
                    }   /* for */
                }   /* else */

                buf[1] = 0x88;
                for(i = 0; i < 4; i++) 
                    buf[i + 2] = completeUID[i];

                SelectCommand(select, &buf[1]);
                NVB = 0x20;
                more = 1;                           //������־λ�趨����ݹ����
            }
            else                                    //UID������ȫ����UID��ʾ��LCMͼ����ʾ����
            {
                if(POLLING)
                {
                    found = 1;                      //�ҵ�ISO14443A��Ƭ
                    Display_find_tag(1);            //���øú�������ʾ��⵽��ISO14443A��Ƭ����Ϣ                       		 

                    /* ����UID��1λ���ݣ���������LCM����ʾ���� */
                    /*====================================================================================================*/
                    temp1 = (buf[1] &~NvBits) | (*UID & NvBits);                    
                    Display_Char(6, 0, (unsigned char *) (Num + (temp1 >> 4) * 0x10));
                    Display_Char(6, 1, (unsigned char *) (Num + (temp1 & 0x0f) * 0x10));
                    /*====================================================================================================*/
                
                    /* ����UID��2,3,4,5λ���ݣ���������LCM����ʾ���� */
                    /*====================================================================================================*/
                    for (temp1 = 2, temp2 = 2; temp1 < 6; temp1++, temp2 += 2)
                    {
                        Display_Char(6, temp2, (unsigned char *) (Num + (buf[temp1] >> 4) * 0x10));
                        Display_Char(6, temp2 + 1, (unsigned char *)(Num + (buf[temp1] & 0x0f) * 0x10));
                    }
                    /*====================================================================================================*/
                }
                else
                {
                    kputchar('[');                  //��UID�ŷ�������λPC��
                    if(NvBytes > 0)
                    {
                        kputchar('b');
                        for(i = 0; i < 4; i++)
                        {
                            if(i < (NvBytes - 1))   //����֪���ֽںͽ��յ����ֽ���ϳ�������UID��
                                Put_byte(*(UID + i + 1));
                            else if(i = (NvBytes - 1))
                                Put_byte((buf[i + 2 - NvBytes] &~NvBits) | (*(UID + i + 1) & NvBits));
                            else
                                Put_byte(buf[i + 2 - NvBytes]);
                        }/* for */
                    }
                    else
                    {
                        Put_byte((buf[1] &~NvBits) | (*UID & NvBits));
                        for(i = 0; i < 4; i++)
                        {
                            Put_byte(buf[i + 2]);
                        }/* for */
                    }/* if-else */
                    kputchar(']');
                /*====================================================================================================*/
                }
            }   /* else */

            select = 0x95;                          //selectֵΪ0x95,����Ϊ2
            break;

            case 0x95:                              //�����ȼ�2
            if(buf[1] == 0x88)                      //UID��δ��������
            {
                for(i = 0; i < 4; i++)
                {
                    completeUID[i + 4] = buf[i + 2];
                }
                SelectCommand(select, &buf[1]);     //ѡ�����������д�뵽FIFO��
                more = 1;                           //������־λ�趨����ݹ����
            }
            else                                    //UID������ȫ����UID��ʾ��LCMͼ����ʾ����
            {                           		
                for(i = 0; i < 5; i++)
                {
                    completeUID[i + 4] = buf[i + 1];
                }
                
                if(POLLING)
                {
                    found = 1;
                    Display_find_tag(1);            //���øú�������ʾ��⵽��ISO14443A��Ƭ����Ϣ
                 
                    /* ����UID��0,1,2λ���ݣ���������LCM����ʾ���� */
                    /*====================================================================================================*/
                    for (temp1 = 0, temp2 = 0; temp1 < 3; temp1++, temp2 += 2)
                    {
                        Display_Char(6, temp2, (unsigned char *)(Num + (completeUID[temp1] >> 4) * 0x10));
                        Display_Char(6, temp2 + 1, (unsigned char *)(Num + (completeUID[temp1] & 0x0f) * 0x10));
                    }
                    /*====================================================================================================*/
                
                    /* ����UID��4,5,6,7λ���ݣ���������LCM����ʾ���� */
                    /* completeUID[3]ΪISO14443AЭ���е�BCC1У��ֵ */
                    /* completeUID[8]ΪISO14443AЭ���е�BCC2У��ֵ */
                    /*====================================================================================================*/
                    for (temp1 = 4, temp2 = 6; temp1 < 8; temp1++, temp2 += 2)
                    {
                        Display_Char(6, temp2, (unsigned char *)(Num + (completeUID[temp1] >> 4) * 0x10));
                        Display_Char(6, temp2 + 1, (unsigned char *)(Num + (completeUID[temp1] & 0x0f) * 0x10));
                    }
                    /*====================================================================================================*/
                }
                else
                {
                    kputchar('[');
                    for(i = 0; i < 3; i++)          //����UID�ȼ�1
                        Put_byte(completeUID[i]);
                    Put_byte(completeUID[3]);       //����UID1��BCCУ����

                    for(i = 4; i < 8; i++)          //����UID�ȼ�2
                        Put_byte(completeUID[i]);
                    Put_byte(completeUID[8]);       //����UID2��BCCУ����
                    kputchar(']');
                }
                /*====================================================================================================*/
            }

            select = 0x97;                          //selectֵΪ0x97,����Ϊ3
            break;

        case 0x97:                                  //�����ȼ�3                
            /* �������������е����ݴ洢������UID������ */
            /*====================================================================================================*/
            for(i = 0; i < 5; i++)
            {
                completeUID[i + 8] = buf[i + 1];
            }
            /*====================================================================================================*/

            if(POLLING)
            {
                found = 1;
                Display_find_tag(1);                //���øú�������ʾ��⵽��ISO14443A��Ƭ����Ϣ
            
                /* ����UID��0,1,2λ���ݣ���������LCM����ʾ���� */
                /*====================================================================================================*/
                for (temp1 = 0, temp2 = 0; temp1 < 3; temp1++, temp2 += 2)
                {
                    Display_Char(6, temp2, (unsigned char *)(Num + (completeUID[temp1] >> 4) * 0x10));
                    Display_Char(6, temp2 + 1, (unsigned char *)(Num + (completeUID[temp1] & 0x0f) * 0x10));
                }
                /*====================================================================================================*/
            
                /* completeUID[3]ΪISO14443AЭ���е�BCC1У��ֵ */
            
                /* ����UID��4,5,6λ���ݣ���������LCM����ʾ���� */
                /*====================================================================================================*/
                for (temp1 = 4, temp2 = 6; temp1 < 7; temp1++, temp2 += 2)
                {
                    Display_Char(6, temp2, (unsigned char *)(Num + (completeUID[temp1] >> 4) * 0x10));
                    Display_Char(6, temp2 + 1, (unsigned char *)(Num + (completeUID[temp1] & 0x0f) * 0x10));
                }
                /*====================================================================================================*/
            
                /* completeUID[7]ΪISO14443AЭ���е�BCC2У��ֵ */
                /* completeUID[12]ΪISO14443AЭ���е�BCC3У��ֵ */
            
                /* ����UID��8,9,10,11λ���ݣ���������LCM����ʾ���� */
                /*====================================================================================================*/
                for (temp1 = 8, temp2 = 12; temp1 < 12; temp1++, temp2 += 2)
                {
                    Display_Char(6, temp2, (unsigned char *)(Num + (completeUID[temp1] >> 4) * 0x10));
                    Display_Char(6, temp2 + 1, (unsigned char *)(Num + (completeUID[temp1] & 0x0f) * 0x10));
                }
                /*====================================================================================================*/
            }
            else
            {
                kputchar('[');
                for(i = 0; i < 3; i++)              //����UID�ȼ�1
                    Put_byte(completeUID[i]);
                Put_byte(completeUID[3]);           //����UID1��BCCУ����

                for(i = 4; i < 7; i++)              //����UID�ȼ�2
                    Put_byte(completeUID[i]);
                Put_byte(completeUID[7]);           //����UID2��BCCУ����

                for(i = 8; i < 12; i++)             //����UID�ȼ�3
                    Put_byte(completeUID[i]);
                Put_byte(completeUID[12]);          //����UID3��BCCУ����
                kputchar(']');
            }
            /*====================================================================================================*/
            
            break;
        }   /* switch */
    }   /* if(i_reg == 0xff) */
    else if(i_reg == 0x02)                          //��ײ����
    {  
        if(!POLLING)
        {
            kputchar('(');
            kputchar('z');
            kputchar(')');
        }
    }
    else if(i_reg == 0x00)                          //��ʱ���ж�
    {               
        if(!POLLING)
        {
            kputchar('(');
            kputchar(')');
        }
    }
    else
        ;

    if(i_reg == 0x02)                               //�����ײ�����������³�ײѭ��
    {                   	
        CollPoss++;                                 //�Ķ������س�ײλ�ü�1
        for(i = 1; i < 5; i++)
            newUID[i - 1] = buf[i];                 //���µ�UID���鸳ֵ

        CounterSet();                               //���ö�ʱ��
        CountValue = 100;                           //��ʱʱ��Ϊ1.2ms
        StartCounter;                               //��ʼ�Ե���ģʽ��ʱ
        i_reg = 0x01;
        while(i_reg == 0x01)                        //�ȴ�RX���ս������ߵȴ�ʱ�䵽
        {
        }               	

        AnticollisionLoopA(select, CollPoss,newUID);//�ݹ����AnticollisionLoopA����
    }   /* if(i_reg == 0x02) */

    if(more)                                        //����д�����־�趨����ݹ���ú���ִ�з³�ײ������õ�7������10���ֽڳ��ȵ�UID
    {
        AnticollisionLoopA(select, NVB, UID);       //�ݹ���ú�����UID�룺��ѡ��󼶲�ͬ����������ͬ
        if(POLLING)found = 1;                       //�ҵ���Ƭ
    }   /* if(more) */

    if(found)                                       //����ҵ���Ƭ����LED��ӦЭ��ָʾ����
    {
        LEDtypeAON();               
        Beep_Waring(1,BeeptypeA);                   //����������A��������������
    }
    else                                            //���δ�ҵ���Ƭ����LEDϨ�𡢷�����������
    {
        LEDtypeAOFF();
        BeepOFF();
    }
}   /* AnticollisionLoopA */

/******************************************************************************************************************
* �������ƣ�AnticollisionSequenceA()
* ��    �ܣ�ISO14443A�³�ײ���С�
* ��ڲ�����REQA       ��������
* ���ڲ�������    
* ˵    �����ú�������REQA��������ִ��ISO14443A��Ƭ��ͬ������
            ���ѻ�ʵ����ʾ���������ʾ�˶�ȡUID�룬��Ϊ0x00��ΪWUPA�������
*******************************************************************************************************************/
void AnticollisionSequenceA(unsigned char REQA)
{
    unsigned char i, select = 0x93, NVB = 0x20;
    
    buf[0] = ModulatorControl;                      // ���ƺ�ϵͳʱ�ӿ��ƣ�0x21 - 6.78MHz OOK(100%)
    buf[1] = 0x21;
    WriteSingle(buf, 2);

    buf[0] = ISOControl;                            // ����ѡ��ISO14443A����ģʽΪ:������106kbps
    buf[1] = 0x88;                                  // ���ղ���CRCУ��
    WriteSingle(buf, 2);

    /* �ж�REQAֵ����Ϊ0����WUPA�����������Ϊ0����ΪREQA�������� */
    /*====================================================================================================*/
    if(REQA) 
        buf[5] = 0x26;                              //���� REQA ���� */
    else
        buf[5] = 0x52;                              //���� WUPA ���� */
    /*====================================================================================================*/
    
    RequestCommand(&buf[0], 0x00, 0x0f, 1);         //������������
    IRQCLR();                                       //���жϱ�־λ
    IRQON();                                        //�ⲿ�жϿ���

    if(i_reg == 0xff || i_reg == 0x02)              //������յ����ݻ��߳�ײ����
    {
        for(i = 40; i < 45; i++)                    //�� buf ���
            buf[i] = 0x00;
      
        AnticollisionLoopA(select, NVB, &buf[40]);  //���÷³�ײѭ�� AnticollisionLoopA ����
        if(POLLING) 
            LEDtypeAON();
    }
    else                                            //����LEDָʾ���𣬷�������
    {
        LEDtypeAOFF();
    }

    buf[0] = ISOControl;
    buf[1] = 0x08;                                  //��������TRF7960��Ϊ���ղ���CRCУ��
    WriteSingle(buf, 2);
    IRQOFF();                                       //�жϹر�
}   /* AnticollisionSequenceA */

/******************************************************************************************************************
* �������ƣ�Request14443A()
* ��    �ܣ�ISO14443A�����������
* ��ڲ�����pbuf       ��������
*           lenght     �����
*           BitRate    ������
* ���ڲ�����1 �ɹ�ִ�� 0������    
* ˵    �����������������ִ��ISO14443A��Ƭ�������������
*******************************************************************************************************************/
unsigned char Request14443A(unsigned char *pbuf, unsigned char lenght, unsigned char BitRate)
{
    unsigned char index, j, command, RXBitRate, TXBitRate, reg[2];

    TXBitRate = ((BitRate >> 4) & 0x0F) + 0x08;
    RXBitRate = (BitRate & 0x0F) + 0x08;

    reg[0] = ISOControl;
    reg[1] = TXBitRate;
    WriteSingle(reg, 2);

    RXTXstate = lenght;     

    *pbuf = 0x8f;
    *(pbuf + 1) = 0x91;                             //ΪFIFO��д�����û�����
    *(pbuf + 2) = 0x3d;
    *(pbuf + 3) = RXTXstate >> 4;
    *(pbuf + 4) = RXTXstate << 4;

    if(lenght > 12) lenght = 12;

    RAWwrite(pbuf, lenght + 5);                     //ʹ��ֱ��дģʽ������������

    IRQCLR();                                       //����ⲿ�жϱ�־λ
    IRQON();                                        //�����ж�

    RXTXstate = RXTXstate - 12;
    index = 18;

    i_reg = 0x01;

    while(RXTXstate > 0)
    {
        LPM0;                                       //����͹���ģʽ�����ж�ʱ�˳�
        if(RXTXstate > 9)                           //���δ���͵��ֽ���������9
        {                          
            lenght = 10;                            //�������ó�10
        }
        else if(RXTXstate < 1)                      //������е��ֽ��Ѿ����͵�FIFO�У�����жϷ���
        {
            break;                
        }
        else
        {
            lenght = RXTXstate + 1;                 //�����ֽ��Ѿ�������
        }   /* if */

        buf[index - 1] = FIFO;                      //���͹����У�д��9�����߸��ٵ��ֽڵ�FIFO��
        WriteCont(&buf[index - 1], lenght);
        RXTXstate = RXTXstate - 9;                  //д9�ֽڵ�FIFO��
        index = index + 9;
    }   /* while */

    RXTXstate = 1;         
    while(i_reg == 0x01)
    {
    }

    reg[0] = ISOControl;
    reg[1] = RXBitRate;
    WriteSingle(reg, 2);

    command = 0x16;
    DirectCommand(&command);
    command = 0x17;
    DirectCommand(&command);

    i_reg = 0x01;

    CounterSet();
    CountValue = 0xF000;                            //����60ms�ȴ�ʱ��
    StartCounter;                                   //������ʱ��

    while(i_reg == 0x01)                            //�ȴ�RX�������
    {
    }               

    if(i_reg == 0xFF)                               //���յ�Ӧ��
    {                       
        kputchar('[');
        for(j = 1; j < RXTXstate; j++)
        {
            Put_byte(buf[j]);
        }   /* for */

        kputchar(']');
        return(0);
    }
    else if(i_reg == 0x02)                          //��ײ����
    {       
        kputchar('[');
        kputchar('z');
        kputchar(']');
        return(0);
    }
    else if(i_reg == 0x00)                          //��ʱ���ж�
    {               
        kputchar('[');
        kputchar(']');
        return(1);
    }
    else
        ;

    IRQOFF();
    return(1);
}   /* Request14443A */

/******************************************************************************************************************
* �������ƣ�SlotMarkerCommand()
* ��    �ܣ��ú�������ISO14443BЭ��۱�����������ͬʱ�����˲ۺš�
* ��ڲ�����number       �ۺ�
* ���ڲ�������    
* ˵    ������
*******************************************************************************************************************/
void SlotMarkerCommand(unsigned char number)
{
    buf[0] = 0x8f;
    buf[1] = 0x91;
    buf[2] = 0x3d;
    buf[3] = 0x00;
    buf[4] = 0x10;
    RAWwrite(&buf[0], 5);                           //д��������ֵ

    buf[5] = 0x3F;
    buf[6] = (number << 4) | 0x05;
    buf[7] = 0x00;
    
    i_reg = 0x01;
    RAWwrite(&buf[5], 3);                           //д��������ֵ

    IRQCLR();                                       //���жϱ�־λ
    IRQON();                                        //���ж�

    while(i_reg == 0x01)
    {
        CounterSet();                               //��ʱ������
        CountValue = 0x9c40;                        //��ʱʱ�� 40ms
        StartCounter;                               //��ʼ��ʱ
        LPM0;   
    }
}   

/******************************************************************************************************************
* �������ƣ�AnticollisionSequenceB()
* ��    �ܣ�ISO14443B�³�ײ���С�
* ��ڲ�����command       ��������
*           slots           �ۺ�
* ���ڲ�������    
* ˵    �����ú������� command ��������Ͳۺ�slotsִ��ISO14443B��Ƭ��ͬ������
*******************************************************************************************************************/
void AnticollisionSequenceB(unsigned char command, unsigned char slots)
{
    unsigned char i, collision = 0x00, j, found = 0;
    unsigned int k = 0;
    unsigned char temp1, temp2;

    buf[0] = ModulatorControl;                      // ���ƺ�ϵͳʱ�ӿ��ƣ�0x20 - 6.78MHz ASK(10%)
    buf[1] = 0x20;
    WriteSingle(buf, 2);

    RXErrorFlag = 0x00;

    buf[0] = 0x8f;
    buf[1] = 0x91;
    buf[2] = 0x3d;
    buf[3] = 0x00;
    buf[4] = 0x30;
    buf[5] = 0x05;
    buf[6] = 0x00;
    //buf[6] = 0x20;                                //AFI Ӧ�����־ֵ

    if(slots == 0x04)                               //0x04��ʾ16����
    {
        EnableSlotCounter();                        //ʹ�ܲۼ�����
        buf[7] |= 0x08;
    }

    buf[7] = slots;

    if(command == 0xB1)                             //���Ϊ0xB1����ô��WUPB��������
        buf[7] |= 0x08;                             //�����0xB1����ô��REQB��������

    i_reg = 0x01;
    RAWwrite(&buf[0], 8);                           //д������8������ֵ��FIFO��

    IRQCLR();                                       //���жϱ�־λ
    IRQON();                                        //���ж�

    j = 0;
    while((i_reg == 0x01) && (j < 2))               //�ȴ�TX���ͽ���
    {
        j++;
        CounterSet();                               //��ʱ������
        CountValue = 0x4E20;                        //��ʱʱ�� 20ms
        StartCounter;                               //��ʼ��ʱ
        LPM0;
    }   

    i_reg = 0x01;                                   //�ָ���־λ
    CounterSet();                                   //��ʱ������
    CountValue = 0x4E20;                            //��ʱʱ�� 20ms
    //CountValue = 0x9c40;                          //��ʱʱ�� 20ms �ھ���Ϊ 13.56 MHz������£���ֵΪ0x9c40*/
    StartCounter;                                   //��ʼ��ʱ

    for(i = 1; i < 17; i++)                         //1-16������ѭ
    {
        RXTXstate = 1;                              //Ӧ�����ݽ����洢��buf[1]�Ժ��ַ��

        while(i_reg == 0x01)                        //�ȴ�RX�������
        {               	
            k++;
            if(k == 0xFFF0)
            {
                i_reg = 0x00;
                RXErrorFlag = 0x00;
                break;
            }
        }

        if(RXErrorFlag == 0x02) 
            i_reg = RXErrorFlag;

        if(i_reg == 0xFF)                           //������յ�PUPI
        {                   	
            if(POLLING)
        {
                found = 1;                          //���ñ�־λ����ʾ�ҵ�ISO14443BЭ�鿨Ƭ
                Display_find_tag(2);                //��ʾ��⵽��ISO14443A��Ƭ����Ϣ   
            
                for (temp1 = 2, temp2 = 0; temp1 < 6; temp1++, temp2 += 2)
                {
                    Display_Char(6, temp2, (unsigned char *)(Num + (buf[temp1] >> 4) * 0x10));
                    Display_Char(6, temp2 + 1, (unsigned char *)(Num+(buf[temp1] & 0x0f) * 0x10));
                }
            }
            else
            {             
                kputchar('[');
                for(j = 1; j < RXTXstate; j++) Put_byte(buf[j]);
                kputchar(']');
            }
        }
        else if(i_reg == 0x02)                      //��ײ����
        {               
            if(!POLLING)
            {
                kputchar('[');
                kputchar('z');
                kputchar(']');
            }
            collision = 0x01;
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

        /* �жϲۺż�ѭ��������������16�Σ�������forѭ�� */
        /*====================================================================================================*/
        if((slots == 0x00) || (slots == 0x01) || (slots == 0x02) || ((slots == 0x04) && (i == 16))) break;
        /*====================================================================================================*/

        SlotMarkerCommand(i);                       //ִ�в۱�ǹ���

        i_reg = 0x01;
        if(!POLLING)
        {
            put_crlf();
        }
    }   /* for */

    if(slots == 0x04)                               //���Ϊ16�ۣ���ֹͣ�ۼ���
        DisableSlotCounter();

    IRQOFF();                                       //�ر��ж�

    if(found)                                       //����ҵ���Ƭ����LED��ӦЭ��ָʾ����           
    {
        LEDtypeBON();
        Beep_Waring(1,BeeptypeB);                   //����������A��������������
    }
    else                                            //���δ�ҵ���Ƭ����LEDϨ�𡢷�����������
    {
        LEDtypeBOFF();
        BeepOFF();
    }

    if(collision != 0x00)                           //�����ײλ������0x00����ݹ����16ʱ���ISO14443B���к���
        AnticollisionSequenceB(0x20, 0x02);
}   /* AnticollisionSequenceB */





