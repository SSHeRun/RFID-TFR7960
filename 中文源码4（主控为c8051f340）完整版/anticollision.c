/***********************************************************************************************************************
* 文 件 名：ANTICOLLISION.C
* 功    能：ISO15693协议卡片操作函数。包括仿冲撞处理等。
*           本文件包含了ISO15693协议的演示函数。
*           注意：在传输和接收过程中，均需要通过FIFO操作。
* 作    者：EMDOOR
* 日    期：2011-9-29
*************************************************************************************************************************/
#include <anticollision.h>


#define DBG  0
#define LEDOFF    P2 = 0X04
#define LEDON     P2 = 0XFB
unsigned char Found_tag;                            //定义是否检测到卡片全局变量
unsigned char rssival;                              //定义检测到的卡片接收信号强度值

//C8051F与STM32间通信协议数据结构的定义

//----------------------------------------------------------------
//*发送帧
//----------------------------------------------------------------


//----------------------------------------------------------------


/******************************************************************************************************************
* 函数名称：EnableSlotCounter()
* 功    能：使能槽计数功能。
* 入口参数：无
* 出口参数：无     
* 说    明：该函数使能槽计数功能，用于多个槽时。
*******************************************************************************************************************/
void EnableSlotCounter(void)
{
    buf[41] = IRQMask;                              //下个计数槽
    buf[40] = IRQMask;
    ReadSingle(&buf[41], 1);                        //读取缓冲区数据
    buf[41] |= 0X01;                                //在缓冲区寄存器0x41位置设置BIT0有效
    WriteSingle(&buf[40], 2);
}

/******************************************************************************************************************
* 函数名称：DisableSlotCounter()
* 功    能：禁止槽计数功能。
* 入口参数：无
* 出口参数：无     
* 说    明：该函数使槽计数功能停止。
*******************************************************************************************************************/
void DisableSlotCounter(void)
{
    buf[41] = IRQMask;                              //下个计数槽
    buf[40] = IRQMask;
    ReadSingle(&buf[41], 1);                        //读取缓冲区数据
    buf[41] &= 0xfe;                                //在缓冲区寄存器0x41位置设置BIT0无效
    WriteSingle(&buf[40], 2);
}

/******************************************************************************************************************
* 函数名称：InventoryRequest()
* 功    能：ISO15693协议卡片总量请求命令。
* 入口参数：*mask       标记命令
*           lenght      命令长度
* 出口参数：无     
* 说    明：执行该函数可以使ISO15693协议标准总量命令循环16时间槽或者1个时间槽.
*           其中：0x14表示16槽；0x17表示1个槽。
*           注意：在脱机模式下，接收到UID码将被显示到LCM图形显示屏上。
*******************************************************************************************************************/
void InventoryRequest(unsigned char *mask, unsigned char lenght)
{
    unsigned char i = 1, j=3, command[2], NoSlots;
    unsigned char *PslotNo, slotNo[17];
    unsigned char NewMask[8], NewLenght, masksize;
    int size;
    unsigned int k = 0;

    buf[0] = ModulatorControl;                      // 调制和系统时钟控制：0x21 - 6.78MHz OOK(100%)
    buf[1] = 0x21;
    WriteSingle(buf, 2);
 
 /* 如果使用SPI串行模式的低数据率，那么 RXNoResponseWaitTime 需要被重新设置 */
/*====================================================================================================*/
  
        if((flags & 0x02) == 0x00)                  //低数据比特率
        {
            buf[0] = RXNoResponseWaitTime;
            buf[1] = 0x2F;
            WriteSingle(buf, 2);
        }
        else                                        //高数据比特率
        {
            buf[0] = RXNoResponseWaitTime;
            buf[1] = 0x13;
            WriteSingle(buf, 2);
        }
 /*====================================================================================================*/
    
    slotNo[0] = 0x00;

    if((flags & 0x20) == 0x00)                      //位5标志位指示槽的数量
    {                       
        NoSlots = 17;                               //位5为0x00，表示选择16槽模式
        EnableSlotCounter();
    }
    else                                            //如果位5不为0x00，表示选择1个槽模式
        NoSlots = 2;

    PslotNo = &slotNo[0];                           //槽数量指针
    
    /* 如果lenght是4或者8，那么masksize 标记大小值为 1  */
    /* 如果lenght是12或者16，那么masksize 标记大小值为 2，并依次类推 */
    /*====================================================================================================*/
    masksize = (((lenght >> 2) + 1) >> 1);      
    /*====================================================================================================*/
    
    size = masksize + 3;                            // mask value + mask lenght + command code + flags

    buf[0] = 0x8f;
    buf[1] = 0x91;                                  //发送带CRC校验
    buf[2] = 0x3d;                                  //连续写模式
    buf[3] = (char) (size >> 8);
    buf[4] = (char) (size << 4);
    buf[5] = flags;                                 //ISO15693 协议标志flags
    buf[6] = 0x01;                                  //仿冲撞命令值

    /* 可以在此加入AFI应用族标识符 */

    buf[7] = lenght;                                //标记长度 masklenght
    if(lenght > 0)
    {
        for(i = 0; i < masksize; i++) 
            buf[i + 8] = *(mask + i);
    }                   

    command[0] = IRQStatus;
    command[1] = IRQMask;                           //虚拟读(Dummy read)
    ReadCont(command, 2);

    Timer0_Delay(20);                    			//计时时间为 20ms
                                                    //此处没有加StartCounter();
    IRQCLR();                                       //清中断标志位
    IRQON();                                        //中断开启

    //RAWwrite(&buf[0], masksize + 8);                //将数据写入到FIFO缓冲区中
		RAWwrite(buf, masksize + 8);  

    i_reg = 0x01;                                   //设置中断标志值
    StartCounter();                                   //开始以递增模式计时
    PCON |=0X01;                                           //等待TX发送结束

    for(i = 1; i < NoSlots; i++)                    //寻卡循环1个槽或者16个槽
    {       
        /* 初始化全局计数器 */
        /*====================================================================================================*/
        RXTXstate = 1;                              //设置标志位，其接收位存储于buf[1]起始位置
        Timer0_Delay(20);                //计时时间为 20ms                      
        StartCounter();                               //开始以递增模式计时
        k = 0;
        PCON |=0X01;
        /*====================================================================================================*/
        
        while(i_reg == 0x01)                        //等待RX接收结束
        {           
            k++;

            if(k == 0xFFF0)
            {
               i_reg = 0x00;
               RXErrorFlag = 0x00;
               break;
            }
        }
				
        command[0] = RSSILevels;                    //读取信号强度值 RSSI
        ReadSingle(command, 1);

        if(i_reg == 0xFF)                           //如果字节已经是最后字节，接收到UID数据
        {     		
            LEDON;							 
            for(j = 10; j>=3; j--)
            {
                send_byte( buf[j] );               //发送ISO15693 UID码
            }
            LEDOFF;
        }
        else if(i_reg == 0x00)
        {	

        }
        else;

        command[0] = Reset;                         //在接收下一个槽之前，使用直接命令复位FIFO
        DirectCommand(command);

        if((NoSlots == 17) && (i < 16))             //如果在16个槽模式下，未循环16个槽，则需要发送EOF命令(下个槽)
        {                   
            command[0] = StopDecoders;
            DirectCommand(command);                 //停止解碼
            command[0] = RunDecoders;               
            DirectCommand(command);             
            command[0] = TransmitNextSlot;
            DirectCommand(command);                 //传送下一个槽
        }
        else if((NoSlots == 17) && (i == 16))       //如果在16个槽模式下，循环了16个槽，则需要发送停止槽计数命令
        {                   
            DisableSlotCounter();                   //停止槽计数
        }
        else if(NoSlots == 2)                       //如果是单个槽模式，则跳出本 for 循环
            break;
    }   /* for */

		
    NewLenght = lenght + 4;                         //标记长度为4比特位倍数
    masksize = (((NewLenght >> 2) + 1) >> 1) - 1;

    /* 如果是16个槽模式，及槽指针不为0x00, 则递归调用本函数，再次寻找卡片 */
    /*====================================================================================================*/
    while((*PslotNo != 0x00) && (NoSlots == 17)) 
    {		
        *PslotNo = *PslotNo - 1;
        for(i = 0; i < 8; i++) NewMask[i] = *(mask + i);            //首先将标记值拷贝到新标记数组中

        if((NewLenght & 0x04) == 0x00) *PslotNo = *PslotNo << 4;

        NewMask[masksize] |= *PslotNo;                              //标记值被改变
        InventoryRequest(&NewMask[0], NewLenght);                   //递归调用 InventoryRequest 函数
        PslotNo--;     
	                                            //槽递减
    }   /* while */
    /*====================================================================================================*/
    
    IRQOFF();    
	                                               //仿冲撞过程结束，关闭中断
}   /* InventoryRequest */
/******************************************************************************************************************
* 函数名称：RequestCommand()
* 功    能：卡片协议请求命令及处理和计时阅读器到卡片的应答。
* 入口参数：*pbuf           命令值
*           lenght          命令长度
*           brokenBits      不完整字节的位数量
*           noCRC           是否有CRC校验
* 出口参数：1     
* 说    明：该函数为协议请求命令，若返回1，则说明该函数成功执行，若返回0或者不返回，则异常。
*******************************************************************************************************************/
unsigned char RequestCommand(unsigned char *pbuf, unsigned char lenght, unsigned char brokenBits, char noCRC)
{
    unsigned char index, command;                 //定义变量
    unsigned char j;
	
    RXTXstate = lenght;                             
			
    *pbuf = 0x8f;
    if(noCRC) 
        *(pbuf + 1) = 0x90;                         //传输不带CRC校验
    else
        *(pbuf + 1) = 0x91;                          //传输带CRC校验
    
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

    RAWwrite(pbuf, lenght + 5);                    //以直接写FIFO模式发送命令
		
    IRQCLR();                                    //清中断标志位
    IRQON();

    RXTXstate = RXTXstate - 12;
    index = 17;
		
    i_reg = 0x01;
    while(RXTXstate > 0)
    {
			
        //LPM0;                                       //进入低功耗模式，并退出中断
		PCON |=0X01;
        if(RXTXstate > 9)                            //在RXTXstate全局变量中未发送的字节数量如果大于9
        {                       
            lenght = 10;                            //长度为10，其中包括FIFO中的9个字节及1个字节的地址值
        }
        else if(RXTXstate < 1)                     //如果该值小于1，则说明所有的字节已经发送到FIFO中，并从中断返回
        {
            break;
        }
        else                                        //所有的值已经全部被发送
        {
            lenght = RXTXstate + 1;         
        }   /* if */

        buf[index - 1] = FIFO;                      //向FIFO缓冲器中写入9个或者更少字节的数据，将用于发送
        WriteCont(&buf[index - 1], lenght);
        RXTXstate = RXTXstate - 9;                  //写9个字节到FIFO中
        index = index + 9;
    }   /* while */
		
		
		
    RXTXstate = 1;                                 //设置标志位，其接收位存储于buf[1]起始位置
		
    while(i_reg == 0x01)                            //等待传送结束
    {				
                                                    //定时器设置
        Timer0_Delay(60);                      	     //计时时间 60ms
        StartCounter();                              //开始以递增模式计时
        PCON |= 0x01;
    }
		
    i_reg = 0x01;
    Timer0_Delay(60);                            	
    StartCounter();                                 

		
     /* 如果中断标志位错误，则先复位后发送下个槽命令 */
    /*====================================================================================================*/
    if(((buf[5] & 0x40) == 0x40) && ((buf[6] == 0x21) || (buf[6] == 0x24) || (buf[6] == 0x27) || (buf[6] == 0x29))||(buf[5] == 0x00 && ((buf[6] & 0xF0) == 0x20 || (buf[6] & 0xF0) == 0x30 || (buf[6] & 0xF0) == 0x40)))
    {
            
    delay_ms(20);        
    command = Reset;
    DirectCommand(&command);
        
    command = TransmitNextSlot;
    DirectCommand(&command);
    }

    
    while(i_reg == 0x01)                        //等待RX接收结束
    {           
        
    }
				
   	
	 if(i_reg == 0xFF)                      //接收到应答
        {
            if((buf[1]) == 0x00)                //操作成功 
            {
                send_cstring("ojbk");
                send_crlf();
            }
            else                              //操作失败
            {
                send_cstring("shibai");
                send_crlf();
        }
        }
			else if(i_reg == 0x02)
		{		 //冲撞发生
			sendchar('[');
			sendchar('z');
			sendchar(']');
			return(0);
		}
		else if(i_reg == 0x00)
		{		 //定时时间到
//			sendchar('[');
//			sendchar('i');
//			sendchar(']');
			return(1);
		}
		else
			;
		
		for(j = 1; j < RXTXstate; j++)
		{   
				send_byte(buf[j]);
		}
		send_crlf();                            

  
    IRQOFF();                                       //关闭中断
    return(1);                                      //函数全部执行完毕，返回 1
}   /* RequestCommand */

