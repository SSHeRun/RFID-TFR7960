/******************************************************************************************************************
* �� �� ����AUTOMATIC.C
* ��    �ܣ�����Ķ����Ķ���Χ�ڵ����о�꿨Ƭ��
*
* ��    �ߣ�EMDOOR
* ��    �ڣ�2011-9-29
********************************************************************************************************************/
#include <automatic.h>

unsigned char Set_pro[9]={0x0C,0x00,0x03,0x04,0x10,0x00,0x21,0x01,0x00};                    //���ò�������
unsigned char Write_Sig[12]={0x0F,0x00,0x03,0x04,0x18,0x00,0x21,0x08,0xFF,0xFF};          //д���ַ0x01 ����12345678
unsigned char Read_Sig[8]={0x0B,0x00,0x03,0x04,0x18,0x00,0x20,0x08};                         //д���ַ0x01
//unsigned char Write_AFI[8]={0x0B,0x00,0x03,0x04,0x18,0x00,0x27,0x01};                       //дAFI����01
//unsigned char Write_DSFID[8]={0x0B,0x00,0x03,0x04,0x18,0x40,0x29,0xEE};                      //дDSFID����EE
//unsigned char Get_info[7]={0x0A,0x00,0x03,0x04,0x18,0x00,0x2B};                             //��ȡ��Ƭϵͳ��Ϣ
//unsigned char Get_sec[9]={0x0C,0x00,0x03,0x04,0x18,0x00,0x2C,0x01,0x01};                   //��ȡ�鰲ȫ״̬��ַ0x01,������02��ʵ��Ϊ3���飩

/******************************************************************************************************************
* �������ƣ�RFID_Test()
* ��    �ܣ�RFID��Ƭ�ѻ����Ժ�����
* ��ڲ�������
* ���ڲ�������
* ˵    �����ú����ܶԿ�Ƭ���ж�д��������дAFI��дDSFID���ͻ�ȡ��Ƭ��Ϣ��Ϣ�Ȳ���    
*******************************************************************************************************************/
void RFID_test(void)
{
    unsigned char i,count;   

      for(i = 0; i < 9; i++)                      //����TRF7961ͨ�Ų���
      {
          buf[i]=Set_pro[i];
      }
      
      count = buf[0] - 8;
      WriteSingle(&buf[5], count);
      
      // delay_ms(800);

      // for(i = 0; i < 12; i++)             //дһ�����ַΪ0x01 12345678
      // {
      //     buf[ i ] = Write_Sig[ i ];
      // }
      
      for(i = 0; i < 9; i++)              //��һ�����ַΪ0x01
      {
          buf[ i ] = Read_Sig[ i ];
      }
  
      // for(i = 0; i < 8; i++)              //дAFIӦ����
      // {
      //     buf[ i ] = Write_AFI[ i ];
      // }
 
      // for(i = 0; i < 8; i++)              //дDSFID���ݴ洢��ʽ
      // {
      //     buf[i]=Write_DSFID[i];
      // }
   
      // for(i = 0; i < 7; i++)              //��ȡ��Ƭ��Ϣ��Ϣ
      // {
      //     buf[ i ] = Get_info[ i ];
      // }
          
      // for(i = 0; i < 9; i++)              //��ȡ�鰲ȫ״̬   
      // {
      //     buf[ i ] = Get_sec[ i ];
			//	}
      
      count=buf[0]-8;
      RequestCommand(buf,count,0x00,0);//������������  

      
      //delay_ms(800);
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
		unsigned char i, count;
	while(1)
	{
					command[0] = ChipStateControl;          // ����RFʹ�ܣ�ѡ��5V����ģʽ
					command[1] = 0x21;
					command[2] = ISOControl;                // ����ѡ��ISO15693����ģʽΪ:�߱�����26.48kbps �����ز� 1/4(Ĭ��ģʽ)
					command[3] = 0x02;
					WriteSingle(command, 4);                 // д4���ֽ����TRF7960�Ĵ�����

					delay_ms(5);
					flags = 0x06;                            // 16(slot)��ģʽ
					//flags = 0x26;                          // 1(slot)��ģʽ

					command[0] = 0x04;
					
					InventoryRequest(command, 0);          // ����������������(��Ѱ������)      

					command[0] = ChipStateControl;          // �ر�RF���ֵ�·
					command[1] = 0x01;
					WriteSingle(command, 2);
					delay_ms(1);

					command[0] = IRQStatus;                // ���Ĵ�����ֵ
					command[1] = IRQMask;               

																		
					ReadCont(command, 2);							    //��ȡIRQ�ж�״̬�Ĵ������жϱ�־

					RFID_test();
	
	
			delay_ms(10);
	}   /* while */
//	
//		while(1)                                         //ISO14443AЭ���׼
//		{
//						command[0] = ChipStateControl;           // ����RFʹ�ܣ�ѡ��5V����ģʽ
//            command[1] = 0x21;
//            command[2] = ISOControl;                 // ����ѡ��ISO14443A����ģʽΪ:������106kbps
//            command[3] = 0x08;
//            WriteSingle(command, 4);
//            delay_ms(5);
//						
//            AnticollisionSequenceA(0x01);           //ִ��ISO14443A�����³�ײ����
//						
//            command[0] = ChipStateControl;          // ���Ĵ�����ֵ
//            command[1] = 0x01;
//            WriteSingle(command, 2);                 // �ر�RF���ֵ�· 
//            delay_ms(1);

//            command[0] = IRQStatus;                  // ���Ĵ�����ֵ 
//            command[1] = IRQMask;   
//        
////            if(SPIMODE)
//                ReadCont(command, 2);             //��ȡIRQ�ж�״̬�Ĵ������жϱ�־
////            else
////                ReadSingle(command, 1); 
//		}


//			while(1)                                        //ISO14443BЭ���׼
//			{
//						command[0] = ChipStateControl;
//            command[1] = 0x21;                     // ����RFʹ�ܣ�ѡ��5V����ģʽ
//            WriteSingle(command, 2);
//				
//            //command[0] = ISOControl;                // ����ѡ��ISO14443B����ģʽΪ:������106kbps
//            command[1] = 0x0C;
//            WriteSingle(command, 2);

//            delay_ms(5);
//            AnticollisionSequenceB(0xB0, 0x04);     //ִ��ISO14443A�����³�ײ����(0x04��ʾ16��slots)
//            //AnticollisionSequenceB(0xB0, 0x00);  //0x00 ��ʾ������slot

//            command[0] = ChipStateControl;  
//            command[1] = 0x01;
//            WriteSingle(command, 2);               // �ر�RF���ֵ�· 
//            delay_ms(1);

//            command[0] = IRQStatus;
//            command[1] = IRQMask;   
//        
////            if(SPIMODE)                            //��ȡIRQ�ж�״̬�Ĵ������жϱ�־
//                ReadCont(command, 2);
////            else
////                ReadSingle(command, 1); 
//			}
      // while(1){
      //    command[0] = ChipStateControl;          // ����RFʹ�ܣ�ѡ��5V����ģʽ
      //       command[1] = 0x21;
      //       command[2] = ISOControl;                // ����ѡ��Tag-it����ģʽ
      //       command[3] = 0x13;
      //       WriteSingle(command, 4);
      //       delay_ms(5);
      //       flags = 0x00;
      //       command[0] = 0x00;
      //       TIInventoryRequest(command, 0);         //����Ѱ������

      //       command[0] = ChipStateControl;          // �ر�RF���ֵ�· 
      //       command[1] = 0x01;
      //       WriteSingle(command, 2);
      //       delay_ms(1);

      //       command[0] = IRQStatus;                 // ���Ĵ�����ֵ 
      //       command[1] = IRQMask;
        
      //       if(SPIMODE)
      //           ReadCont(command, 2);               //��ȡIRQ�ж�״̬�Ĵ������жϱ�־
      //       else
      //           ReadSingle(command, 1); 
      // }
}   /* FindTags */
