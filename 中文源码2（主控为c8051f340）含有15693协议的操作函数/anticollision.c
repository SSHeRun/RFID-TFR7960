/***********************************************************************************************************************
* �� �� ����ANTICOLLISION.C
* ��    �ܣ�ISO15693Э�鿨Ƭ���������������³�ײ����ȡ�
*           ���ļ�������ISO15693Э�����ʾ������
*           ע�⣺�ڴ���ͽ��չ����У�����Ҫͨ��FIFO������
* ��    �ߣ�EMDOOR
* ��    �ڣ�2011-9-29
*************************************************************************************************************************/
#include <anticollision.h>

#ifndef  DBG
#define DBG  0
#define LEDOFF    P2 = 0X04
#define LEDON     P2 = 0XFB
unsigned char Found_tag;                            //�����Ƿ��⵽��Ƭȫ�ֱ���
unsigned char rssival;                              //�����⵽�Ŀ�Ƭ�����ź�ǿ��ֵ

//C8051F��STM32��ͨ��Э�����ݽṹ�Ķ���

//----------------------------------------------------------------
//*����֡
//----------------------------------------------------------------
struct uartsend
{
  
  unsigned char header;//֡ͷ

  unsigned char length;//֡��

  unsigned char seq;//֡���к�

  unsigned char format;//֡��ʽ

  unsigned char tag_it;//��ǩЭ��

  unsigned char id_data[8];//����

  unsigned char par_bit;//У��λ

  unsigned char tail;//֡β
  
} uartsend_protocol;

//----------------------------------------------------------------


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
    buf[41] |= 0X01;                                //�ڻ������Ĵ���0x41λ������BIT0��Ч
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
    unsigned char i = 1, j=3, command[2], NoSlots;
    unsigned char *PslotNo, slotNo[17];
    unsigned char NewMask[8], NewLenght, masksize;
    int size;
//		int counter,temp;
    unsigned int k = 0;

    buf[0] = ModulatorControl;                      // ���ƺ�ϵͳʱ�ӿ��ƣ�0x21 - 6.78MHz OOK(100%)
    buf[1] = 0x21;
    WriteSingle(buf, 2);
 
 /* ���ʹ��SPI����ģʽ�ĵ������ʣ���ô RXNoResponseWaitTime ��Ҫ���������� */
/*====================================================================================================*/
  
        if((flags & 0x02) == 0x00)                  //�����ݱ�����
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
 /*====================================================================================================*/
    
    slotNo[0] = 0x00;

    if((flags & 0x20) == 0x00)                      //λ5��־λָʾ�۵�����
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

    Timer0_Delay(20);                    //��ʱʱ��Ϊ 20ms
    IRQCLR();                                       //���жϱ�־λ
    IRQON();                                        //�жϿ���

    RAWwrite(&buf[0], masksize + 8);                //������д�뵽FIFO��������

    i_reg = 0x01;                                   //�����жϱ�־ֵ
    StartCounter();                                   //��ʼ�Ե���ģʽ��ʱ
    PCON |=0X01;                                           //�ȴ�TX���ͽ���

    for(i = 1; i < NoSlots; i++)                    //Ѱ��ѭ��1���ۻ���16����
    {       
        /* ��ʼ��ȫ�ּ����� */
        /*====================================================================================================*/
        RXTXstate = 1;                              //���ñ�־λ�������λ�洢��buf[1]��ʼλ��
        Timer0_Delay(20);                //��ʱʱ��Ϊ 20ms                      
        StartCounter();                               //��ʼ�Ե���ģʽ��ʱ
        k = 0;
        PCON |=0X01;
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
			
//					Found_tag = 1��
					int counter,temp;
					LEDON;
					uartsend_protocol.header = 0X5A;
					uartsend_protocol.length = 0X15;
					uartsend_protocol.seq = 0x00;
					uartsend_protocol.format = 0x01;
					uartsend_protocol.tag_it = 0x02;
				 
								for(j = 10,counter=0; j>=3,counter<8; j--,counter++)
										{
												uartsend_protocol.id_data[counter] = buf[j];               //����ISO15693 UID��
										}

					uartsend_protocol.par_bit = uartsend_protocol.header^uartsend_protocol.length;
						uartsend_protocol.par_bit = uartsend_protocol.par_bit^uartsend_protocol.seq;
					uartsend_protocol.par_bit = uartsend_protocol.par_bit^uartsend_protocol.format;
						uartsend_protocol.par_bit = uartsend_protocol.par_bit^uartsend_protocol.tag_it;
					for(temp=0;temp<8;temp++)
					{
					uartsend_protocol.par_bit = uartsend_protocol.par_bit^uartsend_protocol.id_data[temp];
					}
					uartsend_protocol.tail = 0xA5;

					sendchar(uartsend_protocol.header);
					sendchar(uartsend_protocol.length);
						sendchar(uartsend_protocol.seq);
					sendchar(uartsend_protocol.format);
					sendchar(uartsend_protocol.tag_it);

					for(j=0;j<8;j++)
					{
					send_byte(uartsend_protocol.id_data[j]);
					}
					sendchar(uartsend_protocol.par_bit);
					sendchar(uartsend_protocol.tail);
					send_crlf();
		#if  DBG
            sendchar(',');
            send_byte(command[0]);               //����RSSI�����ź�ǿ��
            sendchar(']');
			
		#endif
		
			delay_ms(30) ;			
			LEDOFF;
	   
	    }
        else if(i_reg == 0x02)                      //����г�ײ����
        {  
		#if DBG
                sendchar('[');
                sendchar('z');                      //���� z
                sendchar(',');
                send_byte(command[0]);               //����RSSI�����ź�ǿ��
                sendchar(']');

		#endif
                PslotNo++;
               *PslotNo = i;

			
        }
        else if(i_reg == 0x00)                      //�����ʱʱ�䵽���жϷ���
        { 

		#if DBG           
               sendchar('[');
               sendchar(',');
               send_byte(command[0]);               //����RSSI�����ź�ǿ��
               sendchar(']'); 
		#endif			         
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
    }   /* for */

    NewLenght = lenght + 4;                         //��ǳ���Ϊ4����λ����
    masksize = (((NewLenght >> 2) + 1) >> 1) - 1;

    /* �����16����ģʽ������ָ�벻Ϊ0x00, ��ݹ���ñ��������ٴ�Ѱ�ҿ�Ƭ */
    /*====================================================================================================*/
    while((*PslotNo != 0x00) && (NoSlots == 17)) 
    {		
        *PslotNo = *PslotNo - 1;
        for(i = 0; i < 8; i++) NewMask[i] = *(mask + i);            //���Ƚ����ֵ�������±��������

        if((NewLenght & 0x04) == 0x00) *PslotNo = *PslotNo << 4;

        NewMask[masksize] |= *PslotNo;                              //���ֵ���ı�
        InventoryRequest1(&NewMask[0], NewLenght);                   //�ݹ���� InventoryRequest ����
        PslotNo--;     
	                                            //�۵ݼ�
    }   /* while */
    /*====================================================================================================*/
    
    IRQOFF();    
	                                               //�³�ײ���̽������ر��ж�
}   /* InventoryRequest */

/******************************************************************************************************************
* �������ƣ�InventoryRequest1()
* ��    �ܣ�ISO15693Э�鿨Ƭ�����������
* ��ڲ�����*mask       �������
*           lenght      �����
* ���ڲ�������     
* ˵    ����ִ�иú�������ʹISO15693Э���׼��������ѭ��16ʱ��ۻ���1��ʱ���.
*           ���У�0x14��ʾ16�ۣ�0x17��ʾ1���ۡ�
*           ע�⣺���ѻ�ģʽ�£����յ�UID�뽫����ʾ��LCMͼ����ʾ���ϡ�
*******************************************************************************************************************/
void InventoryRequest1(unsigned char *mask, unsigned char lenght)
{
    unsigned char i = 1, j=3, command[2], NoSlots;
    unsigned char *PslotNo, slotNo[17];
    unsigned char NewMask[8], NewLenght, masksize;
    int size;
//		int counter,temp;
    unsigned int k = 0;

    buf[0] = ModulatorControl;                      // ���ƺ�ϵͳʱ�ӿ��ƣ�0x21 - 6.78MHz OOK(100%)
    buf[1] = 0x21;
    WriteSingle(buf, 2);
 
 /* ���ʹ��SPI����ģʽ�ĵ������ʣ���ô RXNoResponseWaitTime ��Ҫ���������� */
/*====================================================================================================*/
  
        if((flags & 0x02) == 0x00)                  //�����ݱ�����
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
 /*====================================================================================================*/
    
    slotNo[0] = 0x00;

    if((flags & 0x20) == 0x00)                      //λ5��־λָʾ�۵�����
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

    Timer0_Delay(20);                    //��ʱʱ��Ϊ 20ms
    IRQCLR();                                       //���жϱ�־λ
    IRQON();                                        //�жϿ���

    RAWwrite(&buf[0], masksize + 8);                //������д�뵽FIFO��������

    i_reg = 0x01;                                   //�����жϱ�־ֵ
    StartCounter();                                   //��ʼ�Ե���ģʽ��ʱ
    PCON |=0X01;                                           //�ȴ�TX���ͽ���

    for(i = 1; i < NoSlots; i++)                    //Ѱ��ѭ��1���ۻ���16����
    {       
        /* ��ʼ��ȫ�ּ����� */
        /*====================================================================================================*/
        RXTXstate = 1;                              //���ñ�־λ�������λ�洢��buf[1]��ʼλ��
        Timer0_Delay(20);                //��ʱʱ��Ϊ 20ms                      
        StartCounter();                               //��ʼ�Ե���ģʽ��ʱ
        k = 0;
        PCON |=0X01;
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
			
//					Found_tag = 1��
					int counter,temp;
					LEDON;
					uartsend_protocol.header = 0X5A;
					uartsend_protocol.length = 0X15;
					uartsend_protocol.seq = 0x00;
					uartsend_protocol.format = 0x01;
					uartsend_protocol.tag_it = 0x02;
				 
								for(j = 10,counter=0; j>=3,counter<8; j--,counter++)
										{
												uartsend_protocol.id_data[counter] = buf[j];               //����ISO15693 UID��
										}

					uartsend_protocol.par_bit = uartsend_protocol.header^uartsend_protocol.length;
						uartsend_protocol.par_bit = uartsend_protocol.par_bit^uartsend_protocol.seq;
					uartsend_protocol.par_bit = uartsend_protocol.par_bit^uartsend_protocol.format;
						uartsend_protocol.par_bit = uartsend_protocol.par_bit^uartsend_protocol.tag_it;
					for(temp=0;temp<8;temp++)
					{
					uartsend_protocol.par_bit = uartsend_protocol.par_bit^uartsend_protocol.id_data[temp];
					}
					uartsend_protocol.tail = 0xA5;

					sendchar(uartsend_protocol.header);
					sendchar(uartsend_protocol.length);
						sendchar(uartsend_protocol.seq);
					sendchar(uartsend_protocol.format);
					sendchar(uartsend_protocol.tag_it);

					for(j=0;j<8;j++)
					{
					send_byte(uartsend_protocol.id_data[j]);
					}
					sendchar(uartsend_protocol.par_bit);
					sendchar(uartsend_protocol.tail);
					send_crlf();
		#if  DBG
            sendchar(',');
            send_byte(command[0]);               //����RSSI�����ź�ǿ��
            sendchar(']');
			
		#endif
		
			delay_ms(30) ;			
			LEDOFF;
	   
	    }
        else if(i_reg == 0x02)                      //����г�ײ����
        {  
		#if DBG
                sendchar('[');
                sendchar('z');                      //���� z
                sendchar(',');
                send_byte(command[0]);               //����RSSI�����ź�ǿ��
                sendchar(']');

		#endif
                PslotNo++;
               *PslotNo = i;

			
        }
        else if(i_reg == 0x00)                      //�����ʱʱ�䵽���жϷ���
        { 

		#if DBG           
               sendchar('[');
               sendchar(',');
               send_byte(command[0]);               //����RSSI�����ź�ǿ��
               sendchar(']'); 
		#endif			         
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
    }   /* for */

    NewLenght = lenght + 4;                         //��ǳ���Ϊ4����λ����
    masksize = (((NewLenght >> 2) + 1) >> 1) - 1;

    /* �����16����ģʽ������ָ�벻Ϊ0x00, ��ݹ���ñ��������ٴ�Ѱ�ҿ�Ƭ */
    /*====================================================================================================*/
    while((*PslotNo != 0x00) && (NoSlots == 17)) 
    {		
        *PslotNo = *PslotNo - 1;
        for(i = 0; i < 8; i++) NewMask[i] = *(mask + i);            //���Ƚ����ֵ�������±��������

        if((NewLenght & 0x04) == 0x00) *PslotNo = *PslotNo << 4;

        NewMask[masksize] |= *PslotNo;                              //���ֵ���ı�
        InventoryRequest(&NewMask[0], NewLenght);                   //�ݹ���� InventoryRequest ����
        PslotNo--;     
	                                            //�۵ݼ�
    }   /* while */
    /*====================================================================================================*/
    
    IRQOFF();    
	                                               //�³�ײ���̽������ر��ж�
}   /* InventoryRequest */

/******************************************************************************************************************
* ????:RequestCommand()
* ?    ?:????????????????????????
* ????:*pbuf           ???
*           lenght          ????
*           brokenBits      ?????????
*           noCRC           ???CRC??
* ????:1     
* ?    ?:??????????,???1,??????????,???0?????,????
*******************************************************************************************************************/
unsigned char RequestCommand(unsigned char *pbuf, unsigned char lenght, unsigned char brokenBits, char noCRC)
{
    unsigned char index, j, command;                //????
    unsigned char temp2;
    
    RXTXstate = lenght;                             

    *pbuf = 0x8f;
    if(noCRC) 
        *(pbuf + 1) = 0x90;                         //????CRC??
    else
        *(pbuf + 1) = 0x91;                         //???CRC??
    
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

    RAWwrite(pbuf, lenght + 5);                     //????FIFO??????

    IRQCLR();                                       //??????
    IRQON();

    RXTXstate = RXTXstate - 12;
    index = 17;

    i_reg = 0x01;
    while(RXTXstate > 0)
    {
//        LPM0;                                       //???????,?????
			 PCON |=0X01;                                           //�ȴ�TX���ͽ���
        if(RXTXstate > 9)                           //?RXTXstate?????????????????9
        {                       
            lenght = 10;                            //???10,????FIFO??9????1???????
        }
        else if(RXTXstate < 1)                      //??????1,?????????????FIFO?,??????
        {
            break;
        }
        else                                        //???????????
        {
            lenght = RXTXstate + 1;         
        }   /* if */

        buf[index - 1] = FIFO;                      //?FIFO??????9??????????,?????
        WriteCont(&buf[index - 1], lenght);
        RXTXstate = RXTXstate - 9;                  //?9????FIFO?
        index = index + 9;
    }   /* while */

    RXTXstate = 1;                                  //?????,???????buf[1]????

    while(i_reg == 0x01)                            //??????
    {
//        CounterSet();                               //?????
				Timer0_Delay(60);
//        CountValue = 0xF000;                        //???? 60ms
//        StartCounter();                               //?????????
//        LPM0;
			 PCON |=0X01;                                           //�ȴ�TX���ͽ���
    }

    i_reg = 0x01;
		Timer0_Delay(60);
//    CounterSet();                                   //?????
//    CountValue = 0xF000;                            //???? 60ms
    StartCounter();                                   //?????????

    /* ?????????,???????????? */
    /*====================================================================================================*/
    if((((buf[5] & 0x06) == 0x06) && ((buf[6] == 0x21) || (buf[6] == 0x24) || (buf[6] == 0x27) || (buf[6] == 0x29)))
    || (buf[5] == 0x00 && ((buf[6] & 0xF0) == 0x20 || (buf[6] & 0xF0) == 0x30 || (buf[6] & 0xF0) == 0x40)))
    {
        delay_ms(20);
        command = Reset;
        DirectCommand(&command);
        command = TransmitNextSlot;
        DirectCommand(&command);
    }   /* if */
    /*====================================================================================================*/
    
    while(i_reg == 0x01)                            //??????
    { 
    }
    
//    if(POLLING)                                     //???????????????
//    {
//        if(Found_tag)
//        {
//            if(i_reg == 0xFF)                       //?????
//            {
//                if((buf[1]) == 0x00)                //???? 
//                {
//                    Display_Char(6, 13, (unsigned char *)CO);           //??OK
//                    Display_Char(6, 14, (unsigned char *)CK);
//                    Beep_Waring(1, Beep15693); 
//                }
//                else                                //????
//                {
//                    Display_Char(6,10,(unsigned char *)Num+0x0E*0x10);  //??ERROR
//                    Display_Char(6, 11, (unsigned char *)CR);
//                    Display_Char(6, 12, (unsigned char *)CR);
//                    Display_Char(6, 13, (unsigned char *)CO);
//                    Display_Char(6, 14, (unsigned char *)CR);
//                    Beep_Waring(3, Beep15693);      //?????????
//                }
//            }
//        }
//        
//          switch(test_no)
//        {
//            case 0:                                 //????
//                break;
//            case 1:                                 //????
                for(j = 2; j < RXTXstate; j++)
                {   //???????
                    send_byte(buf[j]);
                }                                   
//                break;
//            case 2:                                 //?AFI
//                break;
//            case 3:                                 //?DSFID
//                break;
//            case 4:                                 //??????
//                for(j = 11, temp2 = 9; j < RXTXstate - 1; j++, temp2 += 2)
//                {   //????????????
//                    Display_Char(2, temp2-1, (unsigned char *)(Num + (buf[j] >> 4) * 0x10));
//                    Display_Char(2, temp2, (unsigned char *)(Num+(buf[j] & 0x0F) * 0x10));
//                }                                 
//                break;
//            case 5:                                 //???????
//                 for(j = 2, temp2 = 15; j < RXTXstate; j++, temp2 -= 2)
//                {   //?????????
//                    Display_Char(2, temp2 - 1, (unsigned char *)(Num + (buf[j] >> 4) * 0x10));
//                    Display_Char(2, temp2, (unsigned char *)(Num+(buf[j] & 0x0F) * 0x10));
//                }                                   
//                break;
//            default:break;
//        }
//    }
    
  
    
//     if(!POLLING)
//    {
//        switch(noCRC)                               //????CRC??
//        {
//            case 0:
//            if(i_reg == 0xFF)                       //?????
//            {       
//                kputchar('[');                      //??[]
//                for(j = 1; j < RXTXstate; j++)
//                {
//                    Put_byte(buf[j]);
//                }   /* for */

//                kputchar(']');
//                return(0);
//            }
//            else if(i_reg == 0x02)                  //????
//            {      
//                kputchar('[');                      //??[z]
//                kputchar('z');
//                kputchar(']');
//                return(0);
//            }
//            else if(i_reg == 0x00)                  //?????
//            {      
//                kputchar('[');
//                kputchar(']');
//                return(1);
//            }
//            else
//            ;
//            break;

//            case 1:
//            if(i_reg == 0xFF)                       //?????
//                        {
//                kputchar('(');                      //??()
//                for(j = 1; j < RXTXstate; j++)
//                {
//                    Put_byte(buf[j]);
//                }   /* for */

//                kputchar(')');
//                return(0);
//            }
//            else if(i_reg == 0x02)                  //????
//            {        
//                kputchar('(');                      //??(z)
//                kputchar('z');
//                kputchar(')');
//                return(0);
//            }
//            else if(i_reg == 0x00)                  //?????
//            {   
//                kputchar('(');
//                kputchar(')');
//                return(1);
//            }
//            else
//            ;
//            break;
//        }   /* switch */
//    }
    
    IRQOFF();                                       //????
    return(1);                                      //????????,?? 1
}   /* RequestCommand */
