#ifndef _TRF_7960_C_INCLUDED_
#define _TRF_7960_C_INCLUDED_
#include "msp430x12x.h"
#include "TRF7960.h"
char buffer[12];
char rec_length;
char tagtype[2];
char UID[5];
void spi_delay (char loop)
{
  char j;
  for (j = 0x00;j < loop; j++);
  return;
}

//SPI(without ss)
void start_bit (void)
{       SPI_CLK_LOW;
	//_NOP();
	SPI_OUT_LOW;
	//_NOP();
	SPI_CLK_HIGH;
	//_NOP();
	SPI_OUT_HIGH;   
	//_NOP();
	SPI_CLK_LOW;	
}

void stop_bit (void)
{
        SPI_CLK_LOW;
	//_NOP();
	SPI_OUT_HIGH;	
	//_NOP();
	SPI_CLK_HIGH;
	//_NOP();
	SPI_OUT_LOW;	
	//_NOP();
	SPI_CLK_LOW;
}

void byte_send(char sendbyte)
{
  char i,temp;
  //SPI_CLK_LOW;
  temp = sendbyte & 0x80;
  for(i=0;i<8;i++)
  { 
    if(temp)
     { SPI_OUT_HIGH;
     }
    else
     { SPI_OUT_LOW;
     }
    //_NOP();
    SPI_CLK_HIGH;
    //_NOP();
    sendbyte<<=1;
    temp = sendbyte & 0x80;
    SPI_CLK_LOW;
  }
}

char spi_receive(void)
{
  char i,temp=0;
    //SPI_CLK_LOW;
  for(i=0;i<8;i++)
  {
    //_NOP();
    SPI_CLK_HIGH;
    temp<<=1;
    if(SPI_IN)
    {  
      temp|=0x01;
    } 
    else
    {  
     temp&=0xfe;
    }     
    SPI_CLK_LOW;
  }
  return(temp);
}

void single_command_send(char command_code)
{ 
   command_code &= 0x9f;
   command_code |= 0x80;   
   start_bit();
   byte_send(command_code);
   stop_bit();
   return;
}

char single_register_read(char reg_ad)
{ 
   char i;
   reg_ad &=0x1F;
   reg_ad |=0x40;
   start_bit();
   byte_send(reg_ad);
   i=spi_receive();
   stop_bit();   
   return(i);
}

void single_register_write(char reg_ad,char reg_data)
{  
   reg_ad &=0x1F;
   start_bit();
   byte_send(reg_ad);
   byte_send(reg_data);
   stop_bit();

   return;
}
//char FIFO_level(void)
//{ char i;
 // i=single_register_read(0x1C);
  
char FIFO_length( void )
{ char i;
   start_bit();
   byte_send(0x5c);
   i=spi_receive();
   stop_bit();  
   i &= 0x0F;
  return(i);
}

char Clear_FIFO(void)
{  char temp;
   start_bit();
   byte_send(0x8F);
   byte_send(0x4C);
   temp=spi_receive();
   byte_send(0x00);
   stop_bit();
   return(temp);
}
char Read_FIFO(char * buff)
{
   char temp,i;
   temp=FIFO_length();
   temp++;
  
      start_bit();
       byte_send(0x7f);
      //for(i=0;i<temp;i++)
       for(i=0;i<5;i++)
      *(buff+i)=spi_receive();
       stop_bit();
       return(temp);
   
}
    
char IRQ_status( void )
{ 
   char temp;
   SPI_RX_HIGH;
   start_bit();
   byte_send(0x4C);
   temp=spi_receive();
   byte_send(0x00);
   stop_bit();
   SPI_RX_LOW;  
   return(temp);
}

void softinit(void)
{  // char temp; 
    single_command_send(0x03);             //soft init
     spi_delay(0x50); 
     single_register_write(0x0B,0x06);      //3.3V    
     spi_delay(0x20); 
     single_register_write(0x09,0x21);    //CL_SYS 6.78MHz 100%OOK
     spi_delay(0x20);  
     single_register_write(0x00,0x23);//0x21);      //active mode,rf_on,5V system]
     spi_delay(0x20); 
     single_register_write(0x01,0x88);//0x08);      //ISO14443A,bit rate 106kbit/S,rx_crc
     spi_delay(0x20);
     //single_register_write(0x07,0x30);    //no response delay time 37.76*48=1812us
     //delay(0x20); 
     //single_register_write(0x0D,0x3f);      //no_resp   
     single_register_write(0x0D,0x01);       //enable no_resp,disable else
     spi_delay(0x20);
     //temp=single_register_read(0x03);
    // temp |= 0x08;
     //single_register_write(0x03,temp);
     //temp=single_register_read(0x03);
     return;       
}


char data_transceive(char byte_count,char broken_bit,char any_crc,char * buff)
{      
  char j,temp,sended,remain_send;
  char TxLengthByte1,TxLengthByte2; 
  
  Clear_FIFO();
  sended=0x00;
  rec_length=0;  
  remain_send = byte_count;
  TxLengthByte1 = (byte_count & 0xf0)>>4;
  TxLengthByte2 = (byte_count & 0x0f)<<4; 
  if(broken_bit != 0)
  { 
        broken_bit = broken_bit <<1;
        broken_bit &= 0x0E;
        broken_bit |= 0x01;
        TxLengthByte2 |= broken_bit;
        remain_send++;
  }
  if(remain_send==0)
     return(0x00);
  if(remain_send==1)
       remain_send++;
  if(TRF_7960_IRQ != 0)
     return(0x01);
  else
  {
     start_bit();
     //byte_send(0x8f);//reset
     //byte_send(0x90);//transmit without crc
     byte_send(any_crc);//transmit withCRC
     byte_send(0x3D);// length to 1d,1e
     byte_send(TxLengthByte1);    //1d
     byte_send(TxLengthByte2);    //1e
     SPI_TX_HIGH;
     if(remain_send<=12)
     {
       temp=remain_send;
       remain_send=0;
       
     }
     else
     {
       temp=12;
       remain_send=remain_send-12;
       sended=12;
     }
     for(j=0;j<temp;j++)
       byte_send(*(buff+j));
     stop_bit();
     if(remain_send==0)
       SPI_TX_LOW;
     while(remain_send!=0) 
     {
        temp=FIFO_length();
        if(temp<6)
        {
           start_bit();
           byte_send(0x3f);
           if(remain_send<=6)
           {
              temp=remain_send;
              remain_send=0;
              
           }
           else
           {
              temp=6;
              remain_send=remain_send-6;
            }
            for(j=0;j<temp;j++)
              byte_send(*(buff+j+sended));
            stop_bit();
            if(remain_send==0)
            SPI_TX_LOW;
            sended=sended+6;
        }
     }  
     while(TRF_7960_IRQ==0)
             {    _NOP();     }
     temp=Clear_FIFO();
           if((temp & 0x80)==0)
              return(TRF7960_TRANSERR);
     while(1)
     {
            
       
        if(TRF_7960_IRQ!=0)
        {   
            temp=IRQ_status();
            if((temp & 0x01)!=0)
              return(TRF7960_NOTAGERR);
            if((temp & 0x40)!=0)
            {  
              j=Read_FIFO(buffer+rec_length);
              rec_length += j;
              j=single_register_read(0x3d);
              j=single_register_read(0x3e);
               /*if((temp & 0x20)!=0)
               {
                  start_bit();
                  byter_send(0x7f);
                  for(j=0;j<8;j++)
                   *(buff+j+rec_length)=spi_receive();
                  stop_bit();
               }*/
               if((temp & 0x02)!=0)
                  return(TRF7960_ANTICOLLERR);
               if((temp & 0x04)!=0)
                  return(TRF7960_FRAMINGERR);
               if((temp & 0x08)!=0)
                  return(TRF7960_PARITYERR);
               if((temp & 0x10)!=0)
                  return(TRF7960_CRCERR);
  
               else
                  return(TRF7960_OK);
              return(temp);
            }
            else
               return(TRF7960_TRANSERR);
        }
        else
        {    temp=FIFO_length();
              if(temp>6)
              {  start_bit();
                 byte_send(0x7f);
                 for(j=0;j<6;j++)
                   *(buff+j+rec_length)=spi_receive();
                  stop_bit();
                  rec_length +=6;
              }
                 
        }
    }
  }            
}  

void initial_7960 (void)
{   
    //初始化interface
    
    SPI_CLK_SEL;
    SPI_CLK_OUT;
    SPI_CLK_LOW;
        
    SPI_IN_SEL;
    SPI_IN_IN; 
       
    SPI_OUT_SEL;
    SPI_OUT_OUT;
    SPI_OUT_LOW;
    
    SPI_STROBE_SEL;
    SPI_STROBE_IN ;
    
    
    TRF_7960_MOD_SEL;
    TRF_7960_MOD_OUT;
    TRF_7960_MOD_NO_DIRECT;

    TRF_7960_OOK_SEL;    
    TRF_7960_00K_OUT;
    //TRF_7960_ASK;
    TRF_7960_OOK;
    
    TRF_7960_IRQ_SEL;
    TRF_7960_IRQ_IN;   
   
    
    TRF_7960_EN_SEL;
    TRF_7960_EN_OUT;
    TRF_7960_EN_HIGH;

   //FOR TEST
    SPI_TX_SEL;
    SPI_TX_OUT;
    SPI_TX_LOW;
    
    SPI_RX_SEL;
    SPI_RX_OUT;
    SPI_RX_LOW;
 
    spi_delay(0x10);
    softinit();
}

char Judge_Req(char * buff)
{
	char temp1,temp2;
	
	temp1 = *buff;
	temp2 = *(buff + 1);

	if((temp1 == ATQA_BYTE0) && (temp2 == ATQA_BYTE1))
	  return(1);
	else
	  return(0);
}

/********************************************************************************************************/
/*名称: Save_UID                                                                     	                */
/*功能: 该函数实现保存卡片收到的序列号                                                  	        */                           	             
/*												        */											
/*输入:                                                                                                 */
/*       row: 产生冲突的行									 	*/
/*       col: 产生冲突的列										*/						
/*       length: 接収到的UID数据长度                                                                    */
/*                                                                                                     	*/
/*输出:                                                                                                	*/
/*	 N/A                                                   		                		*/
/********************************************************************************************************/
void Save_UID(char row,char col,char length)
{
	char i;
	char temp;
	char temp1;
	
	if(row==0x00 && col==0x00)
		for(i=0;i<length;i++)
			UID[i]=buffer[i];
	else
	{	
		temp=buffer[0];
		temp1=UID[row-1];
		switch(col)
		{
			case 0:
				temp1=0x00;
				row=row+1;
				break;
			case 1:
				temp=temp & 0xFE;
				temp1=temp1 & 0x01;
				break;			
			case 2:
				temp=temp & 0xFC;
				temp1=temp1 & 0x03;
				break;
			case 3:
				temp=temp & 0xF8;
				temp1=temp1 & 0x07;
				break;
			case 4:
				temp=temp & 0xF0;
				temp1=temp1 & 0x0F;
				break;
			case 5:
				temp=temp & 0xE0;
				temp1=temp1 & 0x1F;
				break;
			case 6:
				temp=temp & 0xC0;
				temp1=temp1 & 0x3F;
				break;
			case 7:
				temp=temp & 0x80;
				temp1=temp1 & 0x7F;
				break;
			default:
				break;
		}
		buffer[0]=temp;
		UID[row-1]=temp1 | temp;
		for(i=1;i<length;i++)
		{
			UID[row-1+i]=buffer[i];
		}
	}
}

/********************************************************************************************************/
/*名称: Check_UID                                                                     	               	*/
/*功能: 该函数实现对收到的卡片的序列号的判断                                                       	*/               	             
/*												        */		
/*输入:                                                                                                	*/
/*       N/A                                                                      		 	*/
/*                                                                                                     	*/
/*输出:                                                                                                	*/
/*	 TRUE: 序列号正确                                                 		                */
/*       FALSE: 序列号错误              								*/
/********************************************************************************************************/
char Check_UID(void)
{
	char temp;
	char i;
	
	temp=0x00;
	for(i=0;i<5;i++)
	{
		temp=temp ^ UID[i];
	}
	if(temp==0)
		return(1);
	return(0);
}


/********************************************************************************************************/
/*名称: Set_BitFraming                                                                     	        */
/*功能: 该函数实现对收到的卡片的序列号的判断                                                       	*/               	             
/*												        */		
/*输入:                                                                                                	*/
/*       row: 产生冲突的行                                                                     		*/
/*	 col: 产生冲突的列										*/
/*                                                                                                     	*/
/*输出:                                                                                                	*/
/*       N/A      											*/
/********************************************************************************************************/
void Set_BitFraming(char  row,char  col)
{   
	switch(row)
	{
		case 0: 
			buffer[1]=0x20;
			break;
		case 1:
			buffer[1]=0x30;
			break;
		case 2:
			buffer[1]=0x40;
			break;
		case 3:
			buffer[1]=0x50;
			break;
		case 4:
			buffer[1]=0x60;
			break;
		default:
			break;
	}	
	
	switch(col)
	{
		case 0:
			
			break;
		case 1:
			
			buffer[1]=(buffer[1] | 0x01);
			break;
		case 2:
			
			buffer[1]=(buffer[1] | 0x02);
			break;
		case 3:
			
			buffer[1]=(buffer[1] | 0x03);
			break;
		case 4:
			
			buffer[1]=(buffer[1] | 0x04);
			break;
		case 5:
			
			buffer[1]=(buffer[1] | 0x05);
			break;
		case 6:
			
			buffer[1]=(buffer[1] | 0x06);
			break;
		case 7:
			
			buffer[1]=(buffer[1] | 0x07);
			break;
		default:
			break;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//名称: Request                                                                //
//功能: 该函数实现对放入RC531操作范围之内的卡片的Request操作                   //                       	             
//									       //								
//输入:                                                                        //
//       mode: WUPA(监测所以RC531操作范围之内的卡片)			       //
//	       REQA(监测在RC531操作范围之内不处于HALT状态的空闲卡片)           //
//                                                                             //
//输出:                                                                        //
//	 RC531_NOTAGERR: 无卡                                                  //
//       RC531_OK: 应答正确                                                    //
//	 RC531_REQERR: 应答错误						       //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
char Request(char mode)
{
	char  temp;
	single_register_write(0x01,0x88); 	//关闭rx_crc
	buffer[0]=mode;					//Request模式选择
	temp=data_transceive(/*byte_count*/0,/*broken_bit*/7,/*any_crc*/0x90,buffer);
	_NOP();
        if(temp!=TRF7960_OK)
           return(temp);
        temp=Judge_Req(buffer);				//判断应答信号是否正确
	if (temp==1)
	{
		tagtype[0]=buffer[0];
		tagtype[1]=buffer[1];
		return(TRF7960_OK);
	}
	else
		return(TRF7960_REQERR);
} 

/********************************************************************************************************/
/*名称: AntiColl                                                                     	        	*/
/*功能: 该函数实现对放入RC531操作范围之内的卡片的防冲突检测                                             */                           	             
/*												       	*/											
/*输入:                                                                                                	*/
/*       N/A                                                                        		        */
/*                                                                                                     	*/
/*输出:                                                                                                	*/
/*	RC531_NOTAGERR: 无卡                                                 		                */
/*      RC531_BYTECOUNTERR: 接收字节错误                                                                */
/*	RC531_SERNRERR: 卡片序列号应答错误								*/
/*	RC531_OK: 卡片应答正确                                                                          */
/********************************************************************************************************/
char AntiColl(void)
{
	char temp;
	char i;
	char row,col;
	char pre_row;
	single_register_write(0x01,0x88); 	//关闭rx_crc
        
	row=0;
	col=0;
	pre_row=0;
	single_register_read(0x0E);
	buffer[0]=RF_CMD_ANTICOL;
	buffer[1]=0x20;
        temp=data_transceive(2,0,0x90,buffer);
	while(1)
	{	
		if((temp!=TRF7960_ANTICOLLERR)&&(temp!=TRF7960_OK))
			return(temp);
		Save_UID(row,col,rec_length);     //将收到的UID放入UID数组
                if(temp==TRF7960_OK)
		{
		    temp=Check_UID();			//校验收到的UID
		    if(temp==0)
                        return(TRF7960_SERNRERR);
                    else
			return(TRF7960_OK);
		 }
		else
		{      
		     temp=single_register_read(0x0E);				//读取冲突检测寄存器
		     row=temp/8;
		     col=temp%8;
			buffer[0]=RF_CMD_ANTICOL;
			Set_BitFraming(row+pre_row,col);	//设置待发送数据的字节数
			pre_row=pre_row+row;
			for(i=0;i<pre_row+1;i++)
				buffer[i+2]=UID[i];
			if(col!=0x00)
				row=pre_row+1;
			else
				row=pre_row;
                        _NOP();
			temp=data_transceive(row+1,col,0x90,buffer);
		}
	}
}

/********************************************************************************************************/
/*名称: Select_Card                                                                     	        */
/*功能: 该函数实现对放入RC531操作范围之内的某张卡片进行选择                                             */                           	             
/*												       	*/											
/*输入:                                                                                                	*/
/*       N/A                                                                        		        */
/*                                                                                                     	*/
/*输出:                                                                                                	*/
/*	RC531_NOTAGERR: 无卡                                                 		                */
/*      RC531_PARITYERR: 奇偶校验错                                                                     */
/*	RC531_CRCERR: CRC校验错										*/
/*	RC531_BYTECOUNTERR: 接收字节错误								*/
/*	RC531_OK: 应答正确										*/
/*	RC531_SELERR: 选卡出错										*/
/********************************************************************************************************/
char Select_Card(void)
{
		char temp,i;
		
		buffer[0]=RF_CMD_SELECT;
		buffer[1]=0x70;	
		for(i=0;i<5;i++)
		    buffer[i+2]=UID[i];
		single_register_write(0x01,0x08); 	//开启rx_crc				//开启CRC,奇偶校验校验
		temp=data_transceive(7,0,0x91,buffer);
		
		if(temp!=TRF7960_OK)
			return(temp);
		else
		{	
			if (temp!=1)
				return(TRF7960_BYTECOUNTERR);
			if (*buffer==SAK_BYTE0 || *buffer==SAK_BYTE1)	//判断应答信号是否正确
				return(TRF7960_OK);
			else 
				return(TRF7960_SELERR);
		}
}

#endif