/**********************************************************************************************************************************************************
* 文 件 名：TIRIS.C
* 功    能：TI公司Tag-it协议读写操作驱动函数。
* 
* 作    者：EMDOOR
* 日    期：2011年04月13号
**********************************************************************************************************************************************************/
#include "tiris.h"


/******************************************************************************************************************
* 函数名称：TIInventoryRequest()
* 功    能：主函数入口。
* 入口参数：*mask       命令值
*           length      数据长度
* 出口参数：无
* 说    明：Tag-it总量请求命令
*******************************************************************************************************************/
void TIInventoryRequest(unsigned char *mask, unsigned char length)                                                      	/* 010800030414(req.packet)[00ff] */
{
    unsigned char i = 1, j = 3, command, found = 0;
    unsigned char *PslotNo, slotNo[17];
    unsigned char NewMask[8], Newlength, masksize;
    int size;

    
    buf[0] = RXNoResponseWaitTime;                  // 重新定义接收无应答等待时间
    buf[1] = 0x14;
    buf[2] = ModulatorControl;                      // 调制和系统时钟控制：0x21 - 6.78MHz OOK(100%)
    buf[3] = 0x21;
    WriteSingle(buf, 4);

    slotNo[0] = 0x00;
    EnableSlotCounter();                            //使能槽计数器
    PslotNo = &slotNo[0];   	

    masksize = (((length >> 2) + 1) >> 1);
    size = masksize + 3;                            //标志值+标志长度+命令码+标志位

    buf[0] = 0x8f;
    buf[1] = 0x91;                                  //发送带CRC校验命令
    buf[2] = 0x3d;                                  //写连续模式到0x1D/* write continous from 1D */
    buf[3] = (char) (size >> 4);
    buf[4] = (char) (size << 4);
    buf[5] = 0x00;
    buf[6] = 0x50;                                  //TI协议卷标 SID冲撞命令值

    buf[7] = (length | 0x80);                       //标记长度 masklenght，添加信息标记1
    if(length > 0)
    {
        for(i = 0; i < masksize; i++) 
        buf[i + 8] = *(mask + i);
    }   /* if */

    if(length & 0x04)                               //如果有不完整字节
    {
        buf[4] = (buf[4] | 0x09);                   //4位
        buf[masksize + 7] = (buf[masksize + 7] << 4);       //移动4位到最高有效位MSB
    }

    CounterSet();                                   //设置定时器
    CountValue = count1ms * 20;                     //计时时间为 20ms
    i_reg = 0x01;
    IRQCLR();                                       //清中断标志位
    RAWwrite(&buf[0], masksize + 8);                //写入FIFO
    IRQON();                                        //IRQ中断开启
    StartCounter;                                   //开始以递增模式计时
    LPM0;                                           //等待TX中断结束

    for(i = 1; i < 17; i++)                         //16 槽模式
    {
        RXTXstate = 1;                              //接收发送状态寄存器赋初值，其接收位存储于buf[1]起始位置
        
        i_reg = 0x01;
        j = 0;
        while((i_reg == 0x01) && (j < 2))           //等待RX接收中断完
        {   				
            j++;
            CounterSet();                           //设置定时器
            CountValue = count1ms * 20;             //计时时间为 20ms
            StartCounter;                           //开始以递增模式计时
            LPM0;
        }
    
        if(i_reg == 0xFF)                           //如果字节已经是最后字节，接收SID数据到缓冲区中
        {                                   
            if(POLLING)
            {
                found = 1;                          //找到卷标标志位
            }
            else
            {
                kputchar('[');
                for(j = 1; j < 11; j++) Put_byte(buf[j]);
                kputchar(']');
            }
        }
        else if(i_reg == 0x02)                      //冲撞发生
        {
            if(!POLLING)
            {
                PslotNo++;                          //则冲撞位标记值递加1
                *PslotNo = i;
            }
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
        
        command = Reset;                            //在接收下一个槽之前，使用直接命令复位FIFO
        DirectCommand(&command);

        if(i < 16)                                  //如果未循环16个槽，则需要发送EOF命令(下个槽)
        {   				
            command = TransmitNextSlot;
            DirectCommand(&command);
        }
        if(!POLLING)
        {
            put_crlf();   
        }
    }   /* for */

    DisableSlotCounter();                           //禁止槽计数器

    if(found)                                       //如果找到卡片，则LED相应协议指示灯亮
    {
        LEDtagitON();
        
    }
    else
    {
        LEDtagitOFF();                              //如果未找到卡片，则LED熄灭、蜂鸣器不发声
       
    }

    Newlength = length + 4;                         //标记长度为4比特位倍数
    masksize = (((Newlength >> 2) + 1) >> 1) - 1;

    while(*PslotNo != 0x00)
    {
        *PslotNo = *PslotNo - 1;                    //槽计数器从1到16计数

        for(i = 0; i < 4; i++) 
            NewMask[i] = *(mask + i);               //首先将标记值拷贝到新标记数组中

        if((Newlength & BIT2) == 0x00) 
            *PslotNo = *PslotNo << 4;

        NewMask[masksize] |= *PslotNo;              //标记值被改变/* the mask is changed */
        TIInventoryRequest(&NewMask[0], Newlength); //递归调用 TIInventoryRequest 函数

        PslotNo--;                                  //槽递减
    }   /* while */

    IRQOFF();                                       //仿冲撞过程结束，关闭中断
}   /* TIInventoryRequest */
