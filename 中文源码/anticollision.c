/**********************************************************************************************************************************************************
* �� �� ����ANTICOLLISION.C
* ��    �ܣ�ISO15693Э�鿨Ƭ���������������³�ײ����ȡ�
*           ���ļ�������ISO15693Э�����ʾ������
*           ע�⣺�ڴ���ͽ��չ����У�����Ҫͨ��FIFO������
* ��    �ߣ�EMDOOR
* ��    �ڣ�2011��04��13��
**********************************************************************************************************************************************************/
#include "anticollision.h"
#include "lcd.h"


unsigned char POLLING;                              //�����ѻ�����������־λ:0-Ϊ����USBģʽ;1-Ϊ�ѻ�LCDģʽ
unsigned char Found_tag;                            //�����Ƿ��⵽��Ƭȫ�ֱ���
unsigned char test_no;                              //����RFID��Ƭ������Ŀ���к�
unsigned char rssival;                              //�����⵽�Ŀ�Ƭ�����ź�ǿ��ֵ


/******************************************************************************************************************
* �������ƣ�EnableSlotCounter()
* ��    �ܣ�ʹ�ܲۼ������ܡ�
* ��ڲ�������
* ���ڲ�������     
* ˵    �����ú���ʹ�ܲۼ������ܣ����ڶ����ʱ��
*******************************************************************************************************************/
void EnableSlotCounter(void)
{
    buf[41] = IRQMask;                              //�¸�������
    buf[40] = IRQMask;
    ReadSingle(&buf[41], 1);                        //��ȡ����������
    buf[41] |= BIT0;                                //�ڻ������Ĵ���0x41λ������BIT0��Ч
    WriteSingle(&buf[40], 2);
}

/******************************************************************************************************************
* �������ƣ�DisableSlotCounter()
* ��    �ܣ���ֹ�ۼ������ܡ�
* ��ڲ�������
* ���ڲ�������     
* ˵    �����ú���ʹ�ۼ�������ֹͣ��
*******************************************************************************************************************/
void DisableSlotCounter(void)
{
    buf[41] = IRQMask;                              //�¸�������
    buf[40] = IRQMask;
    ReadSingle(&buf[41], 1);                        //��ȡ����������
    buf[41] &= 0xfe;                                //�ڻ������Ĵ���0x41λ������BIT0��Ч
    WriteSingle(&buf[40], 2);
}

/******************************************************************************************************************
* �������ƣ�InventoryRequest()
* ��    �ܣ�ISO15693Э�鿨Ƭ�����������
* ��ڲ�����*mask       �������
*           lenght      �����
* ���ڲ�������     
* ˵    ����ִ�иú�������ʹISO15693Э���׼��������ѭ��16ʱ��ۻ���1��ʱ���.
*           ���У�0x14��ʾ16�ۣ�0x17��ʾ1���ۡ�
*           ע�⣺���ѻ�ģʽ�£����յ�UID�뽫����ʾ��LCMͼ����ʾ���ϡ�
*******************************************************************************************************************/
void InventoryRequest(unsigned char *mask, unsigned char lenght)
{
    unsigned char i = 1, j=3, command[2], NoSlots, found = 0;
    unsigned char *PslotNo, slotNo[17];
    unsigned char NewMask[8], NewLenght, masksize;
    int size;
    unsigned int k = 0;
    unsigned char temp1, temp2;

    buf[0] = ModulatorControl;                      // ���ƺ�ϵͳʱ�ӿ��ƣ�0x21 - 6.78MHz OOK(100%)
    buf[1] = 0x21;
    WriteSingle(buf, 2);
 
    /* ���ʹ��SPI����ģʽ�ĵ������ʣ���ô RXNoResponseWaitTime ��Ҫ���������� */
    /*====================================================================================================*/
    if(SPIMODE)
    {
        if((flags & BIT1) == 0x00)                  //�����ݱ�����
        {
            buf[0] = RXNoResponseWaitTime;
            buf[1] = 0x2F;
            WriteSingle(buf, 2);
        }
        else                                        //�����ݱ�����
        {
            buf[0] = RXNoResponseWaitTime;
            buf[1] = 0x13;
            WriteSingle(buf, 2);
        }
    }
    /*====================================================================================================*/
    
    slotNo[0] = 0x00;

    if((flags & BIT5) == 0x00)                      //λ5��־λָʾ�۵�����
    {                       
        NoSlots = 17;                               //λ5Ϊ0x00����ʾѡ��16��ģʽ
        EnableSlotCounter();
    }
    else                                            //���λ5��Ϊ0x00����ʾѡ��1����ģʽ
        NoSlots = 2;

    PslotNo = &slotNo[0];                           //������ָ��
    
    /* ���lenght��4����8����ômasksize ��Ǵ�СֵΪ 1  */
    /* ���lenght��12����16����ômasksize ��Ǵ�СֵΪ 2������������ */
    /*====================================================================================================*/
    masksize = (((lenght >> 2) + 1) >> 1);      
    /*====================================================================================================*/
    
    size = masksize + 3;                            // mask value + mask lenght + command code + flags

    buf[0] = 0x8f;
    buf[1] = 0x91;                                  //���ʹ�CRCУ��
    buf[2] = 0x3d;                                  //����дģʽ
    buf[3] = (char) (size >> 8);
    buf[4] = (char) (size << 4);
    buf[5] = flags;                                 //ISO15693 Э���־flags
    buf[6] = 0x01;                                  //�³�ײ����ֵ

    /* �����ڴ˼���AFIӦ�����ʶ�� */

    buf[7] = lenght;                                //��ǳ��� masklenght
    if(lenght > 0)
    {
        for(i = 0; i < masksize; i++) 
            buf[i + 8] = *(mask + i);
    }                   

    command[0] = IRQStatus;
    command[1] = IRQMask;                           //�����(Dummy read)
    ReadCont(command, 1);

    CounterSet();                                   //���ö�ʱ��
    CountValue = count1ms * 20;                     //��ʱʱ��Ϊ 20ms
    IRQCLR();                                       //���жϱ�־λ
    IRQON();                                        //�жϿ���

    RAWwrite(&buf[0], masksize + 8);                //������д�뵽FIFO��������

    i_reg = 0x01;                                   //�����жϱ�־ֵ
    StartCounter;                                   //��ʼ�Ե���ģʽ��ʱ
    LPM0;                                           //�ȴ�TX���ͽ���

    for(i = 1; i < NoSlots; i++)                    //Ѱ��ѭ��1���ۻ���16����
    {       
        /* ��ʼ��ȫ�ּ����� */
        /*====================================================================================================*/
        RXTXstate = 1;                              //���ñ�־λ�������λ�洢��buf[1]��ʼλ��
        CounterSet();                               //���ö�ʱ��
        CountValue = count1ms * 20;                 //��ʱʱ��Ϊ 20ms 
        //CountValue = 0x4E20;                      
        StartCounter;                               //��ʼ�Ե���ģʽ��ʱ
        k = 0;
        LPM0;
        /*====================================================================================================*/
        
        while(i_reg == 0x01)                        //�ȴ�RX���ս���
        {           
            k++;

            if(k == 0xFFF0)
            {
                i_reg = 0x00;
                RXErrorFlag = 0x00;
                break;
            }
        }

        command[0] = RSSILevels;                    //��ȡ�ź�ǿ��ֵ RSSI
        ReadSingle(command, 1);

        if(i_reg == 0xFF)                           //����ֽ��Ѿ�������ֽڣ����յ�UID����
        {     
            if(POLLING)
            {
                found = 1;
                Display_find_tag(0);                //LCM��ʾ�ҵ���Э���꿨Ƭ��Ϣ
            
                for(temp1 = 10, temp2 = 0; temp1 > 2; temp1--, temp2 += 2)
                {
                    Display_Char(6, temp2, (unsigned char *)(Num + (buf[temp1] >> 4) * 0x10));
                    Display_Char(6, temp2 + 1, (unsigned char *)(Num+(buf[temp1] & 0x0F) * 0x10));
                }
                    
                rssival= 0x07 & (command[0] >> 3);   //��ʾ���յ����ź�ǿ��
                Display_Rssi(rssival);
            }
            else
            {
                kputchar('[');                      //���ͷ��� [               
                for(j = 3; j < 11; j++)
                {
                    Put_byte(buf[j]);               //����ISO15693 UID��
                }
                kputchar(',');
                Put_byte(command[0]);               //����RSSI�����ź�ǿ��
                kputchar(']');  
            }
        }
        else if(i_reg == 0x02)                      //����г�ײ����
        {  
            if(!POLLING)
            {
                kputchar('[');
                kputchar('z');                      //���� z
                kputchar(',');
                Put_byte(command[0]);               //����RSSI�����ź�ǿ��
                kputchar(']');
            }
            PslotNo++;
            *PslotNo = i;
        }
        else if(i_reg == 0x00)                      //�����ʱʱ�䵽���жϷ���
        { 
            if(!POLLING)
            {
                kputchar('[');
                kputchar(',');
                Put_byte(command[0]);               //����RSSI�����ź�ǿ��
                kputchar(']');
            }   
        }
        else
            ;

        command[0] = Reset;                         //�ڽ�����һ����֮ǰ��ʹ��ֱ�����λFIFO
        DirectCommand(command);

        if((NoSlots == 17) && (i < 16))             //�����16����ģʽ�£�δѭ��16���ۣ�����Ҫ����EOF����(�¸���)
        {                   
            command[0] = StopDecoders;
            DirectCommand(command);                 //ֹͣ��a
            command[0] = RunDecoders;               
            DirectCommand(command);             
            command[0] = TransmitNextSlot;
            DirectCommand(command);                 //������һ����
        }
        else if((NoSlots == 17) && (i == 16))       //�����16����ģʽ�£�ѭ����16���ۣ�����Ҫ����ֹͣ�ۼ�������
        {                   
            DisableSlotCounter();                   //ֹͣ�ۼ���
        }
        else if(NoSlots == 2)                       //����ǵ�����ģʽ���������� for ѭ��
            break;

        if(!POLLING)
    {
            put_crlf();                             //���ͻس���������
        }
    }   /* for */

    if(found)                                       //����ҵ���Ƭ����LED��ӦЭ��ָʾ����
    {                       
        LED15693ON();
        Beep_Waring(1,Beep15693);                   //����������ISO15693��������������
        Found_tag= 1;
    }
    else
    {
        LED15693OFF();                              //���δ�ҵ���Ƭ����LEDϨ�𡢷�����������
        found = 0;
        BeepOFF();
    Found_tag = 0;
    }

    NewLenght = lenght + 4;                         //��ǳ���Ϊ4����λ����
    masksize = (((NewLenght >> 2) + 1) >> 1) - 1;

    /* �����16����ģʽ������ָ�벻Ϊ0x00, ��ݹ���ñ��������ٴ�Ѱ�ҿ�Ƭ */
    /*====================================================================================================*/
    while((*PslotNo != 0x00) && (NoSlots == 17)) 
    {
        *PslotNo = *PslotNo - 1;
        for(i = 0; i < 8; i++) NewMask[i] = *(mask + i);            //���Ƚ����ֵ�������±��������

        if((NewLenght & BIT2) == 0x00) *PslotNo = *PslotNo << 4;

        NewMask[masksize] |= *PslotNo;                              //���ֵ���ı�
        InventoryRequest(&NewMask[0], NewLenght);                   //�ݹ���� InventoryRequest ����
        PslotNo--;                                                  //�۵ݼ�
    }   /* while */
    /*====================================================================================================*/
    
    IRQOFF();                                                       //�³�ײ���̽������ر��ж�
}   /* InventoryRequest */


/******************************************************************************************************************
* �������ƣ�RequestCommand()
* ��    �ܣ���ƬЭ�������������ͼ�ʱ�Ķ�������Ƭ��Ӧ��
* ��ڲ�����*pbuf           ����ֵ
*           lenght          �����
*           brokenBits      �������ֽڵ�λ����
*           noCRC           �Ƿ���CRCУ��
* ���ڲ�����1     
* ˵    �����ú���ΪЭ���������������1����˵���ú����ɹ�ִ�У�������0���߲����أ����쳣��
*******************************************************************************************************************/
unsigned char RequestCommand(unsigned char *pbuf, unsigned char lenght, unsigned char brokenBits, char noCRC)
{
    unsigned char index, j, command;                //�������
    unsigned char temp2;
    
    RXTXstate = lenght;                             

    *pbuf = 0x8f;
    if(noCRC) 
        *(pbuf + 1) = 0x90;                         //���䲻��CRCУ��
    else
        *(pbuf + 1) = 0x91;                         //�����CRCУ��
    
    *(pbuf + 2) = 0x3d;
    *(pbuf + 3) = RXTXstate >> 4;
    *(pbuf + 4) = (RXTXstate << 4) | brokenBits;

    if(lenght > 12)
        lenght = 12;

    if(lenght == 0x00 && brokenBits != 0x00)
    {
        lenght = 1;
        RXTXstate = 1;
    }

    RAWwrite(pbuf, lenght + 5);                     //��ֱ��дFIFOģʽ��������

    IRQCLR();                                       //���жϱ�־λ
    IRQON();

    RXTXstate = RXTXstate - 12;
    index = 17;

    i_reg = 0x01;
    while(RXTXstate > 0)
    {
        LPM0;                                       //����͹���ģʽ�����˳��ж�
        if(RXTXstate > 9)                           //��RXTXstateȫ�ֱ�����δ���͵��ֽ������������9
        {                       
            lenght = 10;                            //����Ϊ10�����а���FIFO�е�9���ֽڼ�1���ֽڵĵ�ֵַ
        }
        else if(RXTXstate < 1)                      //�����ֵС��1����˵�����е��ֽ��Ѿ����͵�FIFO�У������жϷ���
        {
            break;
        }
        else                                        //���е�ֵ�Ѿ�ȫ��������
        {
            lenght = RXTXstate + 1;         
        }   /* if */

        buf[index - 1] = FIFO;                      //��FIFO��������д��9�����߸����ֽڵ����ݣ������ڷ���
        WriteCont(&buf[index - 1], lenght);
        RXTXstate = RXTXstate - 9;                  //д9���ֽڵ�FIFO��
        index = index + 9;
    }   /* while */

    RXTXstate = 1;                                  //���ñ�־λ�������λ�洢��buf[1]��ʼλ��

    while(i_reg == 0x01)                            //�ȴ����ͽ���
    {
        CounterSet();                               //��ʱ������
        CountValue = 0xF000;                        //��ʱʱ�� 60ms
        StartCounter;                               //��ʼ�Ե���ģʽ��ʱ
        LPM0;
    }

    i_reg = 0x01;
    CounterSet();                                   //��ʱ������
    CountValue = 0xF000;                            //��ʱʱ�� 60ms
    StartCounter;                                   //��ʼ�Ե���ģʽ��ʱ

    /* ����жϱ�־λ�������ȸ�λ�����¸������� */
    /*====================================================================================================*/
    if((((buf[5] & BIT6) == BIT6) && ((buf[6] == 0x21) || (buf[6] == 0x24) || (buf[6] == 0x27) || (buf[6] == 0x29)))
    || (buf[5] == 0x00 && ((buf[6] & 0xF0) == 0x20 || (buf[6] & 0xF0) == 0x30 || (buf[6] & 0xF0) == 0x40)))
    {
        delay_ms(20);
        command = Reset;
        DirectCommand(&command);
        command = TransmitNextSlot;
        DirectCommand(&command);
    }   /* if */
    /*====================================================================================================*/
    
    while(i_reg == 0x01)                            //�ȴ��������
    { 
    }
    
    if(POLLING)                                     //?������ѻ�ģʽ���ڲ��������
    {
        if(Found_tag)
        {
            if(i_reg == 0xFF)                       //���յ�Ӧ��
            {
                if((buf[1]) == 0x00)                //�����ɹ� 
                {
                    Display_Char(6, 13, (unsigned char *)CO);           //��ʾOK
                    Display_Char(6, 14, (unsigned char *)CK);
                    Beep_Waring(1, Beep15693); 
                }
                else                                //����ʧ��
                {
                    Display_Char(6,10,(unsigned char *)Num+0x0E*0x10);  //��ʾERROR
                    Display_Char(6, 11, (unsigned char *)CR);
                    Display_Char(6, 12, (unsigned char *)CR);
                    Display_Char(6, 13, (unsigned char *)CO);
                    Display_Char(6, 14, (unsigned char *)CR);
                    Beep_Waring(3, Beep15693);      //���������б�������
                }
            }
        }
        
          switch(test_no)
        {
            case 0:                                 //дһ����
                break;
            case 1:                                 //��һ����
                for(j = 2, temp2 = 15; j < RXTXstate; j++, temp2 -= 2)
                {   //��ʾ��ȡ������
                    Display_Char(2, temp2 - 1, (unsigned char *)(Num + (buf[j] >> 4) * 0x10));
                    Display_Char(2, temp2, (unsigned char *)(Num+(buf[j] & 0x0F) * 0x10));
                }                                   
                break;
            case 2:                                 //дAFI
                break;
            case 3:                                 //дDSFID
                break;
            case 4:                                 //��ȡϵͳ��Ϣ
                for(j = 11, temp2 = 9; j < RXTXstate - 1; j++, temp2 += 2)
                {   //��ʾ��ȡ���Ŀ�Ƭϵͳ��Ϣ
                    Display_Char(2, temp2-1, (unsigned char *)(Num + (buf[j] >> 4) * 0x10));
                    Display_Char(2, temp2, (unsigned char *)(Num+(buf[j] & 0x0F) * 0x10));
                }                                 
                break;
            case 5:                                 //��ȡ�鰲ȫ״̬
                 for(j = 2, temp2 = 15; j < RXTXstate; j++, temp2 -= 2)
                {   //��ʾ�찲ȫ״̬��Ϣ
                    Display_Char(2, temp2 - 1, (unsigned char *)(Num + (buf[j] >> 4) * 0x10));
                    Display_Char(2, temp2, (unsigned char *)(Num+(buf[j] & 0x0F) * 0x10));
                }                                   
                break;
            default:break;
        }
    }
    
  
    
     if(!POLLING)
    {
        switch(noCRC)                               //�Ƿ����CRCУ��
        {
            case 0:
            if(i_reg == 0xFF)                       //���յ�Ӧ��
            {       
                kputchar('[');                      //����[]
                for(j = 1; j < RXTXstate; j++)
                {
                    Put_byte(buf[j]);
                }   /* for */

                kputchar(']');
                return(0);
            }
            else if(i_reg == 0x02)                  //��ײ����
            {      
                kputchar('[');                      //����[z]
                kputchar('z');
                kputchar(']');
                return(0);
            }
            else if(i_reg == 0x00)                  //��ʱʱ�䵽
            {      
                kputchar('[');
                kputchar(']');
                return(1);
            }
            else
            ;
            break;

            case 1:
            if(i_reg == 0xFF)                       //���յ�Ӧ��
                        {
                kputchar('(');                      //���ͣ���
                for(j = 1; j < RXTXstate; j++)
                {
                    Put_byte(buf[j]);
                }   /* for */

                kputchar(')');
                return(0);
            }
            else if(i_reg == 0x02)                  //��ײ����
            {        
                kputchar('(');                      //����(z)
                kputchar('z');
                kputchar(')');
                return(0);
            }
            else if(i_reg == 0x00)                  //��ʱʱ�䵽
            {   
                kputchar('(');
                kputchar(')');
                return(1);
            }
            else
            ;
            break;
        }   /* switch */
    }
    
    IRQOFF();                                       //�ر��ж�
    return(1);                                      //����ȫ��ִ����ϣ����� 1
}   /* RequestCommand */



