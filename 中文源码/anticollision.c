/**********************************************************************************************************************************************************
* 文 件 名：ANTICOLLISION.C
* 功    能：ISO15693协议卡片操作函数。包括仿冲撞处理等。
*           本文件包含了ISO15693协议的演示函数。
*           注意：在传输和接收过程中，均需要通过FIFO操作。
* 作    者：EMDOOR
* 日    期：2011年04月13号
**********************************************************************************************************************************************************/
#include "anticollision.h"
#include "lcd.h"


unsigned char POLLING;                              //定义脱机或者连机标志位:0-为连机USB模式;1-为脱机LCD模式
unsigned char Found_tag;                            //定义是否检测到卡片全局变量
unsigned char test_no;                              //定义RFID卡片测试项目序列号
unsigned char rssival;                              //定义检测到的卡片接收信号强度值


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
    buf[41] |= BIT0;                                //在缓冲区寄存器0x41位置设置BIT0有效
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
    unsigned char i = 1, j=3, command[2], NoSlots, found = 0;
    unsigned char *PslotNo, slotNo[17];
    unsigned char NewMask[8], NewLenght, masksize;
    int size;
    unsigned int k = 0;
    unsigned char temp1, temp2;

    buf[0] = ModulatorControl;                      // 调制和系统时钟控制：0x21 - 6.78MHz OOK(100%)
    buf[1] = 0x21;
    WriteSingle(buf, 2);
 
    /* 如果使用SPI串行模式的低数据率，那么 RXNoResponseWaitTime 需要被重新设置 */
    /*====================================================================================================*/
    if(SPIMODE)
    {
        if((flags & BIT1) == 0x00)                  //低数据比特率
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
    }
    /*====================================================================================================*/
    
    slotNo[0] = 0x00;

    if((flags & BIT5) == 0x00)                      //位5标志位指示槽的数量
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
    ReadCont(command, 1);

    CounterSet();                                   //设置定时器
    CountValue = count1ms * 20;                     //计时时间为 20ms
    IRQCLR();                                       //清中断标志位
    IRQON();                                        //中断开启

    RAWwrite(&buf[0], masksize + 8);                //将数据写入到FIFO缓冲区中

    i_reg = 0x01;                                   //设置中断标志值
    StartCounter;                                   //开始以递增模式计时
    LPM0;                                           //等待TX发送结束

    for(i = 1; i < NoSlots; i++)                    //寻卡循环1个槽或者16个槽
    {       
        /* 初始化全局计数器 */
        /*====================================================================================================*/
        RXTXstate = 1;                              //设置标志位，其接收位存储于buf[1]起始位置
        CounterSet();                               //设置定时器
        CountValue = count1ms * 20;                 //计时时间为 20ms 
        //CountValue = 0x4E20;                      
        StartCounter;                               //开始以递增模式计时
        k = 0;
        LPM0;
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
            if(POLLING)
            {
                found = 1;
                Display_find_tag(0);                //LCM显示找到的协议卷标卡片信息
            
                for(temp1 = 10, temp2 = 0; temp1 > 2; temp1--, temp2 += 2)
                {
                    Display_Char(6, temp2, (unsigned char *)(Num + (buf[temp1] >> 4) * 0x10));
                    Display_Char(6, temp2 + 1, (unsigned char *)(Num+(buf[temp1] & 0x0F) * 0x10));
                }
                    
                rssival= 0x07 & (command[0] >> 3);   //显示接收到的信号强度
                Display_Rssi(rssival);
            }
            else
            {
                kputchar('[');                      //发送符号 [               
                for(j = 3; j < 11; j++)
                {
                    Put_byte(buf[j]);               //发送ISO15693 UID码
                }
                kputchar(',');
                Put_byte(command[0]);               //发送RSSI接收信号强度
                kputchar(']');  
            }
        }
        else if(i_reg == 0x02)                      //如果有冲撞发生
        {  
            if(!POLLING)
            {
                kputchar('[');
                kputchar('z');                      //发送 z
                kputchar(',');
                Put_byte(command[0]);               //发送RSSI接收信号强度
                kputchar(']');
            }
            PslotNo++;
            *PslotNo = i;
        }
        else if(i_reg == 0x00)                      //如果定时时间到，中断发生
        { 
            if(!POLLING)
            {
                kputchar('[');
                kputchar(',');
                Put_byte(command[0]);               //发送RSSI接收信号强度
                kputchar(']');
            }   
        }
        else
            ;

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

        if(!POLLING)
    {
            put_crlf();                             //发送回车换行命令
        }
    }   /* for */

    if(found)                                       //如果找到卡片，则LED相应协议指示灯亮
    {                       
        LED15693ON();
        Beep_Waring(1,Beep15693);                   //蜂鸣器发出ISO15693类型脉冲鸣叫声
        Found_tag= 1;
    }
    else
    {
        LED15693OFF();                              //如果未找到卡片，则LED熄灭、蜂鸣器不发声
        found = 0;
        BeepOFF();
    Found_tag = 0;
    }

    NewLenght = lenght + 4;                         //标记长度为4比特位倍数
    masksize = (((NewLenght >> 2) + 1) >> 1) - 1;

    /* 如果是16个槽模式，及槽指针不为0x00, 则递归调用本函数，再次寻找卡片 */
    /*====================================================================================================*/
    while((*PslotNo != 0x00) && (NoSlots == 17)) 
    {
        *PslotNo = *PslotNo - 1;
        for(i = 0; i < 8; i++) NewMask[i] = *(mask + i);            //首先将标记值拷贝到新标记数组中

        if((NewLenght & BIT2) == 0x00) *PslotNo = *PslotNo << 4;

        NewMask[masksize] |= *PslotNo;                              //标记值被改变
        InventoryRequest(&NewMask[0], NewLenght);                   //递归调用 InventoryRequest 函数
        PslotNo--;                                                  //槽递减
    }   /* while */
    /*====================================================================================================*/
    
    IRQOFF();                                                       //仿冲撞过程结束，关闭中断
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
    unsigned char index, j, command;                //定义变量
    unsigned char temp2;
    
    RXTXstate = lenght;                             

    *pbuf = 0x8f;
    if(noCRC) 
        *(pbuf + 1) = 0x90;                         //传输不带CRC校验
    else
        *(pbuf + 1) = 0x91;                         //传输带CRC校验
    
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

    RAWwrite(pbuf, lenght + 5);                     //以直接写FIFO模式发送命令

    IRQCLR();                                       //清中断标志位
    IRQON();

    RXTXstate = RXTXstate - 12;
    index = 17;

    i_reg = 0x01;
    while(RXTXstate > 0)
    {
        LPM0;                                       //进入低功耗模式，并退出中断
        if(RXTXstate > 9)                           //在RXTXstate全局变量中未发送的字节数量如果大于9
        {                       
            lenght = 10;                            //长度为10，其中包括FIFO中的9个字节及1个字节的地址值
        }
        else if(RXTXstate < 1)                      //如果该值小于1，则说明所有的字节已经发送到FIFO中，并从中断返回
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

    RXTXstate = 1;                                  //设置标志位，其接收位存储于buf[1]起始位置

    while(i_reg == 0x01)                            //等待传送结束
    {
        CounterSet();                               //定时器设置
        CountValue = 0xF000;                        //计时时间 60ms
        StartCounter;                               //开始以递增模式计时
        LPM0;
    }

    i_reg = 0x01;
    CounterSet();                                   //定时器设置
    CountValue = 0xF000;                            //计时时间 60ms
    StartCounter;                                   //开始以递增模式计时

    /* 如果中断标志位错误，则先复位后发送下个槽命令 */
    /*====================================================================================================*/
    if((((buf[5] & BIT6) == BIT6) && ((buf[6] == 0x21) || (buf[6] == 0x24) || (buf[6] == 0x27) || (buf[6] == 0x29)))
    || (buf[5] == 0x00 && ((buf[6] & 0xF0) == 0x20 || (buf[6] & 0xF0) == 0x30 || (buf[6] & 0xF0) == 0x40)))
    {
        delay_ms(20);
        command = Reset;
        DirectCommand(&command);
        command = TransmitNextSlot;
        DirectCommand(&command);
    }   /* if */
    /*====================================================================================================*/
    
    while(i_reg == 0x01)                            //等待接收完毕
    { 
    }
    
    if(POLLING)                                     //?如果是脱机模式且在测试情况下
    {
        if(Found_tag)
        {
            if(i_reg == 0xFF)                       //接收到应答
            {
                if((buf[1]) == 0x00)                //操作成功 
                {
                    Display_Char(6, 13, (unsigned char *)CO);           //显示OK
                    Display_Char(6, 14, (unsigned char *)CK);
                    Beep_Waring(1, Beep15693); 
                }
                else                                //操作失败
                {
                    Display_Char(6,10,(unsigned char *)Num+0x0E*0x10);  //显示ERROR
                    Display_Char(6, 11, (unsigned char *)CR);
                    Display_Char(6, 12, (unsigned char *)CR);
                    Display_Char(6, 13, (unsigned char *)CO);
                    Display_Char(6, 14, (unsigned char *)CR);
                    Beep_Waring(3, Beep15693);      //蜂鸣器鸣叫报警声音
                }
            }
        }
        
          switch(test_no)
        {
            case 0:                                 //写一个块
                break;
            case 1:                                 //读一个块
                for(j = 2, temp2 = 15; j < RXTXstate; j++, temp2 -= 2)
                {   //显示读取块数据
                    Display_Char(2, temp2 - 1, (unsigned char *)(Num + (buf[j] >> 4) * 0x10));
                    Display_Char(2, temp2, (unsigned char *)(Num+(buf[j] & 0x0F) * 0x10));
                }                                   
                break;
            case 2:                                 //写AFI
                break;
            case 3:                                 //写DSFID
                break;
            case 4:                                 //获取系统信息
                for(j = 11, temp2 = 9; j < RXTXstate - 1; j++, temp2 += 2)
                {   //显示获取到的卡片系统信息
                    Display_Char(2, temp2-1, (unsigned char *)(Num + (buf[j] >> 4) * 0x10));
                    Display_Char(2, temp2, (unsigned char *)(Num+(buf[j] & 0x0F) * 0x10));
                }                                 
                break;
            case 5:                                 //获取块安全状态
                 for(j = 2, temp2 = 15; j < RXTXstate; j++, temp2 -= 2)
                {   //显示快安全状态信息
                    Display_Char(2, temp2 - 1, (unsigned char *)(Num + (buf[j] >> 4) * 0x10));
                    Display_Char(2, temp2, (unsigned char *)(Num+(buf[j] & 0x0F) * 0x10));
                }                                   
                break;
            default:break;
        }
    }
    
  
    
     if(!POLLING)
    {
        switch(noCRC)                               //是否带有CRC校验
        {
            case 0:
            if(i_reg == 0xFF)                       //接收到应答
            {       
                kputchar('[');                      //发送[]
                for(j = 1; j < RXTXstate; j++)
                {
                    Put_byte(buf[j]);
                }   /* for */

                kputchar(']');
                return(0);
            }
            else if(i_reg == 0x02)                  //冲撞发生
            {      
                kputchar('[');                      //发送[z]
                kputchar('z');
                kputchar(']');
                return(0);
            }
            else if(i_reg == 0x00)                  //定时时间到
            {      
                kputchar('[');
                kputchar(']');
                return(1);
            }
            else
            ;
            break;

            case 1:
            if(i_reg == 0xFF)                       //接收到应答
                        {
                kputchar('(');                      //发送（）
                for(j = 1; j < RXTXstate; j++)
                {
                    Put_byte(buf[j]);
                }   /* for */

                kputchar(')');
                return(0);
            }
            else if(i_reg == 0x02)                  //冲撞发生
            {        
                kputchar('(');                      //发送(z)
                kputchar('z');
                kputchar(')');
                return(0);
            }
            else if(i_reg == 0x00)                  //定时时间到
            {   
                kputchar('(');
                kputchar(')');
                return(1);
            }
            else
            ;
            break;
        }   /* switch */
    }
    
    IRQOFF();                                       //关闭中断
    return(1);                                      //函数全部执行完毕，返回 1
}   /* RequestCommand */



