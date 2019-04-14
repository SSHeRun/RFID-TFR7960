/******************************************************************************************************************
* 文 件 名：AUTOMATIC.C
* 功    能：侦测阅读器阅读范围内的所有卷标卡片。
*
* 作    者：EMDOOR
* 日    期：2011-9-29
********************************************************************************************************************/
#include <automatic.h>

unsigned char Set_pro1[9]={0x0C,0x00,0x03,0x04,0x10,0x00,0x21,0x01,0x00};                    //??????
//unsigned char Write_Sig[12]={0x0F,0x00,0x03,0x04,0x18,0x00,0x21,0x08,0xFF,0xFF};  //????0x01 ??12345678
//unsigned char Read_Sig[8]={0x0B,0x00,0x03,0x04,0x18,0x00,0x20,0x08};                        //????0x01
//unsigned char Write_AFI[8]={0x0B,0x00,0x03,0x04,0x18,0x00,0x27,0x01};                       //?AFI??01
//unsigned char Write_DSFID[8]={0x0B,0x00,0x03,0x04,0x18,0x40,0x29,0xEE};                     //?DSFID??EE
//unsigned char Get_info[7]={0x0A,0x00,0x03,0x04,0x18,0x00,0x2B};                             //????????

//unsigned char Get_sec[9]={0x0C,0x00,0x03,0x04,0x18,0x00,0x2C,0x01,0x01};                    //?????????0x01,???02(???3??)
unsigned char Get_info[7]={0x0A,0x00,0x03,0x04,0x18,0x00,0x2B};                             //???????
//unsigned char Get_sec[9]={0x0C,0x00,0x03,0x04,0x18,0x00,0x2C,0x01,0x01};                    /????????0x01,???02(???3??)

unsigned char Write_Sig[12]={0x0F,0x00,0x03,0x04,0x18,0x00,0x21};//???
unsigned char Write_AFI[8]={0x0B,0x00,0x03,0x04,0x18,0x00,0x27};//?AFI
unsigned char Write_DSFID[8]={0x0B,0x00,0x03,0x04,0x18,0x40,0x29};//?DSFID


unsigned char Read_Sig[8]={0x0B,0x00,0x03,0x04,0x18,0x00,0x20};//???
unsigned char Get_sec[9]={0x0C,0x00,0x03,0x04,0x18,0x00,0x2C};//???????

//??TRF7961????
void Set_Pro()
{
	    unsigned char i,count;   

      for(i = 0; i < 9; i++)                      //??TRF7961????
      {
          buf[i]=Set_pro1[i];
      }
      
      count = buf[0] - 8;
      WriteSingle(&buf[5], count);
}

//?AFI???
void Write_AFI_Command(unsigned char datas)
{
	 unsigned char i,count;
		Set_Pro();//??????
	  for(i = 0; i < 7; i++)             //???????0x01 12345678
   {
      buf[ i ] = Write_AFI[ i ];
   }
	 buf[7]=datas;
	 count=buf[0]-8;
   RequestCommand(buf,count,0x00,0);//?????? 
}

 //?DSFID??????
void Write_DSFID_Command(unsigned char datas)
{
	unsigned char i,count;
		Set_Pro();//??????
	  for(i = 0; i < 7; i++)             //???????0x01 12345678
   {
      buf[ i ] = Write_DSFID[ i ];
   }
	 buf[7]=datas;
	 count=buf[0]-8;
   RequestCommand(buf,count,0x00,0);//?????? 
}

//????????
void Get_Info_Command()
{
	unsigned char i,count;
		Set_Pro();//??????
	  for(i = 0; i < 7; i++)             //???????0x01 12345678
   {
      buf[ i ] = Get_info[ i ];
   }	 
	 count=buf[0]-8;
   RequestCommand(buf,count,0x00,0);//?????? 
}

//?????????
void Get_sec_Command(unsigned char block,unsigned char datas)
{
	 unsigned char i,count;
	 Set_Pro();//??????
	 for(i = 0; i < 7; i++)             //???????0x01 12345678
   {
      buf[ i ] = Get_sec[ i ];
   }
	 buf[7]=block;
	 buf[8]=datas;
	
	 count=buf[0]-8;
   RequestCommand(buf,count,0x00,0);//?????? 
}

//??????
void Write_Block_Command(unsigned char block,unsigned char datas[2])
{
	  unsigned char i,count;
		Set_Pro();//??????
	  for(i = 0; i < 7; i++)             //???????0x01 12345678
   {
      buf[ i ] = Write_Sig[ i ];
   }
	 buf[7]=block;
	 buf[8]=datas[0];
	 buf[9]=datas[1];
	 count=buf[0]-8;
   RequestCommand(buf,count,0x00,0);//?????? 
}
//??????
void Read_Block_Command(unsigned char block)
{
	unsigned char i,count;
	Set_Pro();//??????
	for(i = 0; i < 7; i++)              //???????0x01
  {
          buf[ i ] = Read_Sig[ i ];
  }
	buf[8]=block;
	count=buf[0]-8;
  RequestCommand(buf,count,0x00,0);//?????? 
}

/******************************************************************************************************************
* 函数名称：FindTags()
* 功    能：根据指定卷标协议类型，设置TRF7960配置各相关寄存器后，进行寻卡操作。
* 入口参数：protocol       指定协议类型
* 出口参数：无
* 说    明：该函数是一个死循环函数，所有的脱机演示执行过程均在此完成。
*******************************************************************************************************************/
void FindTags(void)
{
    unsigned char command[10];                      //定义命令数据暂存缓冲器数组
		unsigned char i, count;
		while(1)
    {
            command[0] = ChipStateControl;          // ??RF??,??5V????
            command[1] = 0x21;
            command[2] = ISOControl;                // ????ISO15693?????:????26.48kbps ???? 1/4(????)
            command[3] = 0x02;
            WriteSingle(command, 4);                // ?4??????TRF7960????

            delay_ms(5);
            flags = 0x06;                           // 16(slot)???
            //flags = 0x26;

            command[0] = 0x04;
						
						count = buf[0] - 9;
						InventoryRequest(command, 0);           // ????????(?????)      

            command[0] = ChipStateControl;          // ??RF????
            command[1] = 0x01;
            WriteSingle(command, 2);
            delay_ms(1);

            command[0] = IRQStatus;                 // ??????
            command[1] = IRQMask;               

                                      
		    ReadCont(command, 2);							   //??IRQ????????????
		
        delay_ms(10);
    }   /* while */
	
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


//			while(1)
//			{
//						command[0] = ChipStateControl;
//            command[1] = 0x21;                      // ??RF??,??5V????
//            WriteSingle(command, 2);
//				
//            command[0] = ISOControl;                // ????ISO14443B?????:???106kbps
//            command[1] = 0x0C;
//            WriteSingle(command, 2);

//            delay_ms(5);
//            AnticollisionSequenceB(0xB1, 0x04);     //??ISO14443A???????(0x04??16?slots)
//            //AnticollisionSequenceB(0xB0, 0x00);   //0x00 ?????slot

//            command[0] = ChipStateControl;  
//            command[1] = 0x01;
//            WriteSingle(command, 2);                // ??RF???? 
//            delay_ms(1);

//            command[0] = IRQStatus;
//            command[1] = IRQMask;   
//        
////            if(SPIMODE)                             //??IRQ????????????
//                ReadCont(command, 2);
////            else
////                ReadSingle(command, 1); 
//			}
}   /* FindTags */

