/**********************************************************************************************************************************************************
* 文 件 名：ISO14443.C
* 功    能：ISO14443A和ISO14443B协议卡片操作函数。包括仿冲撞处理等。
*           本文件包含了ISO14443协议和ISO14443B协议卡片的演示函数。
*           注意：在传输和接收过程中，均需要通过FIFO操作。
* 作    者：EMDOOR
* 日    期：2011年04月13号
**********************************************************************************************************************************************************/
#include "ISO14443.h"
#include "lcd.h"

unsigned char completeUID[14];                      //定义完整的ISO14443协议UID码变量

/******************************************************************************************************************
* 函数名称：SelectCommand()
* 功    能：选择命令将入口参数值select写入到TRF7960的FIFO缓冲区中。
* 入口参数：select       协议串级数量
*           *UID         唯一标识码UID字符串变量
* 出口参数：ret     
* 说    明：若该函数返回数值0，则表示写入操作成功完成。
*******************************************************************************************************************/
char SelectCommand(unsigned char select, unsigned char *UID)
{
    unsigned char j;                                //定义变量
    char ret = 0;                                   //定义返回值变量，并赋值为0
    
    buf[50] = ISOControl;                           //设置选择ISO14443A操作模式为:比特率106kbps，并使能CRC校验
    buf[51] = 0x08;
    WriteSingle(&buf[50], 2);                       //写命令

    /* 给buf寄存器变量赋值 */
    /*====================================================================================================*/
    for(j = 0; j < 5; j++) 
    {
        buf[j + 7] = *(UID + j);
    }
    /*====================================================================================================*/
    
    buf[0] = 0x8f;                                  //配置将要写如FIFO的值
    buf[1] = 0x91;          
    buf[2] = 0x3d;
    buf[3] = 0x00;
    buf[4] = 0x70;
    buf[5] = select;
    buf[6] = 0x70;

    RAWwrite(buf, 12);                              //使用直接写命令写入12字节请求命令数据

    i_reg = 0x01;
    RXTXstate = 1;                                  //设置标志位，其接收位存储于buf[1]起始位置

    while(i_reg == 0x01)                            //等待中断接收完成
    {
    }

    i_reg = 0x01;                                   //恢复标志位
    CounterSet();                                   //定时器开始设置
    CountValue = 0x2000;                            //定时10ms
    StartCounter;                                   //开始定时

    while(i_reg == 0x01)                            //等待中断接收完成
    {
    }                   	
    
    if(!POLLING)
    {
        if(i_reg == 0xFF)                           //接受到应答
        {                 
            if((buf[1] & BIT2) == BIT2)             //UID未接收完整
            {           
                kputchar('(');
                for(j = 1; j < RXTXstate; j++)
                {
                    Put_byte(buf[j]);
                }/* for */

                kputchar(')');
                ret = 1;
                goto FINISH;
            }
            else                                    //UID接收完成
            {               
                kputchar('[');
                for(j = 1; j < RXTXstate; j++)
                {
                    Put_byte(buf[j]);
                }/* for */

                kputchar(']');
                ret = 0;
                goto FINISH;
            }
        }
        else if(i_reg == 0x02)                      //冲撞发生
        {                
            kputchar('[');
            kputchar('z');                          //发生[z]
            kputchar(']');
        }
        else if(i_reg == 0x00)                      //定时器中断
        {             
            kputchar('[');
            kputchar(']');
        }
        else
            ;
    }
    
FINISH:
    return(ret);                                    //返回0，表示该函数成功被执行。
}   /* SelectCommand */

/******************************************************************************************************************
* 函数名称：AnticollisionLoopA()
* 功    能：ISO14443A仿冲撞循环。
* 入口参数：select       协议串级数量
*           NVB          有效字节数量
*           *UID         唯一标识码UID字符串变量
* 出口参数：无    
* 说    明：该函数递归函数，根据ISO14443A卡片UID码串级数量不同，递归调用次数不同。
*******************************************************************************************************************/
void AnticollisionLoopA(unsigned char select, unsigned char NVB, unsigned char *UID)
{
    unsigned char i, lenght, newUID[4], more = 0;
    unsigned char NvBytes = 0, NvBits = 0, Xbits, found = 0;
    unsigned char temp1, temp2;

    buf[50] = ISOControl;                           //禁止接收CRC校验
    buf[51] = 0x88;
    WriteSingle(&buf[50], 2);                       //写入设置值

    RXErrorFlag = 0;                                //清接收错误标志
    CollPoss = 0;                                   //清冲撞位置

    lenght = 5 + (NVB >> 4);                        //得到有效字节数量长度
    if((NVB & 0x0f) != 0x00)
    {
        lenght++;
        NvBytes = (NVB >> 4) - 2;                   //获取有效字节数量
        Xbits = NVB & 0x07;                         //获取有效位数量
        for(i = 0; i < Xbits; i++)
        {
            NvBits = NvBits << 1;
            NvBits = NvBits + 1;                    //由此计算出有效位数量
        }
    }   /* if */

    buf[0] = 0x8f;                                  //准备发送选择命令，复位FIFO缓冲区
    if(NVB == 0x70)                                 //判断是选择命令，带CRC校验
        buf[1] = 0x91;                         
    else                                            //否则为是仿冲撞命令
        buf[1] = 0x90;
    
    buf[2] = 0x3d;
    buf[3] = 0x00;
    buf[4] = NVB & 0xf0;                            //完整字节数量
    if((NVB & 0x07) != 0x00)                        //非完整位数量
        buf[4] |= ((NVB & 0x07) << 1) + 1;
    buf[5] = select;                                //select值为串级标记值可以取0x93,0x95或者0x97
    buf[6] = NVB;                                   //有效位数量
    buf[7] = *UID;
    buf[8] = *(UID + 1);
    buf[9] = *(UID + 2);
    buf[10] = *(UID + 3);

    RAWwrite(&buf[0], lenght);                      //直接写命令到FIFO缓冲区,长度为lenth

    RXTXstate = 1;                                  //设置标志位，其接收位存储于buf[1]起始位置

    i_reg = 0x01;
    while(i_reg != 0x00)                            //等待传输完毕
    {
        CounterSet();
        CountValue = 0x2710;                        //计时 10ms 
        StartCounter;                               //以向上计数模式计时
        LPM0;
    }

    i_reg = 0x01;
    i = 0;
    while((i_reg == 0x01) && (i < 2))               //等待传输完毕，或者延时时间到
    {   
        i++;
        CounterSet();
        CountValue = 0x2710;                        //计时 10ms
        StartCounter;                               //以向上计数模式计时
        LPM0;
    }

    if(RXErrorFlag == 0x02)                         //如果接收错误，则置起中断标志位
    {
        i_reg = 0x02;
    }

    if(i_reg == 0xff)                               //如果中断传送接收完毕
    {
        if(!POLLING)
        {
            kputchar('(');
            for(i = 1; i < 6; i++) Put_byte(buf[i]);
            kputchar(')');
        }
        
        switch(select)                              //根据串级值，选择执行
        {
            case 0x93:                              //串级等级1
            if((buf[1] == 0x88) || (*UID == 0x88))  //UID尚未接收完整
            {
                if(NvBytes > 0)
                {
                    for(i = 0; i < 4; i++)
                    {
                        if(i < (NvBytes - 1))       //将已知的字节和接收到的字节合并成一个完整的UID
                            completeUID[i] = *(UID + i + 1);
                        else if(i == (NvBytes - 1)) //将不完整的位合并到整个UID中
                            completeUID[i] = (buf[i + 2 - NvBytes] &~NvBits) | (*(UID + i + 1) & NvBits);
                        else                        //将接收到的字节添加到UID中
                            completeUID[i] = buf[i + 2 - NvBytes];
                    }
                }   /*if(NvBytes > 0)*/
                else                                //如果有效字节为0，则将有效位合并到UID中
                {
                    completeUID[0] = (buf[2] &~NvBits) | (*UID & NvBits);
                    for(i = 0; i < 4; i++)
                    {
                        completeUID[i + 1] = buf[i + 3];
                    }   /* for */
                }   /* else */

                buf[1] = 0x88;
                for(i = 0; i < 4; i++) 
                    buf[i + 2] = completeUID[i];

                SelectCommand(select, &buf[1]);
                NVB = 0x20;
                more = 1;                           //串级标志位设定，需递归调用
            }
            else                                    //UID接收完全，将UID显示在LCM图形显示屏中
            {
                if(POLLING)
                {
                    found = 1;                      //找到ISO14443A卡片
                    Display_find_tag(1);            //调用该函数，显示检测到的ISO14443A卡片等信息                       		 

                    /* 处理UID第1位数据，并将其在LCM中显示出来 */
                    /*====================================================================================================*/
                    temp1 = (buf[1] &~NvBits) | (*UID & NvBits);                    
                    Display_Char(6, 0, (unsigned char *) (Num + (temp1 >> 4) * 0x10));
                    Display_Char(6, 1, (unsigned char *) (Num + (temp1 & 0x0f) * 0x10));
                    /*====================================================================================================*/
                
                    /* 处理UID第2,3,4,5位数据，并将其在LCM中显示出来 */
                    /*====================================================================================================*/
                    for (temp1 = 2, temp2 = 2; temp1 < 6; temp1++, temp2 += 2)
                    {
                        Display_Char(6, temp2, (unsigned char *) (Num + (buf[temp1] >> 4) * 0x10));
                        Display_Char(6, temp2 + 1, (unsigned char *)(Num + (buf[temp1] & 0x0f) * 0x10));
                    }
                    /*====================================================================================================*/
                }
                else
                {
                    kputchar('[');                  //将UID号发送至上位PC机
                    if(NvBytes > 0)
                    {
                        kputchar('b');
                        for(i = 0; i < 4; i++)
                        {
                            if(i < (NvBytes - 1))   //将已知的字节和接收到的字节组合成完整的UID码
                                Put_byte(*(UID + i + 1));
                            else if(i = (NvBytes - 1))
                                Put_byte((buf[i + 2 - NvBytes] &~NvBits) | (*(UID + i + 1) & NvBits));
                            else
                                Put_byte(buf[i + 2 - NvBytes]);
                        }/* for */
                    }
                    else
                    {
                        Put_byte((buf[1] &~NvBits) | (*UID & NvBits));
                        for(i = 0; i < 4; i++)
                        {
                            Put_byte(buf[i + 2]);
                        }/* for */
                    }/* if-else */
                    kputchar(']');
                /*====================================================================================================*/
                }
            }   /* else */

            select = 0x95;                          //select值为0x95,串级为2
            break;

            case 0x95:                              //串级等级2
            if(buf[1] == 0x88)                      //UID尚未接收完整
            {
                for(i = 0; i < 4; i++)
                {
                    completeUID[i + 4] = buf[i + 2];
                }
                SelectCommand(select, &buf[1]);     //选择命令，将数据写入到FIFO中
                more = 1;                           //串级标志位设定，需递归调用
            }
            else                                    //UID接收完全，将UID显示在LCM图形显示屏中
            {                           		
                for(i = 0; i < 5; i++)
                {
                    completeUID[i + 4] = buf[i + 1];
                }
                
                if(POLLING)
                {
                    found = 1;
                    Display_find_tag(1);            //调用该函数，显示检测到的ISO14443A卡片等信息
                 
                    /* 处理UID第0,1,2位数据，并将其在LCM中显示出来 */
                    /*====================================================================================================*/
                    for (temp1 = 0, temp2 = 0; temp1 < 3; temp1++, temp2 += 2)
                    {
                        Display_Char(6, temp2, (unsigned char *)(Num + (completeUID[temp1] >> 4) * 0x10));
                        Display_Char(6, temp2 + 1, (unsigned char *)(Num + (completeUID[temp1] & 0x0f) * 0x10));
                    }
                    /*====================================================================================================*/
                
                    /* 处理UID第4,5,6,7位数据，并将其在LCM中显示出来 */
                    /* completeUID[3]为ISO14443A协议中的BCC1校验值 */
                    /* completeUID[8]为ISO14443A协议中的BCC2校验值 */
                    /*====================================================================================================*/
                    for (temp1 = 4, temp2 = 6; temp1 < 8; temp1++, temp2 += 2)
                    {
                        Display_Char(6, temp2, (unsigned char *)(Num + (completeUID[temp1] >> 4) * 0x10));
                        Display_Char(6, temp2 + 1, (unsigned char *)(Num + (completeUID[temp1] & 0x0f) * 0x10));
                    }
                    /*====================================================================================================*/
                }
                else
                {
                    kputchar('[');
                    for(i = 0; i < 3; i++)          //发送UID等级1
                        Put_byte(completeUID[i]);
                    Put_byte(completeUID[3]);       //发送UID1的BCC校验码

                    for(i = 4; i < 8; i++)          //发送UID等级2
                        Put_byte(completeUID[i]);
                    Put_byte(completeUID[8]);       //发送UID2的BCC校验码
                    kputchar(']');
                }
                /*====================================================================================================*/
            }

            select = 0x97;                          //select值为0x97,串级为3
            break;

        case 0x97:                                  //串级等级3                
            /* 将缓冲区变量中的数据存储到完整UID变量中 */
            /*====================================================================================================*/
            for(i = 0; i < 5; i++)
            {
                completeUID[i + 8] = buf[i + 1];
            }
            /*====================================================================================================*/

            if(POLLING)
            {
                found = 1;
                Display_find_tag(1);                //调用该函数，显示检测到的ISO14443A卡片等信息
            
                /* 处理UID第0,1,2位数据，并将其在LCM中显示出来 */
                /*====================================================================================================*/
                for (temp1 = 0, temp2 = 0; temp1 < 3; temp1++, temp2 += 2)
                {
                    Display_Char(6, temp2, (unsigned char *)(Num + (completeUID[temp1] >> 4) * 0x10));
                    Display_Char(6, temp2 + 1, (unsigned char *)(Num + (completeUID[temp1] & 0x0f) * 0x10));
                }
                /*====================================================================================================*/
            
                /* completeUID[3]为ISO14443A协议中的BCC1校验值 */
            
                /* 处理UID第4,5,6位数据，并将其在LCM中显示出来 */
                /*====================================================================================================*/
                for (temp1 = 4, temp2 = 6; temp1 < 7; temp1++, temp2 += 2)
                {
                    Display_Char(6, temp2, (unsigned char *)(Num + (completeUID[temp1] >> 4) * 0x10));
                    Display_Char(6, temp2 + 1, (unsigned char *)(Num + (completeUID[temp1] & 0x0f) * 0x10));
                }
                /*====================================================================================================*/
            
                /* completeUID[7]为ISO14443A协议中的BCC2校验值 */
                /* completeUID[12]为ISO14443A协议中的BCC3校验值 */
            
                /* 处理UID第8,9,10,11位数据，并将其在LCM中显示出来 */
                /*====================================================================================================*/
                for (temp1 = 8, temp2 = 12; temp1 < 12; temp1++, temp2 += 2)
                {
                    Display_Char(6, temp2, (unsigned char *)(Num + (completeUID[temp1] >> 4) * 0x10));
                    Display_Char(6, temp2 + 1, (unsigned char *)(Num + (completeUID[temp1] & 0x0f) * 0x10));
                }
                /*====================================================================================================*/
            }
            else
            {
                kputchar('[');
                for(i = 0; i < 3; i++)              //发送UID等级1
                    Put_byte(completeUID[i]);
                Put_byte(completeUID[3]);           //发送UID1的BCC校验码

                for(i = 4; i < 7; i++)              //发送UID等级2
                    Put_byte(completeUID[i]);
                Put_byte(completeUID[7]);           //发送UID2的BCC校验码

                for(i = 8; i < 12; i++)             //发送UID等级3
                    Put_byte(completeUID[i]);
                Put_byte(completeUID[12]);          //发送UID3的BCC校验码
                kputchar(']');
            }
            /*====================================================================================================*/
            
            break;
        }   /* switch */
    }   /* if(i_reg == 0xff) */
    else if(i_reg == 0x02)                          //冲撞发生
    {  
        if(!POLLING)
        {
            kputchar('(');
            kputchar('z');
            kputchar(')');
        }
    }
    else if(i_reg == 0x00)                          //定时器中断
    {               
        if(!POLLING)
        {
            kputchar('(');
            kputchar(')');
        }
    }
    else
        ;

    if(i_reg == 0x02)                               //如果冲撞发生，则进入仿冲撞循环
    {                   	
        CollPoss++;                                 //阅读器返回冲撞位置加1
        for(i = 1; i < 5; i++)
            newUID[i - 1] = buf[i];                 //给新的UID数组赋值

        CounterSet();                               //设置定时器
        CountValue = 100;                           //计时时间为1.2ms
        StartCounter;                               //开始以递增模式计时
        i_reg = 0x01;
        while(i_reg == 0x01)                        //等待RX接收结束或者等待时间到
        {
        }               	

        AnticollisionLoopA(select, CollPoss,newUID);//递归调用AnticollisionLoopA函数
    }   /* if(i_reg == 0x02) */

    if(more)                                        //如果有串级标志设定，则递归调用函数执行仿冲撞命令，来得到7个或者10个字节长度的UID
    {
        AnticollisionLoopA(select, NVB, UID);       //递归调用函数，UID码：被选择后级不同，其他均相同
        if(POLLING)found = 1;                       //找到卡片
    }   /* if(more) */

    if(found)                                       //如果找到卡片，则LED相应协议指示灯亮
    {
        LEDtypeAON();               
        Beep_Waring(1,BeeptypeA);                   //蜂鸣器发出A类型脉冲鸣叫声
    }
    else                                            //如果未找到卡片，则LED熄灭、蜂鸣器不发声
    {
        LEDtypeAOFF();
        BeepOFF();
    }
}   /* AnticollisionLoopA */

/******************************************************************************************************************
* 函数名称：AnticollisionSequenceA()
* 功    能：ISO14443A仿冲撞序列。
* 入口参数：REQA       请求命令
* 出口参数：无    
* 说    明：该函数根据REQA请求命令执行ISO14443A卡片不同操作。
            本脱机实验演示程序仅仅演示了读取UID码，故为0x00，为WUPA唤醒命令。
*******************************************************************************************************************/
void AnticollisionSequenceA(unsigned char REQA)
{
    unsigned char i, select = 0x93, NVB = 0x20;
    
    buf[0] = ModulatorControl;                      // 调制和系统时钟控制：0x21 - 6.78MHz OOK(100%)
    buf[1] = 0x21;
    WriteSingle(buf, 2);

    buf[0] = ISOControl;                            // 设置选择ISO14443A操作模式为:比特率106kbps
    buf[1] = 0x88;                                  // 接收不带CRC校验
    WriteSingle(buf, 2);

    /* 判断REQA值，若为0则是WUPA唤醒命令；若不为0，则为REQA请求命令 */
    /*====================================================================================================*/
    if(REQA) 
        buf[5] = 0x26;                              //发送 REQA 命令 */
    else
        buf[5] = 0x52;                              //发送 WUPA 命令 */
    /*====================================================================================================*/
    
    RequestCommand(&buf[0], 0x00, 0x0f, 1);         //发送请求命令
    IRQCLR();                                       //清中断标志位
    IRQON();                                        //外部中断开启

    if(i_reg == 0xff || i_reg == 0x02)              //如果接收到数据或者冲撞发生
    {
        for(i = 40; i < 45; i++)                    //将 buf 清空
            buf[i] = 0x00;
      
        AnticollisionLoopA(select, NVB, &buf[40]);  //调用仿冲撞循环 AnticollisionLoopA 函数
        if(POLLING) 
            LEDtypeAON();
    }
    else                                            //否则：LED指示灯灭，蜂鸣器关
    {
        LEDtypeAOFF();
    }

    buf[0] = ISOControl;
    buf[1] = 0x08;                                  //重新配置TRF7960，为接收不带CRC校验
    WriteSingle(buf, 2);
    IRQOFF();                                       //中断关闭
}   /* AnticollisionSequenceA */

/******************************************************************************************************************
* 函数名称：Request14443A()
* 功    能：ISO14443A请求命令函数。
* 入口参数：pbuf       请求命令
*           lenght     命令长度
*           BitRate    比特率
* 出口参数：1 成功执行 0错误发生    
* 说    明：请求命令函数，能执行ISO14443A卡片的所有请求操作
*******************************************************************************************************************/
unsigned char Request14443A(unsigned char *pbuf, unsigned char lenght, unsigned char BitRate)
{
    unsigned char index, j, command, RXBitRate, TXBitRate, reg[2];

    TXBitRate = ((BitRate >> 4) & 0x0F) + 0x08;
    RXBitRate = (BitRate & 0x0F) + 0x08;

    reg[0] = ISOControl;
    reg[1] = TXBitRate;
    WriteSingle(reg, 2);

    RXTXstate = lenght;     

    *pbuf = 0x8f;
    *(pbuf + 1) = 0x91;                             //为FIFO的写入设置缓冲区
    *(pbuf + 2) = 0x3d;
    *(pbuf + 3) = RXTXstate >> 4;
    *(pbuf + 4) = RXTXstate << 4;

    if(lenght > 12) lenght = 12;

    RAWwrite(pbuf, lenght + 5);                     //使用直接写模式发送请求命令

    IRQCLR();                                       //清除外部中断标志位
    IRQON();                                        //开启中断

    RXTXstate = RXTXstate - 12;
    index = 18;

    i_reg = 0x01;

    while(RXTXstate > 0)
    {
        LPM0;                                       //进入低功耗模式，在中断时退出
        if(RXTXstate > 9)                           //如果未发送的字节数量大于9
        {                          
            lenght = 10;                            //将其设置成10
        }
        else if(RXTXstate < 1)                      //如果所有的字节已经发送到FIFO中，则从中断返回
        {
            break;                
        }
        else
        {
            lenght = RXTXstate + 1;                 //所有字节已经被发送
        }   /* if */

        buf[index - 1] = FIFO;                      //传送过程中，写入9个或者更少的字节到FIFO中
        WriteCont(&buf[index - 1], lenght);
        RXTXstate = RXTXstate - 9;                  //写9字节到FIFO中
        index = index + 9;
    }   /* while */

    RXTXstate = 1;         
    while(i_reg == 0x01)
    {
    }

    reg[0] = ISOControl;
    reg[1] = RXBitRate;
    WriteSingle(reg, 2);

    command = 0x16;
    DirectCommand(&command);
    command = 0x17;
    DirectCommand(&command);

    i_reg = 0x01;

    CounterSet();
    CountValue = 0xF000;                            //设置60ms等待时间
    StartCounter;                                   //启动定时器

    while(i_reg == 0x01)                            //等待RX接收完成
    {
    }               

    if(i_reg == 0xFF)                               //接收到应答
    {                       
        kputchar('[');
        for(j = 1; j < RXTXstate; j++)
        {
            Put_byte(buf[j]);
        }   /* for */

        kputchar(']');
        return(0);
    }
    else if(i_reg == 0x02)                          //冲撞发生
    {       
        kputchar('[');
        kputchar('z');
        kputchar(']');
        return(0);
    }
    else if(i_reg == 0x00)                          //定时器中断
    {               
        kputchar('[');
        kputchar(']');
        return(1);
    }
    else
        ;

    IRQOFF();
    return(1);
}   /* Request14443A */

/******************************************************************************************************************
* 函数名称：SlotMarkerCommand()
* 功    能：该函数发送ISO14443B协议槽标记命令，该命令同时包括了槽号。
* 入口参数：number       槽号
* 出口参数：无    
* 说    明：无
*******************************************************************************************************************/
void SlotMarkerCommand(unsigned char number)
{
    buf[0] = 0x8f;
    buf[1] = 0x91;
    buf[2] = 0x3d;
    buf[3] = 0x00;
    buf[4] = 0x10;
    RAWwrite(&buf[0], 5);                           //写配置命令值

    buf[5] = 0x3F;
    buf[6] = (number << 4) | 0x05;
    buf[7] = 0x00;
    
    i_reg = 0x01;
    RAWwrite(&buf[5], 3);                           //写配置命令值

    IRQCLR();                                       //清中断标志位
    IRQON();                                        //开中断

    while(i_reg == 0x01)
    {
        CounterSet();                               //定时器设置
        CountValue = 0x9c40;                        //计时时间 40ms
        StartCounter;                               //开始计时
        LPM0;   
    }
}   

/******************************************************************************************************************
* 函数名称：AnticollisionSequenceB()
* 功    能：ISO14443B仿冲撞序列。
* 入口参数：command       请求命令
*           slots           槽号
* 出口参数：无    
* 说    明：该函数根据 command 请求命令和槽号slots执行ISO14443B卡片不同操作。
*******************************************************************************************************************/
void AnticollisionSequenceB(unsigned char command, unsigned char slots)
{
    unsigned char i, collision = 0x00, j, found = 0;
    unsigned int k = 0;
    unsigned char temp1, temp2;

    buf[0] = ModulatorControl;                      // 调制和系统时钟控制：0x20 - 6.78MHz ASK(10%)
    buf[1] = 0x20;
    WriteSingle(buf, 2);

    RXErrorFlag = 0x00;

    buf[0] = 0x8f;
    buf[1] = 0x91;
    buf[2] = 0x3d;
    buf[3] = 0x00;
    buf[4] = 0x30;
    buf[5] = 0x05;
    buf[6] = 0x00;
    //buf[6] = 0x20;                                //AFI 应用族标志值

    if(slots == 0x04)                               //0x04表示16个槽
    {
        EnableSlotCounter();                        //使能槽计数器
        buf[7] |= 0x08;
    }

    buf[7] = slots;

    if(command == 0xB1)                             //如果为0xB1，那么是WUPB唤醒命令
        buf[7] |= 0x08;                             //如果非0xB1，那么是REQB请求命令

    i_reg = 0x01;
    RAWwrite(&buf[0], 8);                           //写入以上8个配置值到FIFO中

    IRQCLR();                                       //清中断标志位
    IRQON();                                        //开中断

    j = 0;
    while((i_reg == 0x01) && (j < 2))               //等待TX发送结束
    {
        j++;
        CounterSet();                               //定时器设置
        CountValue = 0x4E20;                        //计时时间 20ms
        StartCounter;                               //开始计时
        LPM0;
    }   

    i_reg = 0x01;                                   //恢复标志位
    CounterSet();                                   //定时器设置
    CountValue = 0x4E20;                            //计时时间 20ms
    //CountValue = 0x9c40;                          //计时时间 20ms 在晶体为 13.56 MHz的情况下，该值为0x9c40*/
    StartCounter;                                   //开始计时

    for(i = 1; i < 17; i++)                         //1-16个槽轮循
    {
        RXTXstate = 1;                              //应答数据将被存储在buf[1]以后地址中

        while(i_reg == 0x01)                        //等待RX接收完毕
        {               	
            k++;
            if(k == 0xFFF0)
            {
                i_reg = 0x00;
                RXErrorFlag = 0x00;
                break;
            }
        }

        if(RXErrorFlag == 0x02) 
            i_reg = RXErrorFlag;

        if(i_reg == 0xFF)                           //如果接收到PUPI
        {                   	
            if(POLLING)
        {
                found = 1;                          //设置标志位，提示找到ISO14443B协议卡片
                Display_find_tag(2);                //显示检测到的ISO14443A卡片等信息   
            
                for (temp1 = 2, temp2 = 0; temp1 < 6; temp1++, temp2 += 2)
                {
                    Display_Char(6, temp2, (unsigned char *)(Num + (buf[temp1] >> 4) * 0x10));
                    Display_Char(6, temp2 + 1, (unsigned char *)(Num+(buf[temp1] & 0x0f) * 0x10));
                }
            }
            else
            {             
                kputchar('[');
                for(j = 1; j < RXTXstate; j++) Put_byte(buf[j]);
                kputchar(']');
            }
        }
        else if(i_reg == 0x02)                      //冲撞发生
        {               
            if(!POLLING)
            {
                kputchar('[');
                kputchar('z');
                kputchar(']');
            }
            collision = 0x01;
        }
        else if(i_reg == 0x00)                      //超时
        {                   
            if(!POLLING)
        {
                kputchar('[');
                kputchar(']');
            }
        }
        else
            ;

        /* 判断槽号及循环次数，若超过16次，则跳出for循环 */
        /*====================================================================================================*/
        if((slots == 0x00) || (slots == 0x01) || (slots == 0x02) || ((slots == 0x04) && (i == 16))) break;
        /*====================================================================================================*/

        SlotMarkerCommand(i);                       //执行槽标记功能

        i_reg = 0x01;
        if(!POLLING)
        {
            put_crlf();
        }
    }   /* for */

    if(slots == 0x04)                               //如果为16槽，则停止槽计数
        DisableSlotCounter();

    IRQOFF();                                       //关闭中断

    if(found)                                       //如果找到卡片，则LED相应协议指示灯亮           
    {
        LEDtypeBON();
        Beep_Waring(1,BeeptypeB);                   //蜂鸣器发出A类型脉冲鸣叫声
    }
    else                                            //如果未找到卡片，则LED熄灭、蜂鸣器不发声
    {
        LEDtypeBOFF();
        BeepOFF();
    }

    if(collision != 0x00)                           //如果冲撞位不等于0x00，则递归调用16时间槽ISO14443B序列函数
        AnticollisionSequenceB(0x20, 0x02);
}   /* AnticollisionSequenceB */





