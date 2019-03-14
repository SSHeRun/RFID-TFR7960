/**********************************************************************************************************************************************************
* �� �� ����AUTOMATIC.C
* ��    �ܣ�����Ķ����Ķ���Χ�ڵ����о�꿨Ƭ��
*
* ��    �ߣ�EMDOOR
* ��    �ڣ�2011��04��13��
**********************************************************************************************************************************************************/
#include "automatic.h"
#include "lcd.h"


#define RFID_TEST       1                           //�����

unsigned char   protocol;                           //����Э�����ͼĴ�������

unsigned char Set_pro[9]={0x0C,0x00,0x03,0x04,0x10,0x00,0x21,0x01,0x00};                    //���ò�������
unsigned char Write_Sig[12]={0x0F,0x00,0x03,0x04,0x18,0x40,0x21,0x01,0x78,0x56,0x34,0x12};  //д���ַ0x01 ����12345678
unsigned char Read_Sig[8]={0x0B,0x00,0x03,0x04,0x18,0x00,0x20,0x01};                        //д���ַ0x01
unsigned char Write_AFI[8]={0x0B,0x00,0x03,0x04,0x18,0x40,0x27,0x01};                       //дAFI����01
unsigned char Write_DSFID[8]={0x0B,0x00,0x03,0x04,0x18,0x40,0x29,0xEE};                     //дDSFID����EE
unsigned char Get_info[7]={0x0A,0x00,0x03,0x04,0x18,0x00,0x2B};                             //��ȡ��Ƭϵͳ��Ϣ
unsigned char Get_sec[9]={0x0C,0x00,0x03,0x04,0x18,0x00,0x2C,0x01,0x02};                    //��ȡ�鰲ȫ״̬��ַ0x01,������02��ʵ��Ϊ3���飩


#if RFID_TEST
/******************************************************************************************************************
* �������ƣ�RFID_Test()
* ��    �ܣ�RFID��Ƭ�ѻ����Ժ�����
* ��ڲ�������
* ���ڲ�������
* ˵    �����ú����ܶԿ�Ƭ���ж�д��������дAFI��дDSFID���ͻ�ȡ��Ƭ��Ϣ��Ϣ�Ȳ���    
*******************************************************************************************************************/
void RFID_test(void)
{
    unsigned char i, count;
    unsigned char j, temp2;
    
    if(Found_tag)
    {
        for(i = 0; i < 9; i++)                      //����TRF7961ͨ�Ų���
        {
            buf[i]=Set_pro[i];
        }
       
        count = buf[0] - 8;
        WriteSingle(&buf[5], count);
    
        for(test_no=0;test_no<6;test_no++)
        {   
            delay_ms(800);
            Display_Clear();                        //����
            Display_Rssi(rssival);                  //��ʾRSSI����ǿ��ͼ��
        
            switch(test_no)
            {
                case 0:
                for(i = 0; i < 12; i++)             //дһ�����ַΪ0x01 12345678
                {
                    buf[ i ] = Write_Sig[ i ];
                }
                Display_Chinese(0, 0, (unsigned char *)xie);                    //��ʾ��дһ���顱����
                Display_Chinese(0, 1, (unsigned char *)yi);
                Display_Chinese(0, 2, (unsigned char *)ge);
                Display_Chinese(0, 3, (unsigned char *)kuai);
                Display_Char(2, 0, (unsigned char *)Num+(buf[7]  >> 4)*0x10);   //��ʾ��ַ
                Display_Char(2, 1, (unsigned char *)Num+(buf[7]  & 0x0f)*0x10);

                for(j = 8, temp2 = 15; j < 12; j++,temp2 -= 2)                  //��ʾ����
                {
                    Display_Char(2, temp2 - 1, (unsigned char *)(Num + (buf[j] >> 4) * 0x10));
                    Display_Char(2, temp2,(unsigned char *)(Num+(buf[j] & 0x0F) * 0x10));
                }
                break;
                case 1:
                for(i = 0; i < 9; i++)              //��һ�����ַΪ0x01
                {
                    buf[ i ] = Read_Sig[ i ];
                }
                Display_Chinese(0, 0, (unsigned char *)du);                     //��ʾ����һ���顱
                Display_Chinese(0, 1, (unsigned char *)yi);
                Display_Chinese(0, 2, (unsigned char *)ge);
                Display_Chinese(0, 3, (unsigned char *)kuai);
                Display_Char(2, 0, (unsigned char *)Num+(buf[7]  >> 4)*0x10);   //��ʾ��ַ
                Display_Char(2, 1, (unsigned char *)Num+(buf[7]  & 0x0f)*0x10);
                break;
                case 2:
                for(i = 0; i < 8; i++)              //дAFIӦ����
                {
                    buf[ i ] = Write_AFI[ i ];
                }
                Display_Chinese(0, 0, (unsigned char *)xie);                    //��ʾ��дAFI���ַ�
                Display_Char(0, 2, (unsigned char *)Num+0x0A*0x10);
                Display_Char(0, 3, (unsigned char *)Num+0x0F*0x10);
                Display_Char(0, 4, (unsigned char *)CI);
                Display_Char(2, 13, (unsigned char *)Num+(buf[7]  >> 4)*0x10);  //��ʾд�������
                Display_Char(2, 14, (unsigned char *)Num+(buf[7]  & 0x0f)*0x10);
                break;
                case 3:
                for(i = 0; i < 8; i++)              //дDSFID���ݴ洢��ʽ
                {
                    buf[i]=Write_DSFID[i];
                }
                Display_Chinese(0, 0, (unsigned char *)xie);                    //��ʾ��дDSFID���ַ�
                Display_Char(0, 2, (unsigned char *)Num+0x0D*0x10);
                Display_Char(0, 3, (unsigned char *)CS);
                Display_Char(0, 4, (unsigned char *)Num+0x0F*0x10);
                Display_Char(0, 5, (unsigned char *)CI);
                Display_Char(0, 6, (unsigned char *)Num+0x0D*0x10);
                Display_Char(2, 13, (unsigned char *)Num+(buf[7]  >> 4)*0x10);  //��ʾ����
                Display_Char(2, 14, (unsigned char *)Num+(buf[7]  & 0x0f)*0x10);
                break;
                case 4:
                for(i = 0; i < 7; i++)              //��ȡ��Ƭ��Ϣ��Ϣ
                {
                    buf[ i ] = Get_info[ i ];
                }
                Display_Chinese(0, 0, (unsigned char *)xi);                     //��ʾ��ϵͳ��Ϣ��
                Display_Chinese(0, 1, (unsigned char *)tong);
                Display_Chinese(0, 2, (unsigned char *)xin);
                Display_Chinese(0, 3, (unsigned char *)xi2);
                break;
                case 5:
                for(i = 0; i < 9; i++)              //��ȡ�鰲ȫ״̬   
                {
                    buf[ i ] = Get_sec[ i ];
                }
                Display_Chinese(0, 0, (unsigned char *)an);                     //��ʾ����ȫ״̬��
                Display_Chinese(0, 1, (unsigned char *)quan);
                Display_Chinese(0, 2, (unsigned char *)zhuang);
                Display_Chinese(0, 3, (unsigned char *)tai);
                Display_Char(2, 0, (unsigned char *)CS);                        //��ʾ��S����ʾ��ʼ���ַ
                Display_Char(2, 1, (unsigned char *)Num+(buf[7]  >> 4)*0x10);   //��ʼ��ַ
                Display_Char(2, 2, (unsigned char *)Num+(buf[7]  & 0x0f)*0x10);
                Display_Char(2, 4, (unsigned char *)CN);                        //��ʾ��N����ʾ������
                Display_Char(2, 5, (unsigned char *)Num+(buf[8]  >> 4)*0x10);   //������
                Display_Char(2, 6, (unsigned char *)Num+(buf[8]  & 0x0f)*0x10);
                break;
                default:break;
            }
            Display_pro();                          //��ʾ������...
            
            count = buf[0] - 8;
            RequestCommand(&buf[0], count, 0x00, 0);//������������  
        }
        delay_ms(800);
    }
}
#endif


/******************************************************************************************************************
* �������ƣ�FindTags()
* ��    �ܣ�����ָ�����Э�����ͣ�����TRF7960���ø���ؼĴ����󣬽���Ѱ��������
* ��ڲ�����protocol       ָ��Э������
* ���ڲ�������
* ˵    �����ú�����һ����ѭ�����������е��ѻ���ʾִ�й��̾��ڴ���ɡ�
*******************************************************************************************************************/
void FindTags(unsigned char protocol)
{
    unsigned char command[10];                      //�������������ݴ滺��������

    while(1)
    {
        if((protocol == 0x00) || (protocol == 0x01))//ISO15693Э���׼
        {
            command[0] = ChipStateControl;          // ����RFʹ�ܣ�ѡ��5V����ģʽ
            command[1] = 0x21;
            command[2] = ISOControl;                // ����ѡ��ISO15693����ģʽΪ:�߱�����26.48kbps �����ز� 1/4(Ĭ��ģʽ)
            command[3] = 0x02;
            WriteSingle(command, 4);                // д4���ֽ����TRF7960�Ĵ�����

            delay_ms(5);
            flags = 0x06;                           // 16(slot)��ģʽ
            //flags = 0x26;                         // 1(slot)��ģʽ

            command[0] = 0x00;
            InventoryRequest(command, 0);           // ����������������(��Ѱ������)

            command[0] = ChipStateControl;          // �ر�RF���ֵ�·
            command[1] = 0x01;
            WriteSingle(command, 2);
            delay_ms(1);

            command[0] = IRQStatus;                 // ���Ĵ�����ֵ
            command[1] = IRQMask;               

            if(SPIMODE)                             //��ȡIRQ�ж�״̬�Ĵ������жϱ�־
                ReadCont(command, 2);
            else
                ReadSingle(command, 1); 
#if RFID_TEST
    if(Found_tag == 1)
        RFID_test();
        //return;
#endif
        }

        if((protocol == 0x00) || (protocol == 0x02))//ISO14443AЭ���׼
        {
            command[0] = ChipStateControl;          // ����RFʹ�ܣ�ѡ��5V����ģʽ
            command[1] = 0x21;
            command[2] = ISOControl;                // ����ѡ��ISO14443A����ģʽΪ:������106kbps
            command[3] = 0x08;
            WriteSingle(command, 4);
            delay_ms(5);

            AnticollisionSequenceA(0x00);           //ִ��ISO14443A�����³�ײ����

            command[0] = ChipStateControl;          // ���Ĵ�����ֵ
            command[1] = 0x01;
            WriteSingle(command, 2);                // �ر�RF���ֵ�· 
            delay_ms(1);

            command[0] = IRQStatus;                 // ���Ĵ�����ֵ 
            command[1] = IRQMask;   
        
            if(SPIMODE)
                ReadCont(command, 2);               //��ȡIRQ�ж�״̬�Ĵ������жϱ�־
            else
                ReadSingle(command, 1); 
        }

        if((protocol == 0x00) || (protocol == 0x03))//ISO14443BЭ���׼
        {
            command[0] = ChipStateControl;
            command[1] = 0x21;                      // ����RFʹ�ܣ�ѡ��5V����ģʽ
            WriteSingle(command, 2);
            command[0] = ISOControl;                // ����ѡ��ISO14443B����ģʽΪ:������106kbps
            command[1] = 0x0C;
            WriteSingle(command, 2);

            delay_ms(5);
            AnticollisionSequenceB(0xB0, 0x04);     //ִ��ISO14443A�����³�ײ����(0x04��ʾ16��slots)
            //AnticollisionSequenceB(0xB0, 0x00);   //0x00 ��ʾ������slot

            command[0] = ChipStateControl;  
            command[1] = 0x01;
            WriteSingle(command, 2);                // �ر�RF���ֵ�· 
            delay_ms(1);

            command[0] = IRQStatus;
            command[1] = IRQMask;   
        
            if(SPIMODE)                             //��ȡIRQ�ж�״̬�Ĵ������жϱ�־
                ReadCont(command, 2);
            else
                ReadSingle(command, 1); 
        }

        if((protocol == 0x00) || (protocol == 0x04))//Tag-itЭ���׼
        {
            command[0] = ChipStateControl;          // ����RFʹ�ܣ�ѡ��5V����ģʽ
            command[1] = 0x21;
            command[2] = ISOControl;                // ����ѡ��Tag-it����ģʽ
            command[3] = 0x13;
            WriteSingle(command, 4);
            delay_ms(5);
            flags = 0x00;
            command[0] = 0x00;
            TIInventoryRequest(command, 0);         //����Ѱ������

            command[0] = ChipStateControl;          // �ر�RF���ֵ�· 
            command[1] = 0x01;
            WriteSingle(command, 2);
            delay_ms(1);

            command[0] = IRQStatus;                 // ���Ĵ�����ֵ 
            command[1] = IRQMask;
        
            if(SPIMODE)
                ReadCont(command, 2);               //��ȡIRQ�ж�״̬�Ĵ������жϱ�־
            else
                ReadSingle(command, 1); 
        }
        delay_ms(800);
    }   /* while */
}   /* FindTags */




