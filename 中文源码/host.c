/**********************************************************************************************************************************************************
* 文 件 名：HOST.C
* 功    能：SPI相关初始化函数USART和UART串口相关命令。
*
* 作    者：EMDOOR
* 日    期：2011年04月13号
**********************************************************************************************************************************************************/
#include "hardware.h"
#include "communicate.h"
#include "anticollision.h"
#include "globals.h"
#include "tiris.h"
#include "ISO14443.h"
#include "host.h"
#include "lcd.h"


unsigned char   RXdone;                             //接收完整数据标志位，若接收完成，置该标志位1
unsigned char   ENABLE;                             //TRF7960使能与禁止切换标志位，1使能；0禁止状态
unsigned char   Firstdata = 1;                      //设置串口同步标志位
unsigned char   LCDcon = 0;                         //LCD连机标志位

/******************************************************************************************************************
* 函数名称：UARTset()
* 功    能：串口初始化设置函数
* 入口参数：无
* 出口参数：无
* 说    明：该串口初始化函数使用USCI_A0。    
*******************************************************************************************************************/
void UARTset(void)
{
    P3SEL |= BIT4 + BIT5;                           //P3.4和P3.5管脚设置成UART模式
    P3DIR |= BIT4;                                  //设置P3.4为输出方向

    UCA0CTL1 |= UCSWRST;                            //禁止UART
    UCA0CTL0 = 0x00;

    UCA0CTL1 |= UCSSEL_2;                           //选择频率SMCLK
    UCA0BR0 = BAUD0;                                //选择115200
    UCA0BR1 = BAUD1;

    UCA0MCTL = 0;                                   //调制参数UCBRSx = 0
  
    UCA0CTL1 &= ~UCSWRST;                           //初始化USCI_A0状态，使能UART
    IE2 |= UCA0RXIE;                                //使能USCI_A0接收中断
}

/******************************************************************************************************************
* 函数名称：USARTset()
* 功    能：SPI初始化设置函数
* 入口参数：无
* 出口参数：无
* 说    明：该函数使用内部晶体振荡器配置 USCI_B0。    
*******************************************************************************************************************/
void USARTset(void)
{
    UCB0CTL1 |= UCSWRST;                             //首先禁止USCI
    UCB0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC;     //设置3管脚，8位SPI主设备
    UCB0CTL1 |= UCSSEL_2;                            //选择SMCLK

    UCB0BR0 = 0x00;
    UCB0BR1 = 0x00;
    P3SEL |= BIT1 + BIT2 + BIT3;                     //选择P3.1为UCB0SIMO,P3.2为UCB0SOMI,P3.3为UCBOCLK

    SlaveSelectPortSet();                            //P3.0 为 Slave Select从设备选择脚
    H_SlaveSelect();                                 //从设备使能Slave Select被禁止(高电平)

    UCB0CTL1 &= ~UCSWRST;                            //初始化USCI状态寄存器
}

/******************************************************************************************************************
* 函数名称：USARTEXTCLKset()
* 功    能：SPI初始化设置函数
* 入口参数：无
* 出口参数：无
* 说    明：该函数使用外部晶体振荡器配置 USCI_B0。    
*******************************************************************************************************************/
void USARTEXTCLKset(void)
{
    UCB0CTL1 |= UCSWRST;                            //首先禁止USCI
    UCB0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC;    //设置3管脚，8位SPI主设备
    UCB0CTL1 |= UCSSEL_2;                           //选择SMCLK

    UCB0BR0 = 0x00;
    UCB0BR1 = 0x00;
    P3SEL |= BIT1 + BIT2 + BIT3;                    //选择P3.1为UCB0SIMO,P3.2为UCB0SOMI,P3.3为UCBOCLK

    SlaveSelectPortSet();                           //P3.0 为 Slave Select从设备选择脚
    H_SlaveSelect();                                //从设备使能Slave Select被禁止(高电平)

    UCB0CTL1 &= ~UCSWRST;                           //初始化USCI状态寄存器
}

/******************************************************************************************************************
* 函数名称：BaudSet()
* 功    能：串口通信设置函数
* 入口参数：mode    模式设置
* 出口参数：无
* 说    明：程序默认外部晶体频率为6.78MHz;内部DCO模式频率为8MHz    
*******************************************************************************************************************/
void BaudSet(unsigned char mode)
{
    if(mode == 0x00)                                //若为0x00 则为外部高速晶体模式
    {
        UCA0BR0 = BAUD0;                            //波特率设置高低位
        UCA0BR1 = BAUD1;
    }
    else                                            //内部DCO振荡模式
    {
        UCA0BR0 = BAUD0EN;                          //波特率设置高低位
        UCA0BR1 = BAUD1EN;
    }
}

/******************************************************************************************************************
* 函数名称：kputchar()
* 功    能：向计算器上位机发送一个字符函数
* 入口参数：TXchar    将要被发送的字符
* 出口参数：无
* 说    明：无   
*******************************************************************************************************************/
void kputchar(char TXchar)
{
    while(!(IFG2 & UCA0TXIFG));                     //等待 USCI_B0 TX 缓冲区准备好（清空）

    UCA0TXBUF = TXchar;                             //发送字符
}

/******************************************************************************************************************
* 函数名称：put_bksp()
* 功    能：向计算器上位机发送一个退格Backspace符号
* 入口参数：无
* 出口参数：无
* 说    明：无   
*******************************************************************************************************************/
void put_bksp(void)
{
    kputchar('\b');                                 //发送字符\b
    kputchar(' ');                                  //发送空格
    kputchar('\b');                                 //发送字符\b
}

/******************************************************************************************************************
* 函数名称：put_space()
* 功    能：向计算器上位机发送一个空格符号
* 入口参数：无
* 出口参数：无
* 说    明：无   
*******************************************************************************************************************/
void put_space(void)
{
    kputchar(' ');                                  //发送空格
}

/******************************************************************************************************************
* 函数名称：put_crlf()
* 功    能：向计算器上位机发送一个回车+换行符号
* 入口参数：无
* 出口参数：无
* 说    明：无   
*******************************************************************************************************************/
void put_crlf(void)
{
    kputchar('\r');                                 //发送回车符号
    kputchar('\n');                                 //发送换行符号
}

/******************************************************************************************************************
* 函数名称：send_cstring()
* 功    能：向计算器上位机发送一字符串
* 入口参数：*pstr       将要被发送的字符串
* 出口参数：无
* 说    明：无   
*******************************************************************************************************************/
void send_cstring(char *pstr)
{
    while(*pstr != '\0')                            //查询是否到达字符串尾
    {
        kputchar(*pstr++);                          //发送字符
    }
}

/******************************************************************************************************************
* 函数名称：Nibble2Ascii()
* 功    能：半位字节转化成ASCII十六进制码
* 入口参数：anibble         将要被转换字节
* 出口参数：AsciiOut        转换后的ASCII十六进制码值
* 说    明：无   
*******************************************************************************************************************/
unsigned char Nibble2Ascii(unsigned char anibble)
{
    unsigned char AsciiOut = anibble;               //定义转换后的变量AsciiOut，并赋值

    if(anibble > 9)                                 //如果被转换的半位字节为A-F，则需要加0x07
        AsciiOut = AsciiOut + 0x07;

    AsciiOut = AsciiOut + 0x30;                     //其他情况则在转换结果基础上增加偏移量0x30

    return(AsciiOut);                               //返回转换后的值
}

/******************************************************************************************************************
* 函数名称：Put_byte()
* 功    能：发送字节函数半位字节转化成ASCII十六进制码
* 入口参数：abyte         将要被发送字节
* 出口参数：无      
* 说    明：该函数调用两次Nibble2Ascii，将一个字节拆分成高低四位，先转换再次序发送。
*******************************************************************************************************************/
void Put_byte(unsigned char abyte)
{
    unsigned char temp1, temp2;

    temp1 = (abyte >> 4) & 0x0F;                    //获取高四位字节
    temp2 = Nibble2Ascii(temp1);                    //转换成ASCII码
    kputchar(temp2);                                //发送之

    temp1 = abyte & 0x0F;                           //获取低四位字节
    temp2 = Nibble2Ascii(temp1);                    //转换成ASCII码
    kputchar(temp2);                                //发送之
}

/******************************************************************************************************************
* 函数名称：Get_nibble()
* 功    能：在发送字节函数Put_byte结束后，获取一个十六进制字节
* 入口参数：无
* 出口参数：rxdata          返回接收到的字节
* 说    明：该函数调用两次Nibble2Ascii，将一个字节拆分成高低四位，先转换再次序发送。
*******************************************************************************************************************/
unsigned char Get_nibble(void)
{
    unsigned char reading;                          //读标志位
    //unsigned char rxdata;

    reading = 1;                                    //标志位置1 表示读取尚未完成
    while(reading)                                  //循环读取字符
    {                   		
        LPM0;                                       //进入低功耗模式，等待唤醒
        if(rxdata >= 'a')                           //转换成大写字母
        {
            rxdata -= 32;
        }

        /* 如果为十六进制，则回显之 */
        /*====================================================================================================*/
        if(((rxdata >= '0') && (rxdata <= '9')) || ((rxdata >= 'A') && (rxdata <= 'F')))
        {
            reading = 0;
            kputchar(rxdata);                       //发送回显字符
        
            if(rxdata > '9')                        //如果ASCII码值范围是A-F，则加9
            {       
                rxdata = (rxdata & 0x0F) + 9;
            }
        }
        /*====================================================================================================*/
    }
    
    return(rxdata);                                 //返回接收字符                             
}

/******************************************************************************************************************
* 函数名称：Get_line()
* 功    能：在获取回显字节函数Get_nibble结束后，获取一字符串字符
* 入口参数：*pline          输入字符串
* 出口参数：err_flg         返回标志
* 说    明：该函数仅允许数字0到9,字母A到F,或者小写字母a到f,及回车<CR>,换行<LF>,和退格<backspace>符号
*           其他符号均为非法字符，将被忽略。
*           正常操作返回0，若超过范围返回1。
*******************************************************************************************************************/
unsigned char Get_line(unsigned char *pline)
{
    unsigned char reading, err_flg;                 //读标志位，错误标志位
    unsigned char pos;                              //接收到的半字节数量
    unsigned char Length_Byte;                      //最大允许值为256字节（512个半字节）
    
    err_flg = 0;                                    //设置无错误标志
    Length_Byte = 0xff;                             //设置最大字节数

    if(!Firstdata)                                  //等待SOF开始标志位；0后面为1
    {
        LPM0;                                       //进入低功耗模式，等待被中断唤醒
        while(rxdata != '0')                        //判断接收到的字符是否为0，再否一直等待
        {
        }
    }
    else
    {
        Firstdata = 0;
    }

    kputchar('0');                                  //回显字符0
    LPM0;                                           //进入低功耗模式，等待被中断唤醒
    while(rxdata != '1')                            //判断接收到的字符是否为1，再否一直等待
    {
    }

    kputchar('1');                                  //回显字符1
    RXdone = 0;                                     //SOF开始标志位清零

    pos = 0;                                        //字符位置计数器置1
    reading = 1;                                    //标志位置1 表示读取尚未完成
    while(reading)                                  //循环读取字符
    {               		
        while(RXdone == 0);                         //等待接收完整

        RXdone = 0;                                 //清除标志位
        switch(rxdata)                              //根据得到的数据分别选择以下操作
        {
            case '\r':                              //如果是回车符号，忽略之
                break;                  
            case '\n':
                reading = 0;                        //如果是换行符号。则退出读循环
                break;
            case '\b':                              //如果是退格backspace符号
                if(pos > 0)
                {                                   //如果需要清除一个字符?
                    pos--;                          //从缓冲区中移除
                    put_bksp();                     //退格并删除
                    if(pos & 0x01 > 0)
                    {                               //偶数位字节（高）
                        *pline--;
                        *pline &= 0xF0;             //清除半位字节
                    }
                }
                break;
            default:                                //其他字符
            if(rxdata >= 'a')                       //转换成大写字母
            {
                rxdata -= 32;
            }

            /* 如果不是十六进制数据，则跳出循环 */
            /*-----------------------------------------------------------------------------*/
            if((rxdata < '0') || ((rxdata > '9') && (rxdata < 'A')) || (rxdata > 'F'))
            {
                break;
            }
            /*-----------------------------------------------------------------------------*/
                
            /* 缓冲区若有空间则存储之 */
            /*-----------------------------------------------------------------------------*/
            if(pos++ < 2 * BUF_LENGTH)
            {
                kputchar(rxdata);                           //回显
                if(rxdata > '9')                            //如果ASCII码值范围是A-F，则加9
                {       	
                    rxdata = (rxdata & 0x0F) + 9;
                }

                if((pos & 0x01) == 0)                       //低四位半字节奇数位为0(低)
                {           		
                    *pline += (rxdata & 0x0F);              //存储之
                    if(pos == 2)                            //如果接收到2个半位字节，则改变长度字节
                        Length_Byte = *pline;

                    pline++;
                    if(((Length_Byte - 1) * 2) == pos)      //如果完成，则标记退出循环标志
                        reading = 0;
                }
                else                                        //偶数位字节（高）
                {   
                    rxdata = rxdata << 4;                   //移动到高四位半字节
                    *pline = rxdata;                        //存储之
                }
                /*-----------------------------------------------------------------------------*/
            }
            else                                            //超过存储空间，则返回错误
                err_flg = 1;
        }   /* switch */
    }   /* while(1) */

    return(err_flg);                                        //返回退出
}

/******************************************************************************************************************
* 函数名称：HostCommands()
* 功    能：接收上位机的主机命令，并做出相应操作
* 入口参数：无
* 出口参数：无
* 说    明：该函数为死循环函数
*******************************************************************************************************************/
void HostCommands(void)
{
    char *phello;
    unsigned char *pbuf, count;
    
    POLLING = 0;                                    //设置POLLING为0，进入连机USB模式
    
    while(1)
    {
        pbuf = &buf[0];                             //指针定位
        Get_line(pbuf);                             //调用Get_line函数获取一命令行
        put_crlf();                                 //回车+换行结束

        pbuf = &buf[4];                             //指象数据包第5个包字节
        RXErrorFlag = 0;                            //RX接受错误标志位清0

        if(*pbuf == 0xFF)                           //检查串口端口号 0xFF表示：TRF7960 EVM \r\n
        {
            phello = "**********************************************************\r\n*                                                        *\r\n* Welcome to use Emdoor RFID(13.56MHz) learning platform *\r\n*                                                        *\r\n**********************************************************\r\n";
            send_cstring(phello);
        }
        else if(*pbuf == 0x10)                      //0x10寄存器写(地址+数据,地址+数据...)
        {           
            send_cstring("Register write request.\r\n");
            count = buf[0] - 8;
            WriteSingle(&buf[5], count);
        }
        else if(*pbuf == 0x11)                      //0x11连续写(地址+数据+数据...)
        {          
            phello = "Continous write request.\r\n";
            send_cstring(phello);
            count = buf[0] - 8;
            WriteCont(&buf[5], count);
        }
        else if(*pbuf == 0x12)                      //0x12寄存器读(地址+数据,地址+数据...)
        {           
            phello = "Register read request.\r\n";
            send_cstring(phello);
            count = buf[0] - 8;
            ReadSingle(&buf[5], count);
            Response(&buf[5], count);
        }
        else if(*pbuf == 0x13)                      //0x13连续读(地址+数据+数据...)
        {        
            send_cstring("Continous read request\r\n");
            pbuf++;
            count = *pbuf;                          //将要被读取寄存器的数量

            pbuf++;
            buf[5] = *pbuf;                         //开始寄存器地址

            ReadCont(&buf[5], count);               //连续读
            Response(&buf[5], count);
        }
        else if(*pbuf == 0x14)                      //0x14总量请求命令
        {                           
            phello = "ISO 15693 Inventory request.\r\n";
            send_cstring(phello);
            flags = buf[5];
            for(count = 0; count < 8; count++) buf[count + 20] = 0x00;
                InventoryRequest(&buf[20], 0x00);
        }
        else if(*pbuf == 0x15)                      //0x15直接命令
        {
            phello = "Direct command.\r\n";
            send_cstring(phello);
            DirectCommand(&buf[5]);
        }
        else if(*pbuf == 0x16)                      //0x16直接写命令
        {
            phello = "RAW mode.\r\n";
            send_cstring(phello);
            count = buf[0] - 8;
            RAWwrite(&buf[5], count);
        }
        else if(*pbuf == 0x17)                      //0x17预留
        {
        }
        else if(*pbuf == 0x18)                      //0x18请求命令
        {                
            phello = "Request mode.\r\n";
            send_cstring(phello);
            count = buf[0] - 8;
            RequestCommand(&buf[0], count, 0x00, 0);
        }
        else if(*pbuf == 0x19)                      //0x19 ISO14443A 测试命令：发送和接收
        {
            /* 在不同的比特率模式下，在TX发送后改变ISO模式寄存器 */
            phello = "14443A Request - change bit rate.\r\n";
            send_cstring(phello);
            count = buf[0] - 9;
            Request14443A(&buf[1], count, buf[5]);
        }
        else if(*pbuf == 0x34)                      //0x34 SID Tag-it协议请求命令
        {             
            phello = "Ti SID Poll.\r\n";
            send_cstring(phello);
            flags = buf[5];
            for(count = 0; count < 4; count++) buf[count + 20] = 0x00;
                TIInventoryRequest(&buf[20], 0x00);
        }
        else if(*pbuf == 0x0F)                      //0x0F直接模式
        {
            phello = "Direct mode.\r\n";
            send_cstring(phello);
            DirectMode();
        }
        else if((*pbuf == 0xB0) || (*pbuf == 0xB1)) //ISO14443B协议请求命令和唤醒命令
        {      
            /* 0xB0 - REQB 0xB1 - WUPB */
            phello = "14443B REQB.\r\n";   
            send_cstring(phello);
            AnticollisionSequenceB(*pbuf, *(pbuf + 1));
            //AnticollisionSequenceB(0xB0, 0x00);   //单个槽slot模式
        }
        else if((*pbuf == 0xA0) || (*pbuf == 0xA1)) //ISO14443A协议请求命令
        {                   
            phello = "14443A REQA.\r\n";
            send_cstring(phello);
            AnticollisionSequenceA(*(pbuf + 1));
        }
        else if(*pbuf == 0xA2)
        {       
            phello = "14443A Select.\r\n";
            send_cstring(phello);
            switch(buf[0])
            {
                case 0x0D:
                    for(count = 1; count < 6; count++)
                        buf[99 + count] = *(pbuf + count);
                    break;
                case 0x11:
                    for(count = 1; count < 11; count++)
                        buf[100 + count] = *(pbuf + count);
                    buf[100] = 0x88;
                    break;
                case 0x15:
                    for(count = 1; count < 5; count++)
                        buf[100 + count] = *(pbuf + count);
                    buf[100] = 0x88;
                    buf[105] = 0x88;
                    for(count = 1; count < 10; count++)
                        buf[105 + count] = *(pbuf + count + 4);
            }   /* switch */

            buf[0] = ISOControl;
            buf[1] = 0x88;                          //接收不带CRC校验
            WriteSingle(buf, 2);

            buf[5] = 0x26;                          //发送REQA请求命令

            if(RequestCommand(&buf[0], 0x00, 0x0f, 1) == 0)
            {
                if(SelectCommand(0x93, &buf[100]))
                {
                    if(SelectCommand(0x95, &buf[105])) SelectCommand(0x97, &buf[110]);
                }
            }
        }
        else if(*pbuf == 0x03)                      //使能或者禁止ITRF7960阅读器芯片
        {                  
            if(*(pbuf + 1) == 0xAA)                 //0x03AA使能TRF7960
            {
                TRFEnable();
                BCSCTL2 |= SELM1 + SELM0 + SELS;
                InitialSettings();
                BaudSet(*(pbuf + 1));
                OSCsel(*(pbuf + 1));
                send_cstring("Reader enabled.");
                BeepON();
                delay_ms(50);
                BeepOFF();
                ENABLE = 1;
            }
            else if(*(pbuf + 1) == 0xFF)            //0x03FF禁止TRF7960
            {
                BaudSet(*(pbuf + 1));
                OSCsel(*(pbuf + 1));
                TRFDisable();
                BeepON();
                delay_ms(10);
                BeepOFF();
                send_cstring("Reader disabled.");
                ENABLE = 0;
            }
            else if(*(pbuf + 1) == 0x0A)            //0x030A使用外部晶体时钟
            {
                BaudSet(0x00);
                OSCsel(0x00);
                send_cstring("External clock.");
            }
            else if(*(pbuf + 1) == 0x0B)            //0x030B使用内部晶体时钟
            {
                BaudSet(0x01);
                OSCsel(0x01);
                send_cstring("Internal clock.");
            }
            else                                    //其他预留
                ;
        }
        else if(*pbuf == 0xF0)                      //0xF0 AGC设置
        {     
            buf[0] = ChipStateControl;
            buf[1] = ChipStateControl;
            ReadSingle(&buf[1], 1);
            if(*(pbuf + 1) == 0xFF)                 //AGC开启
                buf[1] |= BIT2;
            else                                    //AGC关闭
                buf[1] &= ~BIT2;
            WriteSingle(buf, 2);
        }
        else if(*pbuf == 0xF1)                      //0xF1 AM PM相关设置
        {            
            buf[0] = ChipStateControl;
            buf[1] = ChipStateControl;
            ReadSingle(&buf[1], 1);
            if(*(pbuf + 1) == 0xFF)                 //选择AM
                buf[1] &= ~BIT3;
            else                                    //选择PM
                buf[1] |= BIT3;
            WriteSingle(buf, 2);
        }
        else if(*pbuf == 0xF2)                      //0xF2 功耗设置
        {                  
            buf[0] = ChipStateControl;
            buf[1] = ChipStateControl;
            ReadSingle(&buf[1], 1);
            if(*(pbuf + 1) == 0xFF)
                buf[1] &= ~BIT4;
            else
                buf[1] |= BIT4;
            WriteSingle(buf, 2);
        }
        else if(*pbuf == 0xFE)                      //固件版本号
        {                
            phello = "Firmware V1.0.0 \r\n";
            send_cstring(phello);
        }
        else                                        //其他为非法命令
        {
            phello = "Unknown command.\r\n";
            send_cstring(phello);
        }   /* if */

        while(!(IFG2 & UCA0TXIFG));                 //等待TX缓冲区准备好（清空）
        
        /* 若与上位机成功通信，则在LCD上显示USB连机图标 */
        /*-----------------------------------------------------------------------------*/
        if(LCDcon)
        {
            LCDcon = 0;
        
            Display_Clear();
            Display_Connect();
            //Display_Picture((unsigned char *)usb); //显示USB连接图标
        }
        /*-----------------------------------------------------------------------------*/
    }   /* while(1) */
}   /* HostCommands */

/******************************************************************************************************************
* 函数名称：RXhandler()
* 功    能：串口接收中断服务程序
* 入口参数：无
* 出口参数：无
* 说    明：串口USCI_A0接收向量中断
*******************************************************************************************************************/
#pragma vector = USCIAB0RX_VECTOR
__interrupt void RXhandler (void)
{
    if(IFG2 & UCA0RXIFG)                            //如果串口接收到数据
    {   
        rxdata = UCA0RXBUF;                         //将接收缓冲区UCA0RXBUF数据赋值给rxdata
        RXdone = 1;                                 //置起接收标志位
        if(ENABLE == 0)                             //TRF7960在禁止状态下
        {
            TRFEnable();                            //使能TRF790
            BaudSet(0x00);                          //设置波特率
            OSCsel(0x00);                           //设置外部晶体振荡器 
               
            InitialSettings();                      //初始化TRF7960
            send_cstring("Reader enabled.");        //向上位机发送信息
            ENABLE = 1;                             //设置TRF7960标志位
        }
        __low_power_mode_off_on_exit();

        if(Firstdata)                               //如果是第1次接收到数据
        {
            LCDcon = 1;
            
            IRQOFF();                               //关闭IRQ中断
            StopCounter();                          //停止计数器
                  
            /* 利用SP操作，在中断返回后，可调用HostCommands函数 */
            /*-----------------------------------------------------------------------------*/
            asm("mov.w #HostCommands,10(SP)");      //调用HostCommands函数
            /*-----------------------------------------------------------------------------*/
        }
    }
}




