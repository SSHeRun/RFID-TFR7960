/********************************************************************************************************
* �� �� ����COMMUNICATE.H
* ��    �ܣ�RFID�Ķ���TRF7960��C8051F340΢������֮��ͨ�ŷ�ʽͷ�ļ���
* Ӳ�����ӣ�C8051F340��TRF7960֮��ͨ��Ӳ�����ӹ�ϵ������ʾ��
*                C8051F340                 TRF7960
*********************    PARALLEL INTERFACE    ******************************************         
*               P0.7   				 IRQ
*			    P0.3                 Slave_select
*               P0.2                 SIMO
*               P0.1                 SOMI
*               P0.0                 DATA_CLK
*				P4.0		       	 MOD
*				P4.2				 ASK/OOK
*				P4.3				 EN
*
* ��    ����V1.0
* ��    �ߣ�EMDOOR
* ��    �ڣ�2011-9-29
*********************************************************************************************************/
#include <communicate.h>
#include <globals.h>
#include <hardware.h>
#include <c8051f340.h>
#define DBG  0
static unsigned char temp;
static unsigned int mask = 0x80;

/******************************************************************************************************************
* �������ƣ�WriteSingle()
* ��    �ܣ�д�����Ĵ������������ַ�Ķ���Ĵ�������
* ��ڲ�����*pbuf            ��Ҫд�������           
*           lenght           д�����ݵĳ��� 
* ���ڲ�������
* ˵    ����д���
******************************************************************************************************************/
void WriteSingle(unsigned char *pbuf, unsigned char lenght)
{
	unsigned char i,j;
    /*  SPI λģʽ */
        /*-----------------------------------------------------------------------------*/
        L_SlaveSelect();                            //SS�ܽ�����ͣ�SPI����
        CLKOFF();                                   //CLKʱ�ӹرգ��ͣ�
        while(lenght > 0)
        {
            *pbuf = (0x1f & *pbuf);                 //ȡ��5λB0-B4 �Ĵ�����ַ���� ��ʽΪ000XXXXX
            for(i = 0; i < 2; i++)                  //�Ե����Ĵ������ȷ��͵�ַ���ٷ������ݻ�����
            {
                for(j = 0; j < 8; j++)
                {
                    if (*pbuf & mask)               //��������λ
                        SIMOON();
                    else
                        SIMOOFF();
                   
                    CLKON();                        //����CLKʱ����Ϣ����������
                    CLKOFF();
                    mask >>= 1;                     //��־λ����
                }   /* for */
                mask = 0x80;
                pbuf++;
                lenght--;
            }/*for*/
        } /*while*/

        H_SlaveSelect();                            //SS�ܽ�����ߣ�SPIֹͣ
        /*-----------------------------------------------------------------------------*/
}   /* WriteSingle */

/******************************************************************************************************************
* �������ƣ�WriteCont()
* ��    �ܣ�����д�Ĵ������������ַ�Ķ���Ĵ�������
* ��ڲ�����*pbuf            ��Ҫд�������           
*           lenght           д�����ݵĳ��� 
* ���ڲ�������
* ˵    ����������ַд���
******************************************************************************************************************/
void WriteCont(unsigned char *pbuf, unsigned char lenght)
{
    
    unsigned char j;
     
    /*====================================================================================================*/     
    /* ����(SPI)ģʽͨ�� */
    /*====================================================================================================*/
		/*-----------------------------------------------------------------------------*/
		 /* Ӳ��SPIģʽ */
		/*-----------------------------------------------------------------------------*/
		L_SlaveSelect();                             //SS�ܽ�����ͣ�SPI����
		CLKOFF();                                   //CLKʱ�ӹرգ��ͣ�

		*pbuf = (0x20 | *pbuf);                     //ȡλB5 �Ĵ�����ַ���� ������־λ ��ʽΪ001XXXXX
		*pbuf = (0x3f &*pbuf);                      //ȡ��6λB0-B5 �Ĵ�����ַ����
		while(lenght > 0)
		{
				for(j=0;j<8;j++)
				{
						if (*pbuf & mask)                    //��������λ
								SIMOON();
						else
								SIMOOFF();

						CLKON();                             //����CLKʱ����Ϣ����������
						CLKOFF();
						mask >>= 1;                        //��־λ����
				}/*for*/

				mask = 0x80;                            
				pbuf++;
				lenght--;
		}/*while*/

		H_SlaveSelect();                            //SS�ܽ�����ߣ�SPIֹͣ
		/*-----------------------------------------------------------------------------*/
}   /* WriteCont */

/******************************************************************************************************************
* �������ƣ�ReadSingle()
* ��    �ܣ��������Ĵ���
* ��ڲ�����*pbuf            ��Ҫ��ȡ������           
*           lenght           ��ȡ���ݵĳ��� 
* ���ڲ�������
* ˵    ������
******************************************************************************************************************/
void ReadSingle(unsigned char *pbuf, unsigned char lenght)
{
 unsigned char j;
		 /*  SPI λģʽ */
        /*-----------------------------------------------------------------------------*/
        L_SlaveSelect();                            //SS�ܽ�����ͣ�SPI����

        while(lenght > 0)
        {
            *pbuf = (0x40 | *pbuf);                 //ȡλB6 �Ĵ�����ַ���� ������־λ ��ʽΪ01XXXXXX
            *pbuf = (0x5f &*pbuf);                  //ȡ��7λB0-B6 �Ĵ�����ַ����
            for(j = 0; j < 8; j++)
            {
                if (*pbuf & mask)                   //��������λ
                    SIMOON();
                else
                    SIMOOFF();
                
                CLKON();                            //����CLKʱ����Ϣ����������
                CLKOFF();
                mask >>= 1;                         //��־λ����
            }   /* for */
            mask = 0x80;

            *pbuf = 0;                              //��ʼ��ȡ����
            for(j = 0; j < 8; j++)
            {
                *pbuf <<= 1;                        //��������
                CLKON();                            //����CLKʱ����Ϣ����������
                CLKOFF();

                if (SOMISIGNAL)                     //�ж�SOMI����
                    *pbuf |= 1;                     //��Ϊ�ߵ�ƽ�������ݻ��� 1
            }   /* for */

            pbuf++;
            lenght--;
        } /* while */

        H_SlaveSelect();                            //SS�ܽ�����ߣ�SPIֹͣ
        /*-----------------------------------------------------------------------------*/
}   /* ReadCont */

/******************************************************************************************************************
* �������ƣ�ReadCont()
* ��    �ܣ��������Ĵ������������ַ�Ķ���Ĵ�������
* ��ڲ�����*pbuf            ��Ҫ��ȡ������           
*           lenght           ��ȡ���ݵĳ��� 
* ���ڲ�������
* ˵    ����������ַд���
******************************************************************************************************************/
void ReadCont(unsigned char *pbuf, unsigned char lenght)
{
    unsigned char j;
	/*  SPI λģʽ */
        /*-----------------------------------------------------------------------------*/
        L_SlaveSelect();                            //SS�ܽ�����ͣ�SPI����
        *pbuf = (0x60 | *pbuf);                     //ȡλB6B5 �Ĵ�����ַ���� ������־λ ��ʽΪ011XXXXX
        *pbuf = (0x7f & *pbuf);                     //ȡ��7λB0-B6 �Ĵ�����ַ����

        for(j = 0; j < 8; j++)                      //д�Ĵ�����ַ
        {
            if (*pbuf & mask)                       //��������λ
                SIMOON();
            else
                SIMOOFF();
     
            CLKON();                                //����CLKʱ����Ϣ����������
            CLKOFF();
            mask >>= 1;                             //��־λ����
        }/*for*/
        mask = 0x80;

        while(lenght > 0)                           //��ʼ��ȡ����
        {
            *pbuf = 0;                              //��ջ�����
            for(j = 0; j < 8; j++)
            {
                *pbuf <<= 1;                        //��������
                CLKON();                            //����CLKʱ����Ϣ����������
                CLKOFF();
                if (SOMISIGNAL)
                *pbuf |= 1;
            }/*for*/

            pbuf++;                                 //����������
            lenght--;
        }/* while */

        H_SlaveSelect();                            //SS�ܽ�����ߣ�SPIֹͣ
		
}

/******************************************************************************************************************
* �������ƣ�DirectCommand()
* ��    �ܣ�ֱ������ɷ���һ������Ķ���оƬ
* ��ڲ�����*pbuf            ��Ҫ���͵���������           
* ���ڲ�������
* ˵    ����ֱ�����
******************************************************************************************************************/
void DirectCommand(unsigned char *pbuf)
{
	unsigned char j;
  /*  SPI λģʽ */
        /*-----------------------------------------------------------------------------*/
        L_SlaveSelect();                            //SS�ܽ�����ͣ�SPI����
        *pbuf = (0x80 | *pbuf);                     //ȡλB7 �Ĵ�����ַ���� �����־λ ��ʽΪ1XXXXXXX
        *pbuf = (0x9f & *pbuf);                     //����ֵ

        for(j = 0; j < 8; j++)                      //д�Ĵ�����ַ
        {
            if (*pbuf & mask)                       //��������λ
                SIMOON();
            else
                SIMOOFF();

            CLKON();                                //����CLKʱ����Ϣ����������
            CLKOFF();
            mask >>= 1;                             //��־λ����
        }   /* for */
        mask = 0x80;

        CLKON();                                    //���Ӷ���ʱ������
        CLKOFF();
        H_SlaveSelect();                            //SS�ܽ�����ߣ�SPIֹͣ
        /*-----------------------------------------------------------------------------*/

}   /* DirectCommand */


/******************************************************************************************************************
* �������ƣ�RAWwrite()
* ��    �ܣ�ֱ��д���ݻ�����Ķ���оƬ
* ��ڲ�����*pbuf           ��Ҫ���͵���������    
*           lenght          д�����ݻ�����ĳ���    
* ���ڲ�������
* ˵    ����ֱ��д��
******************************************************************************************************************/
void RAWwrite(unsigned char *pbuf, unsigned char lenght)
{
	 unsigned char j;
         /*  SPI λģʽ */
        /*-----------------------------------------------------------------------------*/
        L_SlaveSelect();                            //SS�ܽ�����ͣ�SPI����
        while(lenght > 0)
        {
            for(j = 0; j < 8; j++)                  //д�Ĵ�����ַ
            {
                if (*pbuf & mask)                   //��������λ
                    SIMOON();
                else
                    SIMOOFF();

                CLKON();                            //����CLKʱ����Ϣ����������
                CLKOFF();
                mask >>= 1;                         //��־λ����
            }   /*for*/
            mask = 0x80;
 
            pbuf++;
            lenght--;
        }   /* while */

        H_SlaveSelect();                            //SS�ܽ�����ߣ�SPIֹͣ

}   /* RAWwrite */

/******************************************************************************************************************
* �������ƣ�InitialSettings()
* ��    �ܣ���ʼ��TRF7960����
* ��ڲ������� 
* ���ڲ�������
* ˵    ��������Ƶ��������������
******************************************************************************************************************/
void InitialSettings(void)
{
    unsigned char command[2];

    command[0] = ModulatorControl;                  
    //command[1] = 0x21;                              // ���ƺ�ϵͳʱ�ӿ��ƣ�0x21 - 6.78MHz OOK(100%)
    command[1] = 0x31;
	WriteSingle(command,2);

}

/******************************************************************************************************************
* �������ƣ�InterruptHandlerReader()
* ��    �ܣ��Ķ����жϴ������
* ��ڲ�����*Register           �ж�״̬�Ĵ��� 
* ���ڲ�������
* ˵    ���������ⲿ�жϷ������
*           IRQ�ж�״̬�Ĵ���˵�����£�
*
*   λ      λ����      ����                ˵��
*   B7      Irq_tx      TX������IRQ��λ         ָʾTX���ڴ����С��ñ�־λ��TX��ʼʱ�����ã������ж���������TX���ʱ�ŷ��͡�
*   B6      Irq_srx     RX��ʼ��IRQ��λ         ָʾRX SOF�Ѿ������յ�����RX���ڴ����С��ñ�־λ��RX��ʼʱ�����ã������ж���������RX���ʱ�ŷ��͡�
*   B5      Irq_fifo    ָʾFIFOΪ1/3>FIFO>2/3      ָʾFIFO�߻��ߵͣ�С��4���ߴ���8����
*   B4      Irq_err1    CRC����             ����CRC
*   B3      Irq_err2    ��żУ�����                    (��ISO15693��Tag-itЭ����δʹ��)
*   B2      Irq_err3    �ֽڳ�֡����EOF���� 
*   B1      Irq_col     ��ײ����            ISO14443A��ISO15693�����ز���
*   B0      Irq_noresp  ����Ӧ�ж�          ָʾMCU���Է�����һ�������
******************************************************************************************************************/
void InterruptHandlerReader(unsigned char *Register)
{
    unsigned char len;

#if DBG
    send_byte(*Register);                            //������ڲ���ֵ
#endif

    if(*Register == 0xA0)                           //A0 = 10100000 ָʾTX���ͽ�����������FIFO��ʣ��3�ֽ�����
    {                
        i_reg = 0x00;
#if DBG
        sendchar('.');                              //�ڴ��͹�����FIFO�Ѿ������
#endif
    }

    else if(*Register == 0x80)                      //BIT7 = 10000000 ָʾTX���ͽ���
    {            
        i_reg = 0x00;
        *Register = Reset;                          //��TX���ͽ����� ִ�и�λ����
        DirectCommand(Register);
#if DBG
        sendchar('T');                              //TX���ͽ���
#endif
    }

    else if((*Register & 0x02) == 0x02)             //BIT1 = 00000010 ��ײ����
    {                           
        i_reg = 0x02;                               //����RX����

        *Register = StopDecoders;                   //��TX���ͽ�����λFIFO
        DirectCommand(Register);

        CollPoss = CollisionPosition;
        ReadSingle(&CollPoss, 1);

        len = CollPoss - 0x20;                      //��ȡFIFO�е���Ч�����ֽ�����
#if DBG
        sendchar('{');
        send_byte(CollPoss);                     //���ͳ�ײ������λ��
        sendchar('}');
#endif     
        
        if((len & 0x0f) != 0x00) 
            len = len + 0x10;                       //������յ��������ֽڣ������һ���ֽ�
        len = len >> 4;

        if(len != 0x00)
        {
            buf[RXTXstate] = FIFO;                  //�����յ�������д���������ĵ�ǰλ��                               
            ReadCont(&buf[RXTXstate], len);
            RXTXstate = RXTXstate + len;
        }   /* if */

        *Register = Reset;                          //ִ�и�λ����
        DirectCommand(Register);

        *Register = IRQStatus;                      //��ȡIRQ�ж�״̬�Ĵ�����ַ
        *(Register + 1) = IRQMask;

            ReadCont(Register, 2);
   
        IRQCLR();                                   //���ж�
    }
    else if(*Register == 0x40)                      //BIT6 = 01000000 ���տ�ʼ
    {   
        if(RXErrorFlag == 0x02)                     //RX���ձ�־λָʾEOF�Ѿ������գ�����ָʾ��FIFO�Ĵ�����δ�����ֽڵ�����
        {
            i_reg = 0x02;
            return;
        }

        *Register = FIFOStatus;
        ReadSingle(Register, 1);                    //��ȡ��FIFO��ʣ���ֽڵ�����
        *Register = (0x0F & *Register) + 0x01;
        buf[RXTXstate] = FIFO;                      //�����յ�������д���������ĵ�ǰλ��
                                                                                	
        ReadCont(&buf[RXTXstate], *Register);
        RXTXstate = RXTXstate +*Register;

        *Register = TXLenghtByte2;                  //��ȡ�Ƿ��в��������ֽڼ���λ����
        ReadCont(Register, 1);

        if((*Register & 0x01) == 0x01)              //00000001 ����Ӧ�ж�
        {
            *Register = (*Register >> 1) & 0x07;    //���ǰ5λ
            *Register = 8 - *Register;
            buf[RXTXstate - 1] &= 0xFF << *Register;
        }   /* if */
         
#if DBG
        sendchar('E');                              //��������Ӧ��־ E
#endif
        *Register = Reset;                          //���һ���ֽڱ���ȡ��λFIFO
        DirectCommand(Register);

        i_reg = 0xFF;                               //ָʾ���պ�����Щ�ֽ��Ѿ�������ֽ�
    }
    else if(*Register == 0x60)                      //0x60 = 01100000 RX�Ѿ���� ������9���ֽ���FIFO��
    {                            
        i_reg = 0x01;                               //���ñ�־λ
        buf[RXTXstate] = FIFO;
        ReadCont(&buf[RXTXstate], 9);               //��FIFO�ж�ȡ9���ֽ�����
        RXTXstate = RXTXstate + 9;
#if DBG
        sendchar('F');                              //���� F ��ʾFIFO��������
#endif
        if(IRQPORT & IRQPin)                        //����жϹܽ�Ϊ�ߵ�ƽ
        {
            *Register = IRQStatus;                  //��ȡIRQ�ж�״̬�Ĵ�����ַ
            *(Register + 1) = IRQMask;
                            //��ȡ�Ĵ���
                ReadCont(Register, 2);
         
            IRQCLR();                               //���ж�

            if(*Register == 0x40)                   //0x40 = 01000000 ���ս���
            {  
                *Register = FIFOStatus;
                ReadSingle(Register, 1);            //��ȡ��FIFO��ʣ���ֽڵ�����
                *Register = 0x0F & (*Register + 0x01);
                buf[RXTXstate] = FIFO;              //�����յ�������д���������ĵ�ǰλ��
                                                                                	
                ReadCont(&buf[RXTXstate], *Register);
                RXTXstate = RXTXstate +*Register;

                *Register = TXLenghtByte2;          //��ȡ�Ƿ��в��������ֽڼ���λ����
                ReadSingle(Register, 1);         

                if((*Register & 0x01) == 0x01)      //00000001 ����Ӧ�ж�
                {
                    *Register = (*Register >> 1) & 0x07;            //���ǰ5λ
                    *Register = 8 -*Register;
                    buf[RXTXstate - 1] &= 0xFF << *Register;
                }   /* if */
#if DBG
                sendchar('E');                      //��������Ӧ��־ E
#endif
                i_reg = 0xFF;                       //ָʾ���պ�����Щ�ֽ��Ѿ�������ֽ�
                *Register = Reset;                  //������ֽڱ���ȡ��λFIFO
                DirectCommand(Register);
            }
            else if(*Register == 0x50)              //0x50 = 01010000���ս������ҷ���CRC����
            {        
                i_reg = 0x02;
#if DBG
                sendchar('x');                      //����CRCУ������־ x
#endif
            }
        }   /* if(IRQPORT & IRQPin) */
        else                                        
        {
            Register[0] = IRQStatus;                //��ȡIRQ�ж�״̬�Ĵ�����ַ
            Register[1] = IRQMask;
            
                ReadCont(Register, 2);              //��ȡ�Ĵ���
          
            
            if(Register[0] == 0x00) 
              i_reg = 0xFF;                         //ָʾ���պ�����Щ�ֽ��Ѿ�������ֽ�
        }
    }
    else if((*Register & 0x10) == 0x10)             //BIT4 = 00010000 ָʾCRC����
    {                      
        if((*Register & 0x20) == 0x20)              //BIT5 = 00100000 ָʾFIFO����9���ֽ�
        {
            i_reg = 0x01;                           //�������
            RXErrorFlag = 0x02;
        }
        else
            i_reg = 0x02;                           //ֹͣ����        
    }
    else if((*Register & 0x04) == 0x04)             //BIT2 = 00000100  �ֽڳ�֡����EOF����
    {                       
        if((*Register & 0x20) == 0x20)              //BIT5 = 00100000 ָʾFIFO����9���ֽ�
        {
            i_reg = 0x01;                           //�������
            RXErrorFlag = 0x02;
        }
        else
            i_reg = 0x02;                           //ֹͣ���� 
    }
    else if(*Register == 0x01)                      //BIT0 = 00000001 �ж���Ӧ��
    {                      
        i_reg = 0x00;
#if DBG
        sendchar('N');
#endif
    }
    else
    {     
#if DBG    
        send_cstring("Interrupt error");        //�����жϴ���
        send_byte(*Register);
#endif        
        i_reg = 0x02;

        *Register = StopDecoders;                   //��TX���ͽ��պ�λFIFO
        DirectCommand(Register);

        *Register = Reset;
        DirectCommand(Register);

        *Register = IRQStatus;                      //��ȡIRQ�ж�״̬�Ĵ�����ַ
        *(Register + 1) = IRQMask;

       
            ReadCont(Register, 2);                  //��ȡ�Ĵ���
        
        IRQCLR();                                   //���ж�
    }
}   /* InterruptHandlerReader */

/********************************************************************************************************
* �������ƣ�Port_0()
* ��    �ܣ��Ķ����ж���ڳ���
* ��ڲ�������
* ���ڲ�������
* ˵    ���������ⲿ�жϷ������
*********************************************************************************************************/
void Port_0(void) interrupt 0     
{
    unsigned char Register[4];

    StopCounter();                                  //��ʱ��ֹͣ
		
    do
    {
        IRQCLR();                                   //��˿�2�жϱ�־λ
        Register[0] = IRQStatus;                    //��ȡIRQ�ж�״̬�Ĵ�����ַ
        Register[1] = IRQMask;                      //����� Dummy read                                 
        ReadCont(Register, 2); 											//��ȡ�Ĵ���
        if(*Register == 0xA0)                       //A0 = 10100000 ָʾTX���ͽ�����������FIFO��ʣ��3�ֽ�����
        {   
            goto FINISH;                            //��ת��FINISH��������͹���ģʽ
        }
        
        InterruptHandlerReader(Register);       //ִ���жϷ������
				
    }while((IRQPORT & IRQPin) == IRQPin);           //����ִ��
FINISH:
   PCON &= ~0X03;                                   //�˳�idle��stop��״̬

}



