/******************************************************************************************************************
* �� �� ����AUTOMATIC.C
* ��    �ܣ�����Ķ����Ķ���Χ�ڵ����о�꿨Ƭ��
*
* ��    �ߣ�EMDOOR
* ��    �ڣ�2011-9-29
********************************************************************************************************************/
#include <automatic.h>

unsigned char Set_pro[9]={0x0C,0x00,0x03,0x04,0x10,0x00,0x21,0x01,0x00};                    //??????
unsigned char Write_Sig[12]={0x0F,0x00,0x03,0x04,0x18,0x00,0x21,0x08,0xFF,0xFF};  //????0x01 ??12345678
unsigned char Read_Sig[8]={0x0B,0x00,0x03,0x04,0x18,0x00,0x20,0x08};                        //????0x01
//unsigned char Write_AFI[8]={0x0B,0x00,0x03,0x04,0x18,0x00,0x27,0x01};                       //?AFI??01
//unsigned char Write_DSFID[8]={0x0B,0x00,0x03,0x04,0x18,0x40,0x29,0xEE};                     //?DSFID??EE
//unsigned char Get_info[7]={0x0A,0x00,0x03,0x04,0x18,0x00,0x2B};                             //????????
//unsigned char Get_sec[9]={0x0C,0x00,0x03,0x04,0x18,0x00,0x2C,0x01,0x01};                    //?????????0x01,???02(???3??)

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
		unsigned char i, count;
//		while(1)
//    {
//            command[0] = ChipStateControl;          // ??RF??,??5V????
//            command[1] = 0x21;
//            command[2] = ISOControl;                // ????ISO15693?????:????26.48kbps ???? 1/4(????)
//            command[3] = 0x02;
//            WriteSingle(command, 4);                // ?4??????TRF7960????

//            delay_ms(5);
//            flags = 0x06;                           // 16(slot)???
//            //flags = 0x26;

//            command[0] = 0x04;
//						
//						count = buf[0] - 9;
//						InventoryRequest(command, 0);           // ????????(?????)      

//            command[0] = ChipStateControl;          // ??RF????
//            command[1] = 0x01;
//            WriteSingle(command, 2);
//            delay_ms(1);

//            command[0] = IRQStatus;                 // ??????
//            command[1] = IRQMask;               

//                                      
//		    ReadCont(command, 2);							   //??IRQ????????????
//		
//        delay_ms(10);
//    }   /* while */
	
//		while(1)
//		{
//						command[0] = ChipStateControl;          // ??RF??,??5V????
//            command[1] = 0x21;
//            command[2] = ISOControl;                // ????ISO14443A?????:???106kbps
//            command[3] = 0x08;
//            WriteSingle(command, 4);
//            delay_ms(5);
//						
//            AnticollisionSequenceA(0x00);           //??ISO14443A???????
//						
//            command[0] = ChipStateControl;          // ??????
//            command[1] = 0x01;
//            WriteSingle(command, 2);                // ??RF???? 
//            delay_ms(1);

//            command[0] = IRQStatus;                 // ?????? 
//            command[1] = IRQMask;   
//        
////            if(SPIMODE)
//                ReadCont(command, 2);               //??IRQ????????????
////            else
////                ReadSingle(command, 1); 
//		}


			while(1)
			{
						command[0] = ChipStateControl;
            command[1] = 0x21;                      // ??RF??,??5V????
            WriteSingle(command, 2);
				
            command[0] = ISOControl;                // ????ISO14443B?????:???106kbps
            command[1] = 0x0C;
            WriteSingle(command, 2);

            delay_ms(5);
            AnticollisionSequenceB(0xB1, 0x04);     //??ISO14443A???????(0x04??16?slots)
            //AnticollisionSequenceB(0xB0, 0x00);   //0x00 ?????slot

            command[0] = ChipStateControl;  
            command[1] = 0x01;
            WriteSingle(command, 2);                // ??RF???? 
            delay_ms(1);

            command[0] = IRQStatus;
            command[1] = IRQMask;   
        
//            if(SPIMODE)                             //??IRQ????????????
                ReadCont(command, 2);
//            else
//                ReadSingle(command, 1); 
			}
}   /* FindTags */

