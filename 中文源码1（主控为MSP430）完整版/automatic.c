/**********************************************************************************************************************************************************
* 文 件 名：AUTOMATIC.C
* 功    能：侦测阅读器阅读范围内的所有卷标卡片。
*
* 作    者：EMDOOR
* 日    期：2011年04月13号
**********************************************************************************************************************************************************/
#include "automatic.h"
#include "lcd.h"


#define RFID_TEST       1                           //定义宏

unsigned char   protocol;                           //定义协议类型寄存器变量

unsigned char Set_pro[9]={0x0C,0x00,0x03,0x04,0x10,0x00,0x21,0x01,0x00};                    //设置参数数据
unsigned char Write_Sig[12]={0x0F,0x00,0x03,0x04,0x18,0x40,0x21,0x01,0x78,0x56,0x34,0x12};  //写块地址0x01 数据12345678
unsigned char Read_Sig[8]={0x0B,0x00,0x03,0x04,0x18,0x00,0x20,0x01};                        //写块地址0x01
unsigned char Write_AFI[8]={0x0B,0x00,0x03,0x04,0x18,0x40,0x27,0x01};                       //写AFI数据01
unsigned char Write_DSFID[8]={0x0B,0x00,0x03,0x04,0x18,0x40,0x29,0xEE};                     //写DSFID数据EE
unsigned char Get_info[7]={0x0A,0x00,0x03,0x04,0x18,0x00,0x2B};                             //获取卡片系统信息
unsigned char Get_sec[9]={0x0C,0x00,0x03,0x04,0x18,0x00,0x2C,0x01,0x02};                    //获取块安全状态地址0x01,块数量02（实际为3个块）


#if RFID_TEST
/******************************************************************************************************************
* 函数名称：RFID_Test()
* 功    能：RFID卡片脱机测试函数。
* 入口参数：无
* 出口参数：无
* 说    明：该函数能对卡片进行读写操作、及写AFI、写DSFID、和获取卡片信息信息等操作    
*******************************************************************************************************************/
void RFID_test(void)
{
    unsigned char i, count;
    unsigned char j, temp2;
    
    if(Found_tag)
    {
        for(i = 0; i < 9; i++)                      //设置TRF7961通信参数
        {
            buf[i]=Set_pro[i];
        }
       
        count = buf[0] - 8;
        WriteSingle(&buf[5], count);
    
        for(test_no=0;test_no<6;test_no++)
        {   
            delay_ms(800);
            Display_Clear();                        //清屏
            Display_Rssi(rssival);                  //显示RSSI接收强度图案
        
            switch(test_no)
            {
                case 0:
                for(i = 0; i < 12; i++)             //写一个块地址为0x01 12345678
                {
                    buf[ i ] = Write_Sig[ i ];
                }
                Display_Chinese(0, 0, (unsigned char *)xie);                    //显示“写一个块”汉字
                Display_Chinese(0, 1, (unsigned char *)yi);
                Display_Chinese(0, 2, (unsigned char *)ge);
                Display_Chinese(0, 3, (unsigned char *)kuai);
                Display_Char(2, 0, (unsigned char *)Num+(buf[7]  >> 4)*0x10);   //显示地址
                Display_Char(2, 1, (unsigned char *)Num+(buf[7]  & 0x0f)*0x10);

                for(j = 8, temp2 = 15; j < 12; j++,temp2 -= 2)                  //显示数据
                {
                    Display_Char(2, temp2 - 1, (unsigned char *)(Num + (buf[j] >> 4) * 0x10));
                    Display_Char(2, temp2,(unsigned char *)(Num+(buf[j] & 0x0F) * 0x10));
                }
                break;
                case 1:
                for(i = 0; i < 9; i++)              //读一个块地址为0x01
                {
                    buf[ i ] = Read_Sig[ i ];
                }
                Display_Chinese(0, 0, (unsigned char *)du);                     //显示“读一个块”
                Display_Chinese(0, 1, (unsigned char *)yi);
                Display_Chinese(0, 2, (unsigned char *)ge);
                Display_Chinese(0, 3, (unsigned char *)kuai);
                Display_Char(2, 0, (unsigned char *)Num+(buf[7]  >> 4)*0x10);   //显示地址
                Display_Char(2, 1, (unsigned char *)Num+(buf[7]  & 0x0f)*0x10);
                break;
                case 2:
                for(i = 0; i < 8; i++)              //写AFI应用族
                {
                    buf[ i ] = Write_AFI[ i ];
                }
                Display_Chinese(0, 0, (unsigned char *)xie);                    //显示“写AFI”字符
                Display_Char(0, 2, (unsigned char *)Num+0x0A*0x10);
                Display_Char(0, 3, (unsigned char *)Num+0x0F*0x10);
                Display_Char(0, 4, (unsigned char *)CI);
                Display_Char(2, 13, (unsigned char *)Num+(buf[7]  >> 4)*0x10);  //显示写入的数据
                Display_Char(2, 14, (unsigned char *)Num+(buf[7]  & 0x0f)*0x10);
                break;
                case 3:
                for(i = 0; i < 8; i++)              //写DSFID数据存储格式
                {
                    buf[i]=Write_DSFID[i];
                }
                Display_Chinese(0, 0, (unsigned char *)xie);                    //显示“写DSFID”字符
                Display_Char(0, 2, (unsigned char *)Num+0x0D*0x10);
                Display_Char(0, 3, (unsigned char *)CS);
                Display_Char(0, 4, (unsigned char *)Num+0x0F*0x10);
                Display_Char(0, 5, (unsigned char *)CI);
                Display_Char(0, 6, (unsigned char *)Num+0x0D*0x10);
                Display_Char(2, 13, (unsigned char *)Num+(buf[7]  >> 4)*0x10);  //显示数据
                Display_Char(2, 14, (unsigned char *)Num+(buf[7]  & 0x0f)*0x10);
                break;
                case 4:
                for(i = 0; i < 7; i++)              //获取卡片信息信息
                {
                    buf[ i ] = Get_info[ i ];
                }
                Display_Chinese(0, 0, (unsigned char *)xi);                     //显示“系统信息”
                Display_Chinese(0, 1, (unsigned char *)tong);
                Display_Chinese(0, 2, (unsigned char *)xin);
                Display_Chinese(0, 3, (unsigned char *)xi2);
                break;
                case 5:
                for(i = 0; i < 9; i++)              //获取块安全状态   
                {
                    buf[ i ] = Get_sec[ i ];
                }
                Display_Chinese(0, 0, (unsigned char *)an);                     //显示“安全状态”
                Display_Chinese(0, 1, (unsigned char *)quan);
                Display_Chinese(0, 2, (unsigned char *)zhuang);
                Display_Chinese(0, 3, (unsigned char *)tai);
                Display_Char(2, 0, (unsigned char *)CS);                        //显示“S”表示起始块地址
                Display_Char(2, 1, (unsigned char *)Num+(buf[7]  >> 4)*0x10);   //起始地址
                Display_Char(2, 2, (unsigned char *)Num+(buf[7]  & 0x0f)*0x10);
                Display_Char(2, 4, (unsigned char *)CN);                        //显示“N”表示块数量
                Display_Char(2, 5, (unsigned char *)Num+(buf[8]  >> 4)*0x10);   //块数量
                Display_Char(2, 6, (unsigned char *)Num+(buf[8]  & 0x0f)*0x10);
                break;
                default:break;
            }
            Display_pro();                          //显示进度条...
            
            count = buf[0] - 8;
            RequestCommand(&buf[0], count, 0x00, 0);//发送请求命令  
        }
        delay_ms(800);
    }
}
#endif


/******************************************************************************************************************
* 函数名称：FindTags()
* 功    能：根据指定卷标协议类型，设置TRF7960配置各相关寄存器后，进行寻卡操作。
* 入口参数：protocol       指定协议类型
* 出口参数：无
* 说    明：该函数是一个死循环函数，所有的脱机演示执行过程均在此完成。
*******************************************************************************************************************/
void FindTags(unsigned char protocol)
{
    unsigned char command[10];                      //定义命令数据暂存缓冲器数组

    while(1)
    {
        if((protocol == 0x00) || (protocol == 0x01))//ISO15693协议标准
        {
            command[0] = ChipStateControl;          // 开启RF使能，选择5V操作模式
            command[1] = 0x21;
            command[2] = ISOControl;                // 设置选择ISO15693操作模式为:高比特率26.48kbps 单幅载波 1/4(默认模式)
            command[3] = 0x02;
            WriteSingle(command, 4);                // 写4个字节命令到TRF7960寄存器中

            delay_ms(5);
            flags = 0x06;                           // 16(slot)槽模式
            //flags = 0x26;                         // 1(slot)槽模式

            command[0] = 0x00;
            InventoryRequest(command, 0);           // 发送总量请求命令(即寻卡命令)

            command[0] = ChipStateControl;          // 关闭RF部分电路
            command[1] = 0x01;
            WriteSingle(command, 2);
            delay_ms(1);

            command[0] = IRQStatus;                 // 给寄存器赋值
            command[1] = IRQMask;               

            if(SPIMODE)                             //读取IRQ中断状态寄存器及中断标志
                ReadCont(command, 2);
            else
                ReadSingle(command, 1); 
#if RFID_TEST
    if(Found_tag == 1)
        RFID_test();
        //return;
#endif
        }

        if((protocol == 0x00) || (protocol == 0x02))//ISO14443A协议标准
        {
            command[0] = ChipStateControl;          // 开启RF使能，选择5V操作模式
            command[1] = 0x21;
            command[2] = ISOControl;                // 设置选择ISO14443A操作模式为:比特率106kbps
            command[3] = 0x08;
            WriteSingle(command, 4);
            delay_ms(5);

            AnticollisionSequenceA(0x00);           //执行ISO14443A完整仿冲撞序列

            command[0] = ChipStateControl;          // 给寄存器赋值
            command[1] = 0x01;
            WriteSingle(command, 2);                // 关闭RF部分电路 
            delay_ms(1);

            command[0] = IRQStatus;                 // 给寄存器赋值 
            command[1] = IRQMask;   
        
            if(SPIMODE)
                ReadCont(command, 2);               //读取IRQ中断状态寄存器及中断标志
            else
                ReadSingle(command, 1); 
        }

        if((protocol == 0x00) || (protocol == 0x03))//ISO14443B协议标准
        {
            command[0] = ChipStateControl;
            command[1] = 0x21;                      // 开启RF使能，选择5V操作模式
            WriteSingle(command, 2);
            command[0] = ISOControl;                // 设置选择ISO14443B操作模式为:比特率106kbps
            command[1] = 0x0C;
            WriteSingle(command, 2);

            delay_ms(5);
            AnticollisionSequenceB(0xB0, 0x04);     //执行ISO14443A完整仿冲撞序列(0x04表示16槽slots)
            //AnticollisionSequenceB(0xB0, 0x00);   //0x00 表示单个槽slot

            command[0] = ChipStateControl;  
            command[1] = 0x01;
            WriteSingle(command, 2);                // 关闭RF部分电路 
            delay_ms(1);

            command[0] = IRQStatus;
            command[1] = IRQMask;   
        
            if(SPIMODE)                             //读取IRQ中断状态寄存器及中断标志
                ReadCont(command, 2);
            else
                ReadSingle(command, 1); 
        }

        if((protocol == 0x00) || (protocol == 0x04))//Tag-it协议标准
        {
            command[0] = ChipStateControl;          // 开启RF使能，选择5V操作模式
            command[1] = 0x21;
            command[2] = ISOControl;                // 设置选择Tag-it操作模式
            command[3] = 0x13;
            WriteSingle(command, 4);
            delay_ms(5);
            flags = 0x00;
            command[0] = 0x00;
            TIInventoryRequest(command, 0);         //发送寻卡命令

            command[0] = ChipStateControl;          // 关闭RF部分电路 
            command[1] = 0x01;
            WriteSingle(command, 2);
            delay_ms(1);

            command[0] = IRQStatus;                 // 给寄存器赋值 
            command[1] = IRQMask;
        
            if(SPIMODE)
                ReadCont(command, 2);               //读取IRQ中断状态寄存器及中断标志
            else
                ReadSingle(command, 1); 
        }
        delay_ms(800);
    }   /* while */
}   /* FindTags */




