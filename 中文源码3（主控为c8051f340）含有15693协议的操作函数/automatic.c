/******************************************************************************************************************
* �� �� ����AUTOMATIC.C
* ��    �ܣ�����Ķ����Ķ���Χ�ڵ����о�꿨Ƭ��
*
* ��    �ߣ�EMDOOR
* ��    �ڣ�2011-9-29
********************************************************************************************************************/
#include <automatic.h>

unsigned char Set_pro[9]={0x0C,0x00,0x03,0x04,0x10,0x00,0x21,0x01,0x00};                    //??????
unsigned char Write_Sig[12]={0x0F,0x00,0x03,0x04,0x18,0x40,0x21,0x01,0x78,0x56,0x34,0x12};  //????0x01 ??12345678
unsigned char Read_Sig[8]={0x0B,0x00,0x03,0x04,0x18,0x00,0x20,0x01};                        //????0x01
unsigned char Write_AFI[8]={0x0B,0x00,0x03,0x04,0x18,0x40,0x27,0x01};                       //?AFI??01
unsigned char Write_DSFID[8]={0x0B,0x00,0x03,0x04,0x18,0x40,0x29,0xEE};                     //?DSFID??EE
unsigned char Get_info[7]={0x0A,0x00,0x03,0x04,0x18,0x00,0x2B};                             //????????
unsigned char Get_sec[9]={0x0C,0x00,0x03,0x04,0x18,0x00,0x2C,0x01,0x02};                    //?????????0x01,???02(???3??)


/******************************************************************************************************************
* ????:RFID_Test()
* ?    ?:RFID?????????
* ????:?
* ????:?
* ?    ?:????????????????AFI??DSFID?????????????    
*******************************************************************************************************************/
void RFID_test(void)
{
    unsigned char i, count;
    unsigned char j, temp2;
    
//    if(Found_tag)
//    {
        for(i = 0; i < 9; i++)                      //??TRF7961????
        {
            buf[i]=Set_pro[i];
        }
       
        count = buf[0] - 8;
        WriteSingle(&buf[5], count);
    
//        for(test_no=0;test_no<6;test_no++)
//        {   
//            delay_ms(800);
//           
//            switch(test_no)
//            {
//                case 0:
//                for(i = 0; i < 12; i++)             //???????0x01 12345678
//                {
//                    buf[ i ] = Write_Sig[ i ];
//                }
//                Display_Chinese(0, 0, (unsigned char *)xie);                    //??�????�??
//                Display_Chinese(0, 1, (unsigned char *)yi);
//                Display_Chinese(0, 2, (unsigned char *)ge);
//                Display_Chinese(0, 3, (unsigned char *)kuai);
//                Display_Char(2, 0, (unsigned char *)Num+(buf[7]  >> 4)*0x10);   //????
//                Display_Char(2, 1, (unsigned char *)Num+(buf[7]  & 0x0f)*0x10);

//                for(j = 8, temp2 = 15; j < 12; j++,temp2 -= 2)                  //????
//                {
//                    Display_Char(2, temp2 - 1, (unsigned char *)(Num + (buf[j] >> 4) * 0x10));
//                    Display_Char(2, temp2,(unsigned char *)(Num+(buf[j] & 0x0F) * 0x10));
//                }
//                break;
//                case 1:
//                for(i = 0; i < 9; i++)              //???????0x01
//                {
//                    buf[ i ] = Read_Sig[ i ];
//                }
//                Display_Chinese(0, 0, (unsigned char *)du);                     //??�????�
//                Display_Chinese(0, 1, (unsigned char *)yi);
//                Display_Chinese(0, 2, (unsigned char *)ge);
//                Display_Chinese(0, 3, (unsigned char *)kuai);
//                Display_Char(2, 0, (unsigned char *)Num+(buf[7]  >> 4)*0x10);   //????
//                Display_Char(2, 1, (unsigned char *)Num+(buf[7]  & 0x0f)*0x10);
//                break;
//                case 2:
//                for(i = 0; i < 8; i++)              //?AFI???
//                {
//                    buf[ i ] = Write_AFI[ i ];
//                }
//                Display_Chinese(0, 0, (unsigned char *)xie);                    //??�?AFI�??
//                Display_Char(0, 2, (unsigned char *)Num+0x0A*0x10);
//                Display_Char(0, 3, (unsigned char *)Num+0x0F*0x10);
//                Display_Char(0, 4, (unsigned char *)CI);
//                Display_Char(2, 13, (unsigned char *)Num+(buf[7]  >> 4)*0x10);  //???????
//                Display_Char(2, 14, (unsigned char *)Num+(buf[7]  & 0x0f)*0x10);
//                break;
//                case 3:
//                for(i = 0; i < 8; i++)              //?DSFID??????
//                {
//                    buf[i]=Write_DSFID[i];
//                }
//                Display_Chinese(0, 0, (unsigned char *)xie);                    //??�?DSFID�??
//                Display_Char(0, 2, (unsigned char *)Num+0x0D*0x10);
//                Display_Char(0, 3, (unsigned char *)CS);
//                Display_Char(0, 4, (unsigned char *)Num+0x0F*0x10);
//                Display_Char(0, 5, (unsigned char *)CI);
//                Display_Char(0, 6, (unsigned char *)Num+0x0D*0x10);
//                Display_Char(2, 13, (unsigned char *)Num+(buf[7]  >> 4)*0x10);  //????
//                Display_Char(2, 14, (unsigned char *)Num+(buf[7]  & 0x0f)*0x10);
//                break;
//                case 4:
                for(i = 0; i < 7; i++)              //????????
                {
                    buf[ i ] = Get_info[ i ];
                }
//                Display_Chinese(0, 0, (unsigned char *)xi);                     //??�????�
//                Display_Chinese(0, 1, (unsigned char *)tong);
//                Display_Chinese(0, 2, (unsigned char *)xin);
//                Display_Chinese(0, 3, (unsigned char *)xi2);
//                break;
//                case 5:
//                for(i = 0; i < 9; i++)              //???????   
//                {
//                    buf[ i ] = Get_sec[ i ];
//                }
//                Display_Chinese(0, 0, (unsigned char *)an);                     //??�????�
//                Display_Chinese(0, 1, (unsigned char *)quan);
//                Display_Chinese(0, 2, (unsigned char *)zhuang);
//                Display_Chinese(0, 3, (unsigned char *)tai);
//                Display_Char(2, 0, (unsigned char *)CS);                        //??�S�???????
//                Display_Char(2, 1, (unsigned char *)Num+(buf[7]  >> 4)*0x10);   //????
//                Display_Char(2, 2, (unsigned char *)Num+(buf[7]  & 0x0f)*0x10);
//                Display_Char(2, 4, (unsigned char *)CN);                        //??�N�?????
//                Display_Char(2, 5, (unsigned char *)Num+(buf[8]  >> 4)*0x10);   //???
//                Display_Char(2, 6, (unsigned char *)Num+(buf[8]  & 0x0f)*0x10);
//                break;
//                default:break;
//            }
//            Display_pro();                          //?????...
            
            count = buf[0] - 8;
            RequestCommand(&buf[0], count, 0x00, 0);//??????  
//        }
        delay_ms(800);
//    }
}
/******************************************************************************************************************
* �������ƣ�FindTags()
* ��    �ܣ�����ָ�����Э�����ͣ�����TRF7960���ø���ؼĴ����󣬽���Ѱ��������
* ��ڲ�����protocol       ָ��Э������
* ���ڲ�������
* ˵    �����ú�����һ����ѭ�����������е��ѻ���ʾִ�й��̾��ڴ���ɡ�
*******************************************************************************************************************/
void FindTags(void)
{
    unsigned char command[10];                      //�������������ݴ滺��������

    while(1)
    {
            command[0] = ChipStateControl;          // ����RFʹ�ܣ�ѡ��5V����ģʽ
            command[1] = 0x21;
            command[2] = ISOControl;                // ����ѡ��ISO15693����ģʽΪ:�߱�����26.48kbps �����ز� 1/4(Ĭ��ģʽ)
            command[3] = 0x02;
            WriteSingle(command, 4);                // д4���ֽ����TRF7960�Ĵ�����

            delay_ms(5);
            flags = 0x06;                           // 16(slot)��ģʽ
            //flags = 0x26;

            command[0] = 0x04;

            InventoryRequest(command, 0);           // ����������������(��Ѱ������)

            command[0] = ChipStateControl;          // �ر�RF���ֵ�·
            command[1] = 0x01;
            WriteSingle(command, 2);
            delay_ms(1);

            command[0] = IRQStatus;                 // ���Ĵ�����ֵ
            command[1] = IRQMask;               

                                      
						ReadCont(command, 2);							   //��ȡIRQ�ж�״̬�Ĵ������жϱ�־
						
						
						RFID_test();
		
        delay_ms(10);
    }   /* while */
}   /* FindTags */




