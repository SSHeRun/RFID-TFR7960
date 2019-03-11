/**********************************************************************************************************************************************************
* �� �� ����HOST.C
* ��    �ܣ�SPI��س�ʼ������USART��UART����������
*
* ��    �ߣ�EMDOOR
* ��    �ڣ�2011��04��13��
**********************************************************************************************************************************************************/
#include "hardware.h"
#include "communicate.h"
#include "anticollision.h"
#include "globals.h"
#include "tiris.h"
#include "ISO14443.h"
#include "host.h"
#include "lcd.h"


unsigned char   RXdone;                             //�����������ݱ�־λ����������ɣ��øñ�־λ1
unsigned char   ENABLE;                             //TRF7960ʹ�����ֹ�л���־λ��1ʹ�ܣ�0��ֹ״̬
unsigned char   Firstdata = 1;                      //���ô���ͬ����־λ
unsigned char   LCDcon = 0;                         //LCD������־λ

/******************************************************************************************************************
* �������ƣ�UARTset()
* ��    �ܣ����ڳ�ʼ�����ú���
* ��ڲ�������
* ���ڲ�������
* ˵    �����ô��ڳ�ʼ������ʹ��USCI_A0��    
*******************************************************************************************************************/
void UARTset(void)
{
    P3SEL |= BIT4 + BIT5;                           //P3.4��P3.5�ܽ����ó�UARTģʽ
    P3DIR |= BIT4;                                  //����P3.4Ϊ�������

    UCA0CTL1 |= UCSWRST;                            //��ֹUART
    UCA0CTL0 = 0x00;

    UCA0CTL1 |= UCSSEL_2;                           //ѡ��Ƶ��SMCLK
    UCA0BR0 = BAUD0;                                //ѡ��115200
    UCA0BR1 = BAUD1;

    UCA0MCTL = 0;                                   //���Ʋ���UCBRSx = 0
  
    UCA0CTL1 &= ~UCSWRST;                           //��ʼ��USCI_A0״̬��ʹ��UART
    IE2 |= UCA0RXIE;                                //ʹ��USCI_A0�����ж�
}

/******************************************************************************************************************
* �������ƣ�USARTset()
* ��    �ܣ�SPI��ʼ�����ú���
* ��ڲ�������
* ���ڲ�������
* ˵    �����ú���ʹ���ڲ������������� USCI_B0��    
*******************************************************************************************************************/
void USARTset(void)
{
    UCB0CTL1 |= UCSWRST;                             //���Ƚ�ֹUSCI
    UCB0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC;     //����3�ܽţ�8λSPI���豸
    UCB0CTL1 |= UCSSEL_2;                            //ѡ��SMCLK

    UCB0BR0 = 0x00;
    UCB0BR1 = 0x00;
    P3SEL |= BIT1 + BIT2 + BIT3;                     //ѡ��P3.1ΪUCB0SIMO,P3.2ΪUCB0SOMI,P3.3ΪUCBOCLK

    SlaveSelectPortSet();                            //P3.0 Ϊ Slave Select���豸ѡ���
    H_SlaveSelect();                                 //���豸ʹ��Slave Select����ֹ(�ߵ�ƽ)

    UCB0CTL1 &= ~UCSWRST;                            //��ʼ��USCI״̬�Ĵ���
}

/******************************************************************************************************************
* �������ƣ�USARTEXTCLKset()
* ��    �ܣ�SPI��ʼ�����ú���
* ��ڲ�������
* ���ڲ�������
* ˵    �����ú���ʹ���ⲿ������������ USCI_B0��    
*******************************************************************************************************************/
void USARTEXTCLKset(void)
{
    UCB0CTL1 |= UCSWRST;                            //���Ƚ�ֹUSCI
    UCB0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC;    //����3�ܽţ�8λSPI���豸
    UCB0CTL1 |= UCSSEL_2;                           //ѡ��SMCLK

    UCB0BR0 = 0x00;
    UCB0BR1 = 0x00;
    P3SEL |= BIT1 + BIT2 + BIT3;                    //ѡ��P3.1ΪUCB0SIMO,P3.2ΪUCB0SOMI,P3.3ΪUCBOCLK

    SlaveSelectPortSet();                           //P3.0 Ϊ Slave Select���豸ѡ���
    H_SlaveSelect();                                //���豸ʹ��Slave Select����ֹ(�ߵ�ƽ)

    UCB0CTL1 &= ~UCSWRST;                           //��ʼ��USCI״̬�Ĵ���
}

/******************************************************************************************************************
* �������ƣ�BaudSet()
* ��    �ܣ�����ͨ�����ú���
* ��ڲ�����mode    ģʽ����
* ���ڲ�������
* ˵    ��������Ĭ���ⲿ����Ƶ��Ϊ6.78MHz;�ڲ�DCOģʽƵ��Ϊ8MHz    
*******************************************************************************************************************/
void BaudSet(unsigned char mode)
{
    if(mode == 0x00)                                //��Ϊ0x00 ��Ϊ�ⲿ���پ���ģʽ
    {
        UCA0BR0 = BAUD0;                            //���������øߵ�λ
        UCA0BR1 = BAUD1;
    }
    else                                            //�ڲ�DCO��ģʽ
    {
        UCA0BR0 = BAUD0EN;                          //���������øߵ�λ
        UCA0BR1 = BAUD1EN;
    }
}

/******************************************************************************************************************
* �������ƣ�kputchar()
* ��    �ܣ����������λ������һ���ַ�����
* ��ڲ�����TXchar    ��Ҫ�����͵��ַ�
* ���ڲ�������
* ˵    ������   
*******************************************************************************************************************/
void kputchar(char TXchar)
{
    while(!(IFG2 & UCA0TXIFG));                     //�ȴ� USCI_B0 TX ������׼���ã���գ�

    UCA0TXBUF = TXchar;                             //�����ַ�
}

/******************************************************************************************************************
* �������ƣ�put_bksp()
* ��    �ܣ����������λ������һ���˸�Backspace����
* ��ڲ�������
* ���ڲ�������
* ˵    ������   
*******************************************************************************************************************/
void put_bksp(void)
{
    kputchar('\b');                                 //�����ַ�\b
    kputchar(' ');                                  //���Ϳո�
    kputchar('\b');                                 //�����ַ�\b
}

/******************************************************************************************************************
* �������ƣ�put_space()
* ��    �ܣ����������λ������һ���ո����
* ��ڲ�������
* ���ڲ�������
* ˵    ������   
*******************************************************************************************************************/
void put_space(void)
{
    kputchar(' ');                                  //���Ϳո�
}

/******************************************************************************************************************
* �������ƣ�put_crlf()
* ��    �ܣ����������λ������һ���س�+���з���
* ��ڲ�������
* ���ڲ�������
* ˵    ������   
*******************************************************************************************************************/
void put_crlf(void)
{
    kputchar('\r');                                 //���ͻس�����
    kputchar('\n');                                 //���ͻ��з���
}

/******************************************************************************************************************
* �������ƣ�send_cstring()
* ��    �ܣ����������λ������һ�ַ���
* ��ڲ�����*pstr       ��Ҫ�����͵��ַ���
* ���ڲ�������
* ˵    ������   
*******************************************************************************************************************/
void send_cstring(char *pstr)
{
    while(*pstr != '\0')                            //��ѯ�Ƿ񵽴��ַ���β
    {
        kputchar(*pstr++);                          //�����ַ�
    }
}

/******************************************************************************************************************
* �������ƣ�Nibble2Ascii()
* ��    �ܣ���λ�ֽ�ת����ASCIIʮ��������
* ��ڲ�����anibble         ��Ҫ��ת���ֽ�
* ���ڲ�����AsciiOut        ת�����ASCIIʮ��������ֵ
* ˵    ������   
*******************************************************************************************************************/
unsigned char Nibble2Ascii(unsigned char anibble)
{
    unsigned char AsciiOut = anibble;               //����ת����ı���AsciiOut������ֵ

    if(anibble > 9)                                 //�����ת���İ�λ�ֽ�ΪA-F������Ҫ��0x07
        AsciiOut = AsciiOut + 0x07;

    AsciiOut = AsciiOut + 0x30;                     //�����������ת���������������ƫ����0x30

    return(AsciiOut);                               //����ת�����ֵ
}

/******************************************************************************************************************
* �������ƣ�Put_byte()
* ��    �ܣ������ֽں�����λ�ֽ�ת����ASCIIʮ��������
* ��ڲ�����abyte         ��Ҫ�������ֽ�
* ���ڲ�������      
* ˵    �����ú�����������Nibble2Ascii����һ���ֽڲ�ֳɸߵ���λ����ת���ٴ����͡�
*******************************************************************************************************************/
void Put_byte(unsigned char abyte)
{
    unsigned char temp1, temp2;

    temp1 = (abyte >> 4) & 0x0F;                    //��ȡ����λ�ֽ�
    temp2 = Nibble2Ascii(temp1);                    //ת����ASCII��
    kputchar(temp2);                                //����֮

    temp1 = abyte & 0x0F;                           //��ȡ����λ�ֽ�
    temp2 = Nibble2Ascii(temp1);                    //ת����ASCII��
    kputchar(temp2);                                //����֮
}

/******************************************************************************************************************
* �������ƣ�Get_nibble()
* ��    �ܣ��ڷ����ֽں���Put_byte�����󣬻�ȡһ��ʮ�������ֽ�
* ��ڲ�������
* ���ڲ�����rxdata          ���ؽ��յ����ֽ�
* ˵    �����ú�����������Nibble2Ascii����һ���ֽڲ�ֳɸߵ���λ����ת���ٴ����͡�
*******************************************************************************************************************/
unsigned char Get_nibble(void)
{
    unsigned char reading;                          //����־λ
    //unsigned char rxdata;

    reading = 1;                                    //��־λ��1 ��ʾ��ȡ��δ���
    while(reading)                                  //ѭ����ȡ�ַ�
    {                   		
        LPM0;                                       //����͹���ģʽ���ȴ�����
        if(rxdata >= 'a')                           //ת���ɴ�д��ĸ
        {
            rxdata -= 32;
        }

        /* ���Ϊʮ�����ƣ������֮ */
        /*====================================================================================================*/
        if(((rxdata >= '0') && (rxdata <= '9')) || ((rxdata >= 'A') && (rxdata <= 'F')))
        {
            reading = 0;
            kputchar(rxdata);                       //���ͻ����ַ�
        
            if(rxdata > '9')                        //���ASCII��ֵ��Χ��A-F�����9
            {       
                rxdata = (rxdata & 0x0F) + 9;
            }
        }
        /*====================================================================================================*/
    }
    
    return(rxdata);                                 //���ؽ����ַ�                             
}

/******************************************************************************************************************
* �������ƣ�Get_line()
* ��    �ܣ��ڻ�ȡ�����ֽں���Get_nibble�����󣬻�ȡһ�ַ����ַ�
* ��ڲ�����*pline          �����ַ���
* ���ڲ�����err_flg         ���ر�־
* ˵    �����ú�������������0��9,��ĸA��F,����Сд��ĸa��f,���س�<CR>,����<LF>,���˸�<backspace>����
*           �������ž�Ϊ�Ƿ��ַ����������ԡ�
*           ������������0����������Χ����1��
*******************************************************************************************************************/
unsigned char Get_line(unsigned char *pline)
{
    unsigned char reading, err_flg;                 //����־λ�������־λ
    unsigned char pos;                              //���յ��İ��ֽ�����
    unsigned char Length_Byte;                      //�������ֵΪ256�ֽڣ�512�����ֽڣ�
    
    err_flg = 0;                                    //�����޴����־
    Length_Byte = 0xff;                             //��������ֽ���

    if(!Firstdata)                                  //�ȴ�SOF��ʼ��־λ��0����Ϊ1
    {
        LPM0;                                       //����͹���ģʽ���ȴ����жϻ���
        while(rxdata != '0')                        //�жϽ��յ����ַ��Ƿ�Ϊ0���ٷ�һֱ�ȴ�
        {
        }
    }
    else
    {
        Firstdata = 0;
    }

    kputchar('0');                                  //�����ַ�0
    LPM0;                                           //����͹���ģʽ���ȴ����жϻ���
    while(rxdata != '1')                            //�жϽ��յ����ַ��Ƿ�Ϊ1���ٷ�һֱ�ȴ�
    {
    }

    kputchar('1');                                  //�����ַ�1
    RXdone = 0;                                     //SOF��ʼ��־λ����

    pos = 0;                                        //�ַ�λ�ü�������1
    reading = 1;                                    //��־λ��1 ��ʾ��ȡ��δ���
    while(reading)                                  //ѭ����ȡ�ַ�
    {               		
        while(RXdone == 0);                         //�ȴ���������

        RXdone = 0;                                 //�����־λ
        switch(rxdata)                              //���ݵõ������ݷֱ�ѡ�����²���
        {
            case '\r':                              //����ǻس����ţ�����֮
                break;                  
            case '\n':
                reading = 0;                        //����ǻ��з��š����˳���ѭ��
                break;
            case '\b':                              //������˸�backspace����
                if(pos > 0)
                {                                   //�����Ҫ���һ���ַ�?
                    pos--;                          //�ӻ��������Ƴ�
                    put_bksp();                     //�˸�ɾ��
                    if(pos & 0x01 > 0)
                    {                               //ż��λ�ֽڣ��ߣ�
                        *pline--;
                        *pline &= 0xF0;             //�����λ�ֽ�
                    }
                }
                break;
            default:                                //�����ַ�
            if(rxdata >= 'a')                       //ת���ɴ�д��ĸ
            {
                rxdata -= 32;
            }

            /* �������ʮ���������ݣ�������ѭ�� */
            /*-----------------------------------------------------------------------------*/
            if((rxdata < '0') || ((rxdata > '9') && (rxdata < 'A')) || (rxdata > 'F'))
            {
                break;
            }
            /*-----------------------------------------------------------------------------*/
                
            /* ���������пռ���洢֮ */
            /*-----------------------------------------------------------------------------*/
            if(pos++ < 2 * BUF_LENGTH)
            {
                kputchar(rxdata);                           //����
                if(rxdata > '9')                            //���ASCII��ֵ��Χ��A-F�����9
                {       	
                    rxdata = (rxdata & 0x0F) + 9;
                }

                if((pos & 0x01) == 0)                       //����λ���ֽ�����λΪ0(��)
                {           		
                    *pline += (rxdata & 0x0F);              //�洢֮
                    if(pos == 2)                            //������յ�2����λ�ֽڣ���ı䳤���ֽ�
                        Length_Byte = *pline;

                    pline++;
                    if(((Length_Byte - 1) * 2) == pos)      //�����ɣ������˳�ѭ����־
                        reading = 0;
                }
                else                                        //ż��λ�ֽڣ��ߣ�
                {   
                    rxdata = rxdata << 4;                   //�ƶ�������λ���ֽ�
                    *pline = rxdata;                        //�洢֮
                }
                /*-----------------------------------------------------------------------------*/
            }
            else                                            //�����洢�ռ䣬�򷵻ش���
                err_flg = 1;
        }   /* switch */
    }   /* while(1) */

    return(err_flg);                                        //�����˳�
}

/******************************************************************************************************************
* �������ƣ�HostCommands()
* ��    �ܣ�������λ�������������������Ӧ����
* ��ڲ�������
* ���ڲ�������
* ˵    �����ú���Ϊ��ѭ������
*******************************************************************************************************************/
void HostCommands(void)
{
    char *phello;
    unsigned char *pbuf, count;
    
    POLLING = 0;                                    //����POLLINGΪ0����������USBģʽ
    
    while(1)
    {
        pbuf = &buf[0];                             //ָ�붨λ
        Get_line(pbuf);                             //����Get_line������ȡһ������
        put_crlf();                                 //�س�+���н���

        pbuf = &buf[4];                             //ָ�����ݰ���5�����ֽ�
        RXErrorFlag = 0;                            //RX���ܴ����־λ��0

        if(*pbuf == 0xFF)                           //��鴮�ڶ˿ں� 0xFF��ʾ��TRF7960 EVM \r\n
        {
            phello = "**********************************************************\r\n*                                                        *\r\n* Welcome to use Emdoor RFID(13.56MHz) learning platform *\r\n*                                                        *\r\n**********************************************************\r\n";
            send_cstring(phello);
        }
        else if(*pbuf == 0x10)                      //0x10�Ĵ���д(��ַ+����,��ַ+����...)
        {           
            send_cstring("Register write request.\r\n");
            count = buf[0] - 8;
            WriteSingle(&buf[5], count);
        }
        else if(*pbuf == 0x11)                      //0x11����д(��ַ+����+����...)
        {          
            phello = "Continous write request.\r\n";
            send_cstring(phello);
            count = buf[0] - 8;
            WriteCont(&buf[5], count);
        }
        else if(*pbuf == 0x12)                      //0x12�Ĵ�����(��ַ+����,��ַ+����...)
        {           
            phello = "Register read request.\r\n";
            send_cstring(phello);
            count = buf[0] - 8;
            ReadSingle(&buf[5], count);
            Response(&buf[5], count);
        }
        else if(*pbuf == 0x13)                      //0x13������(��ַ+����+����...)
        {        
            send_cstring("Continous read request\r\n");
            pbuf++;
            count = *pbuf;                          //��Ҫ����ȡ�Ĵ���������

            pbuf++;
            buf[5] = *pbuf;                         //��ʼ�Ĵ�����ַ

            ReadCont(&buf[5], count);               //������
            Response(&buf[5], count);
        }
        else if(*pbuf == 0x14)                      //0x14������������
        {                           
            phello = "ISO 15693 Inventory request.\r\n";
            send_cstring(phello);
            flags = buf[5];
            for(count = 0; count < 8; count++) buf[count + 20] = 0x00;
                InventoryRequest(&buf[20], 0x00);
        }
        else if(*pbuf == 0x15)                      //0x15ֱ������
        {
            phello = "Direct command.\r\n";
            send_cstring(phello);
            DirectCommand(&buf[5]);
        }
        else if(*pbuf == 0x16)                      //0x16ֱ��д����
        {
            phello = "RAW mode.\r\n";
            send_cstring(phello);
            count = buf[0] - 8;
            RAWwrite(&buf[5], count);
        }
        else if(*pbuf == 0x17)                      //0x17Ԥ��
        {
        }
        else if(*pbuf == 0x18)                      //0x18��������
        {                
            phello = "Request mode.\r\n";
            send_cstring(phello);
            count = buf[0] - 8;
            RequestCommand(&buf[0], count, 0x00, 0);
        }
        else if(*pbuf == 0x19)                      //0x19 ISO14443A ����������ͺͽ���
        {
            /* �ڲ�ͬ�ı�����ģʽ�£���TX���ͺ�ı�ISOģʽ�Ĵ��� */
            phello = "14443A Request - change bit rate.\r\n";
            send_cstring(phello);
            count = buf[0] - 9;
            Request14443A(&buf[1], count, buf[5]);
        }
        else if(*pbuf == 0x34)                      //0x34 SID Tag-itЭ����������
        {             
            phello = "Ti SID Poll.\r\n";
            send_cstring(phello);
            flags = buf[5];
            for(count = 0; count < 4; count++) buf[count + 20] = 0x00;
                TIInventoryRequest(&buf[20], 0x00);
        }
        else if(*pbuf == 0x0F)                      //0x0Fֱ��ģʽ
        {
            phello = "Direct mode.\r\n";
            send_cstring(phello);
            DirectMode();
        }
        else if((*pbuf == 0xB0) || (*pbuf == 0xB1)) //ISO14443BЭ����������ͻ�������
        {      
            /* 0xB0 - REQB 0xB1 - WUPB */
            phello = "14443B REQB.\r\n";   
            send_cstring(phello);
            AnticollisionSequenceB(*pbuf, *(pbuf + 1));
            //AnticollisionSequenceB(0xB0, 0x00);   //������slotģʽ
        }
        else if((*pbuf == 0xA0) || (*pbuf == 0xA1)) //ISO14443AЭ����������
        {                   
            phello = "14443A REQA.\r\n";
            send_cstring(phello);
            AnticollisionSequenceA(*(pbuf + 1));
        }
        else if(*pbuf == 0xA2)
        {       
            phello = "14443A Select.\r\n";
            send_cstring(phello);
            switch(buf[0])
            {
                case 0x0D:
                    for(count = 1; count < 6; count++)
                        buf[99 + count] = *(pbuf + count);
                    break;
                case 0x11:
                    for(count = 1; count < 11; count++)
                        buf[100 + count] = *(pbuf + count);
                    buf[100] = 0x88;
                    break;
                case 0x15:
                    for(count = 1; count < 5; count++)
                        buf[100 + count] = *(pbuf + count);
                    buf[100] = 0x88;
                    buf[105] = 0x88;
                    for(count = 1; count < 10; count++)
                        buf[105 + count] = *(pbuf + count + 4);
            }   /* switch */

            buf[0] = ISOControl;
            buf[1] = 0x88;                          //���ղ���CRCУ��
            WriteSingle(buf, 2);

            buf[5] = 0x26;                          //����REQA��������

            if(RequestCommand(&buf[0], 0x00, 0x0f, 1) == 0)
            {
                if(SelectCommand(0x93, &buf[100]))
                {
                    if(SelectCommand(0x95, &buf[105])) SelectCommand(0x97, &buf[110]);
                }
            }
        }
        else if(*pbuf == 0x03)                      //ʹ�ܻ��߽�ֹITRF7960�Ķ���оƬ
        {                  
            if(*(pbuf + 1) == 0xAA)                 //0x03AAʹ��TRF7960
            {
                TRFEnable();
                BCSCTL2 |= SELM1 + SELM0 + SELS;
                InitialSettings();
                BaudSet(*(pbuf + 1));
                OSCsel(*(pbuf + 1));
                send_cstring("Reader enabled.");
                BeepON();
                delay_ms(50);
                BeepOFF();
                ENABLE = 1;
            }
            else if(*(pbuf + 1) == 0xFF)            //0x03FF��ֹTRF7960
            {
                BaudSet(*(pbuf + 1));
                OSCsel(*(pbuf + 1));
                TRFDisable();
                BeepON();
                delay_ms(10);
                BeepOFF();
                send_cstring("Reader disabled.");
                ENABLE = 0;
            }
            else if(*(pbuf + 1) == 0x0A)            //0x030Aʹ���ⲿ����ʱ��
            {
                BaudSet(0x00);
                OSCsel(0x00);
                send_cstring("External clock.");
            }
            else if(*(pbuf + 1) == 0x0B)            //0x030Bʹ���ڲ�����ʱ��
            {
                BaudSet(0x01);
                OSCsel(0x01);
                send_cstring("Internal clock.");
            }
            else                                    //����Ԥ��
                ;
        }
        else if(*pbuf == 0xF0)                      //0xF0 AGC����
        {     
            buf[0] = ChipStateControl;
            buf[1] = ChipStateControl;
            ReadSingle(&buf[1], 1);
            if(*(pbuf + 1) == 0xFF)                 //AGC����
                buf[1] |= BIT2;
            else                                    //AGC�ر�
                buf[1] &= ~BIT2;
            WriteSingle(buf, 2);
        }
        else if(*pbuf == 0xF1)                      //0xF1 AM PM�������
        {            
            buf[0] = ChipStateControl;
            buf[1] = ChipStateControl;
            ReadSingle(&buf[1], 1);
            if(*(pbuf + 1) == 0xFF)                 //ѡ��AM
                buf[1] &= ~BIT3;
            else                                    //ѡ��PM
                buf[1] |= BIT3;
            WriteSingle(buf, 2);
        }
        else if(*pbuf == 0xF2)                      //0xF2 ��������
        {                  
            buf[0] = ChipStateControl;
            buf[1] = ChipStateControl;
            ReadSingle(&buf[1], 1);
            if(*(pbuf + 1) == 0xFF)
                buf[1] &= ~BIT4;
            else
                buf[1] |= BIT4;
            WriteSingle(buf, 2);
        }
        else if(*pbuf == 0xFE)                      //�̼��汾��
        {                
            phello = "Firmware V1.0.0 \r\n";
            send_cstring(phello);
        }
        else                                        //����Ϊ�Ƿ�����
        {
            phello = "Unknown command.\r\n";
            send_cstring(phello);
        }   /* if */

        while(!(IFG2 & UCA0TXIFG));                 //�ȴ�TX������׼���ã���գ�
        
        /* ������λ���ɹ�ͨ�ţ�����LCD����ʾUSB����ͼ�� */
        /*-----------------------------------------------------------------------------*/
        if(LCDcon)
        {
            LCDcon = 0;
        
            Display_Clear();
            Display_Connect();
            //Display_Picture((unsigned char *)usb); //��ʾUSB����ͼ��
        }
        /*-----------------------------------------------------------------------------*/
    }   /* while(1) */
}   /* HostCommands */

/******************************************************************************************************************
* �������ƣ�RXhandler()
* ��    �ܣ����ڽ����жϷ������
* ��ڲ�������
* ���ڲ�������
* ˵    ��������USCI_A0���������ж�
*******************************************************************************************************************/
#pragma vector = USCIAB0RX_VECTOR
__interrupt void RXhandler (void)
{
    if(IFG2 & UCA0RXIFG)                            //������ڽ��յ�����
    {   
        rxdata = UCA0RXBUF;                         //�����ջ�����UCA0RXBUF���ݸ�ֵ��rxdata
        RXdone = 1;                                 //������ձ�־λ
        if(ENABLE == 0)                             //TRF7960�ڽ�ֹ״̬��
        {
            TRFEnable();                            //ʹ��TRF790
            BaudSet(0x00);                          //���ò�����
            OSCsel(0x00);                           //�����ⲿ�������� 
               
            InitialSettings();                      //��ʼ��TRF7960
            send_cstring("Reader enabled.");        //����λ��������Ϣ
            ENABLE = 1;                             //����TRF7960��־λ
        }
        __low_power_mode_off_on_exit();

        if(Firstdata)                               //����ǵ�1�ν��յ�����
        {
            LCDcon = 1;
            
            IRQOFF();                               //�ر�IRQ�ж�
            StopCounter();                          //ֹͣ������
                  
            /* ����SP���������жϷ��غ󣬿ɵ���HostCommands���� */
            /*-----------------------------------------------------------------------------*/
            asm("mov.w #HostCommands,10(SP)");      //����HostCommands����
            /*-----------------------------------------------------------------------------*/
        }
    }
}




