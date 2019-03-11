/**********************************************************************************************************************************************************
* 文 件 名：COMMUNICATE.C
* 功    能：RFID阅读器TRF7960与MSP430F2370微控制器之间通信方式。
* 硬件连接：MSP430F2370与TRF7960之间通信硬件连接关系如下所示：
*               MSP430F2370                 TRF7960
*********************    PARALLEL INTERFACE    ******************************************
*               P4.0(Pin26)                 DATA0(Pin17)
*               P4.1(Pin27)                 DATA1(Pin18)
*               P4.2(Pin28)                 DATA2(Pin19)
*               P4.3(Pin29)                 DATA3(Pin20)
*               P4.4(Pin30)                 DATA4(Pin21)
*               P4.5(Pin31)                 DATA5(Pin22)
*               P4.6(Pin32)                 DATA6(Pin23)
*               P4.7(Pin33)                 DATA7(Pin24)
*********************    SERIAL(SPI) INTERFACE    ***************************************           
*               P3.0(Pin18)                 Slave_select(Pin21)
*               P3.1(Pin19)                 SIMO(Pin24)
*               P3.2(Pin20)                 SOMI(Pin23)
*               P3.3(Pin21)                 DATA_CLK(Pin26)
*
* 作    者：EMDOOR
* 日    期：2011年04月13号
**********************************************************************************************************************************************************/
#include "communicate.h"
#include "globals.h"

#define DBG 1

unsigned char temp;
unsigned int DUMMYREAD = 0;
unsigned int mask = 0x80;

/******************************************************************************************************************
* 函数名称：PARset()
* 功    能：并行(PAR)模式配置函数
* 入口参数：无
* 出口参数：无
* 说    明：MSP430微控制器与TRF7960使用并行模式通信，并口模式初始化配置
******************************************************************************************************************/
void PARset(void)
{
    EnableSet();                                    //使能TRF7960阅读器

    TRFDirOUT();                                    //MSP430F2370 端口4输出
    TRFFunc();                                      //设置MSP430F2370 端口4为普通GPIO口
    TRFWrite(0x00);                                 //向端口4 发送数据0x00

    CLKOFF();                                       //CLK时钟关闭
    CLKPOUTset();                                   //设置CLK时钟管脚P3.3方向为输出

    IRQPinset();                                    //设置IRQ中断管脚方向为输入
    IRQEDGEset();                                   //设置上升沿中断

    Port1Set();                                     //端口1设置为输出
    LEDallOFF();                                    //所有的LED指示灯全部关闭
    BeepOFF();                                      //蜂鸣器关
}

/******************************************************************************************************************
* 函数名称：PARStopCondition()
* 功    能：并行(PAR)模式停止通信条件
* 入口参数：无
* 出口参数：无
* 说    明：MSP430微控制器与TRF7960使用并行模式通信，停止命令
******************************************************************************************************************/
void PARStopCondition(void)
{
    TRFWriteBit(0x80);                              //停止条件：端口B7位设置为高电平
    CLKON();                                        //CLK时钟开
    TRFWrite(0x00);                                 //端口写0x00
    CLKOFF();                                       //CLK时钟关
}

/******************************************************************************************************************
* 函数名称：PARStopCont()
* 功    能：并行(PAR)连续模式停止通信条件
* 入口参数：无
* 出口参数：无
* 说    明：MSP430微控制器与TRF7960使用并行连续模式通信，停止命令
******************************************************************************************************************/
void PARStopCont(void)
{   
    TRFWrite(0x00);                                 //端口写0x00
    TRFDirOUT();                                    //端口配置为输出
    TRFWrite(0x80);                                 //端口写0x80
    __no_operation();                               //空操作nop                
    TRFWrite(0x00);                                 //端口写0x00
}

/******************************************************************************************************************
* 函数名称：PARStartCondition()
* 功    能：并行(PAR)模式开始通信条件
* 入口参数：无
* 出口参数：无
* 说    明：MSP430微控制器与TRF7960使用并行模式通信，开始启动命令
******************************************************************************************************************/
void PARStartCondition(void)
{
    TRFWrite(0x00);                                 //端口写0x00
    CLKON();                                        //CLK时钟开
    TRFWrite(0xff);                                 //端口写0xff
    CLKOFF();                                       //CLK时钟关
}

/******************************************************************************************************************
* 函数名称：SPIStartCondition()
* 功    能：串行(SPI)模式开始通信条件
* 入口参数：无
* 出口参数：无
* 说    明：MSP430微控制器与TRF7960使用串行(SPI)模式通信，开始启动命令
******************************************************************************************************************/
void SPIStartCondition(void)
{
    CLKGPIOset();                                   //设置端口P3.3 CLK为普通GPIO
    CLKPOUTset();                                   //设置端口P3.3 CLK方向为输出
    CLKON();                                        //CLK时钟开启（高）
}

/******************************************************************************************************************
* 函数名称：SPIStopCondition()
* 功    能：串行(SPI)模式停止通信条件
* 入口参数：无
* 出口参数：无
* 说    明：MSP430微控制器与TRF7960使用串行(SPI)模式通信，停止命令
******************************************************************************************************************/
void SPIStopCondition(void)
{
    CLKGPIOset();                                   //设置端口P3.3 CLK为普通GPIO
    CLKOFF();                                       //CLK时钟关闭（低）
    CLKPOUTset();                                   //设置端口P3.3 CLK方向为输出
    CLKOFF();                                       //CLK时钟关闭（低）
}

/******************************************************************************************************************
* 函数名称：WriteSingle()
* 功    能：写单个寄存器或者特殊地址的多个寄存器命令
* 入口参数：*pbuf            将要写入的数据           
*           lenght           写入数据的长度 
* 出口参数：无
* 说    明：写命令。
******************************************************************************************************************/
void WriteSingle(unsigned char *pbuf, unsigned char lenght)
{
    unsigned char i;
#ifdef SPI_BITBANG
    unsigned char j;
#endif
    
    /* 并行(PAR)模式通信 */
    /*====================================================================================================*/        
    if((SPIMODE) == 0) 
    {
        PARStartCondition();                        //并行模式开始
        while(lenght > 0)
        {
            *pbuf = (0x1f & *pbuf);                 //取低5位B0-B4 寄存器地址数据 格式为000XXXXX

            for(i = 0; i < 2; i++)                  //以单个寄存器，先发送地址，再发送数据或命令
            {
                TRFWrite(*pbuf);
                CLKON();
                CLKOFF();                           //CLK时钟关闭（低）
                pbuf++;
                lenght--;
            }
        }   /* while */

        PARStopCondition();                         //并行模式停止
    }   /* if((SPIMODE) == 0) */
    /*====================================================================================================*/ 

    /* 串行(SPI)模式通信 */
    /*====================================================================================================*/
    if (SPIMODE)
    {
#ifndef SPI_BITBANG 
        /* 硬件SPI模式 */
        /*-----------------------------------------------------------------------------*/
        L_SlaveSelect();                            //SS管脚输出低，SPI启动

        while(lenght > 0)
        {
            *pbuf = (0x1f & *pbuf);                 //取低5位B0-B4 寄存器地址数据 格式为000XXXXX
            for(i = 0; i < 2; i++)                  //以单个寄存器，先发送地址，再发送数据或命令
            {
                while (!(IFG2 & UCB0TXIFG));        //等待 USCI_B0 TX 缓冲区准备好
                UCB0TXBUF = *pbuf;                  //将数据赋值给TX缓冲区

                //while (!(IFG2 & UCB0RXIFG));
                temp=UCB0RXBUF;                     //读取 USCI_B0 RX 缓冲区，来清空该标志位

                pbuf++;
                lenght--;
            }
        }   /* while */
        H_SlaveSelect();                            //SS管脚输出高，SPI停止
        /*-----------------------------------------------------------------------------*/
#else
        /*  SPI 位模式 */
        /*-----------------------------------------------------------------------------*/
        L_SlaveSelect();                            //SS管脚输出低，SPI启动
        CLKOFF();                                   //CLK时钟关闭（低）
        while(lenght > 0)
        {
            *pbuf = (0x1f & *pbuf);                 //取低5位B0-B4 寄存器地址数据 格式为000XXXXX
            for(i = 0; i < 2; i++)                  //以单个寄存器，先发送地址，再发送数据或命令
            {
                for(j = 0; j < 8; j++)
                {
                    if (*pbuf & mask)               //设置数据位
                        SIMOON();
                    else
                        SIMOOFF();
                   
                    CLKON();                        //关联CLK时钟信息，发送数据
                    CLKOFF();
                    mask >>= 1;                     //标志位右移
                }   /* for */
                mask = 0x80;
                pbuf++;
                lenght--;
            }/*for*/
        } /*while*/

        H_SlaveSelect();                            //SS管脚输出高，SPI停止
        /*-----------------------------------------------------------------------------*/
#endif
   }    /* if (SPIMODE) */
}   /* WriteSingle */

/******************************************************************************************************************
* 函数名称：WriteCont()
* 功    能：连续写寄存器或者特殊地址的多个寄存器命令
* 入口参数：*pbuf            将要写入的数据           
*           lenght           写入数据的长度 
* 出口参数：无
* 说    明：连续地址写命令。
******************************************************************************************************************/
void WriteCont(unsigned char *pbuf, unsigned char lenght)
{
#ifdef SPI_BITBANG  
    unsigned char j;
#endif
     
    /* 并行(PAR)模式通信 */
    /*====================================================================================================*/  
    if ((SPIMODE) == 0)
    {
        PARStartCondition();                        //并行模式开始
        *pbuf = (0x20 | *pbuf);                     //取位B5 寄存器地址数据 连续标志位 格式为001XXXXX
        *pbuf = (0x3f & *pbuf);                     //取低6位B0-B5 寄存器地址数据001XXXXX
        while(lenght > 0)
        {
            TRFWrite(*pbuf);                        //发送数据或命令
            CLKON();
            CLKOFF();
            pbuf++;
            lenght--;
        }   /* while */

        PARStopCont();                              //并行模式停止
    } /* if ((SPIMODE) == 0) */
    /*====================================================================================================*/ 
    
    /* 串行(SPI)模式通信 */
    /*====================================================================================================*/
    if (SPIMODE)
    {
#ifndef SPI_BITBANG
        /* 硬件SPI模式 */
        /*-----------------------------------------------------------------------------*/
        L_SlaveSelect();                            //SS管脚输出低，SPI启动
        *pbuf = (0x20 | *pbuf);                     //取位B5 寄存器地址数据 连续标志位 格式为001XXXXX
        *pbuf = (0x3f & *pbuf);                     //取低6位B0-B5 寄存器地址数据
        while(lenght > 0)
        {
            while (!(IFG2 & UCB0TXIFG));            //等待 USCI_B0 TX 缓冲区准备好
            UCB0TXBUF = *pbuf;                      //将数据赋值给TX缓冲区

            while (!(IFG2 & UCB0RXIFG));
            temp=UCB0RXBUF;                         //读取 USCI_B0 RX 缓冲区，来清空该标志位   

            pbuf++;
            lenght--;
        }   /* while */

        H_SlaveSelect();                            //SS管脚输出高，SPI停止
        /*-----------------------------------------------------------------------------*/
#else
        /*  SPI 位模式 */
        /*-----------------------------------------------------------------------------*/
        L_SlaveSelect();                            //SS管脚输出低，SPI启动
        CLKOFF();                                   //CLK时钟关闭（低）

        *pbuf = (0x20 | *pbuf);                     //取位B5 寄存器地址数据 连续标志位 格式为001XXXXX
        *pbuf = (0x3f &*pbuf);                      //取低6位B0-B5 寄存器地址数据
        while(lenght > 0)
        {
            for(j=0;j<8;j++)
            {
                if (*pbuf & mask)                   //设置数据位
                    SIMOON();
                else
                    SIMOOFF();

                CLKON();                            //关联CLK时钟信息，发送数据
                CLKOFF();
                mask >>= 1;                         //标志位右移
            }/*for*/

            mask = 0x80;                            
            pbuf++;
            lenght--;
        }/*while*/

        H_SlaveSelect();                            //SS管脚输出高，SPI停止
        /*-----------------------------------------------------------------------------*/
#endif
    } /*  if (SPIMODE) */
}   /* WriteCont */

/******************************************************************************************************************
* 函数名称：ReadSingle()
* 功    能：读单个寄存器
* 入口参数：*pbuf            将要读取的数据           
*           lenght           读取数据的长度 
* 出口参数：无
* 说    明：无
******************************************************************************************************************/
void ReadSingle(unsigned char *pbuf, unsigned char lenght)
{
#ifdef SPI_BITBANG        
    unsigned char j;
#endif
    /* 并行(PAR)模式通信 */
    /*====================================================================================================*/  
    if((SPIMODE) == 0)
    {
        PARStartCondition();                        //并行模式开始

        while(lenght > 0)
        {
            *pbuf = (0x40 | *pbuf);                 //取位B6 寄存器地址数据 单个标志位 格式为01XXXXXX
            *pbuf = (0x5f & *pbuf);                 //取低7位B0-B6 寄存器地址数据

            TRFWrite(*pbuf);                        //发送数据命令值
            CLKON();                                //关联CLK时钟信息，发送数据
            CLKOFF();

            TRFDirIN();                             //设置并行端口为输入
            CLKON();
            __no_operation();
            TRFRead(*pbuf);                         //读端口寄存器
            CLKOFF();

            TRFWrite(0x00);                         //写端口0x00
            TRFDirOUT();                            //读取完毕 恢复为输出状态

            pbuf++;
            lenght--;
        }   /* while */

        PARStopCondition();                         //并行模式停止
    }   /*if((SPIMODE) == 0)*/
    /*====================================================================================================*/ 

    /* 串行(SPI)模式通信 */
    /*====================================================================================================*/ 
    if (SPIMODE)
    {
#ifndef SPI_BITBANG
        /* 硬件SPI模式 */
        /*-----------------------------------------------------------------------------*/
        L_SlaveSelect();                            //SS管脚输出低，SPI启动

        while(lenght > 0)
        {
            *pbuf = (0x40 | *pbuf);                 //取位B6 寄存器地址数据 单个标志位 格式为01XXXXXX
            *pbuf = (0x5f & *pbuf);                 //取低7位B0-B6 寄存器地址数据

            while (!(IFG2 & UCB0TXIFG));            //等待 USCI_B0 TX 缓冲区准备好
            UCB0TXBUF = *pbuf;                      //将数据赋值给TX缓冲区

            //while (!(IFG2 & UCB0RXIFG));
            //temp=UCB0RXBUF;                       //Remove by POWER 20090518

            UCB0CTL0 &= ~UCCKPH;                    //数据在第1个UCLK时钟被改变，在下降沿被捕获

            SPIStartCondition();                    //开始SPI通信 SCLK为高
            CLKFUNset();                            //设置P3.3管脚为主功能引脚UCB0CLK

            while (!(IFG2 & UCB0TXIFG));            //等待 USCI_B0 TX 缓冲区准备好
            UCB0TXBUF = 0x00;                       //需要增加一个虚拟读来初始化接收

            while (!(IFG2 & UCB0RXIFG));            //等待 USCI_B0 RX 缓冲区准备好
            _NOP();
            _NOP();
            *pbuf = UCB0RXBUF;                      //读取数据
            pbuf++;
            lenght--;

            UCB0CTL0 |= UCCKPH;                     //数据在第1个UCLK时钟被捕获，在下降沿被改变
        }   /* while(lenght > 0) */

        H_SlaveSelect();                            //SS管脚输出高，SPI停止
        /*-----------------------------------------------------------------------------*/
#else
        /*  SPI 位模式 */
        /*-----------------------------------------------------------------------------*/
        L_SlaveSelect();                            //SS管脚输出低，SPI启动

        while(lenght > 0)
        {
            *pbuf = (0x40 | *pbuf);                 //取位B6 寄存器地址数据 单个标志位 格式为01XXXXXX
            *pbuf = (0x5f &*pbuf);                  //取低7位B0-B6 寄存器地址数据
            for(j = 0; j < 8; j++)
            {
                if (*pbuf & mask)                   //设置数据位
                    SIMOON();
                else
                    SIMOOFF();
                
                CLKON();                            //关联CLK时钟信息，发送数据
                CLKOFF();
                mask >>= 1;                         //标志位右移
            }   /* for */
            mask = 0x80;

            *pbuf = 0;                              //开始读取处理
            for(j = 0; j < 8; j++)
            {
                *pbuf <<= 1;                        //数据左移
                CLKON();                            //关联CLK时钟信息，发送数据
                CLKOFF();

                if (SOMISIGNAL)                     //判断SOMI引脚
                    *pbuf |= 1;                     //若为高电平，则将数据或上 1
            }   /* for */

            pbuf++;
            lenght--;
        } /* while */

        H_SlaveSelect();                            //SS管脚输出高，SPI停止
        /*-----------------------------------------------------------------------------*/
#endif
    }   /* if (SPIMODE) */
}   /* ReadSingle */

/******************************************************************************************************************
* 函数名称：ReadCont()
* 功    能：连续读寄存器或者特殊地址的多个寄存器命令
* 入口参数：*pbuf            将要读取的数据           
*           lenght           读取数据的长度 
* 出口参数：无
* 说    明：连续地址写命令。
******************************************************************************************************************/
void ReadCont(unsigned char *pbuf, unsigned char lenght)
{
    unsigned char j;
  
    /* 并行(PAR)模式通信 */
    /*====================================================================================================*/  
    if ((SPIMODE) == 0)
    {
        PARStartCondition();                        //并行模式开始
        *pbuf = (0x60 | *pbuf);                     //取位B6B5寄存器地址数据 连续标志位 格式为011XXXXX
        *pbuf = (0x7f & *pbuf);                     //取低7位B0-B6 寄存器地址数据
        TRFWrite(*pbuf);                            //发送命令
        CLKON();                                    //关联CLK时钟信息，发送数据
        CLKOFF();
        TRFDirIN();                                 //设置并行端口为输入
        
        while(lenght > 0)
        {
            CLKON();                                //CLK时钟高，开始发送数据
            __no_operation();
            TRFRead(*pbuf);                         //读取数据

            CLKOFF();
            pbuf++;
            lenght--;
        }

        PARStopCont();                              //停止并行连续模式
    }
    /*====================================================================================================*/ 

    /* 串行行(SPI)模式通信 */
    /*====================================================================================================*/  
    if (SPIMODE)
    {
#ifndef SPI_BITBANG
        /* 硬件SPI模式 */
        /*-----------------------------------------------------------------------------*/
        L_SlaveSelect();                            //SS管脚输出低，SPI启动
        *pbuf = (0x60 | *pbuf);                     //取位B6B5 寄存器地址数据 连续标志位 格式为011XXXXX
        *pbuf = (0x7f &*pbuf);                      //取低7位B0-B6 寄存器地址数据
        while (!(IFG2 & UCB0TXIFG));                //等待 USCI_B0 TX 缓冲区准备好
        UCB0TXBUF = *pbuf;                          //将数据赋值给TX缓冲区
        
        //while (!(IFG2 & UCB0RXIFG));
        temp=UCB0RXBUF;                             //读取RX缓冲区，以清空其标志位
        UCB0CTL0 &= ~UCCKPH;                        //数据在第1个UCLK时钟被改变，在下降沿被捕获

        /* 修正数据长度，只有IRQ中断读没被调用，才执行以下if语句 */
        /*-----------------------------------------------------------------------------*/
        if(*pbuf != 0x6C)
        {
            if (lenght != 0x1F)
            {
                for (j = 0; j < 2; j++)
                {
                    while (!(IFG2 & UCB0TXIFG));
                    UCB0TXBUF = 0x00;   
                    while (!(IFG2 & UCB0RXIFG));
                    _NOP();
                    _NOP();
                    temp = UCB0RXBUF;
                }
            }
        }
        /*-----------------------------------------------------------------------------*/
        
        while(lenght > 0)
        {
            while (!(IFG2 & UCB0TXIFG));            //等待 USCI_B0 TX 缓冲区准备好                   
            UCB0TXBUF = 0x00;                       //需要增加一个虚拟读来初始化接收

            while (!(IFG2 & UCB0RXIFG));            //等待 USCI_B0 RX 缓冲区准备好
            _NOP();
            _NOP();
            *pbuf = UCB0RXBUF;                      //读取数据
            pbuf++;
            lenght--;
        }
        UCB0CTL0 |= UCCKPH;                         //数据在第1个UCLK时钟被捕获，在下降沿被改变
        H_SlaveSelect();                            //SS管脚输出高，SPI停止
        /*-----------------------------------------------------------------------------*/
#else
        /*  SPI 位模式 */
        /*-----------------------------------------------------------------------------*/
        L_SlaveSelect();                            //SS管脚输出低，SPI启动
        *pbuf = (0x60 | *pbuf);                     //取位B6B5 寄存器地址数据 单个标志位 格式为011XXXXX
        *pbuf = (0x7f & *pbuf);                     //取低7位B0-B6 寄存器地址数据

        for(j = 0; j < 8; j++)                      //写寄存器地址
        {
            if (*pbuf & mask)                       //设置数据位
                SIMOON();
            else
                SIMOOFF();
     
            CLKON();                                //关联CLK时钟信息，发送数据
            CLKOFF();
            mask >>= 1;                             //标志位右移
        }/*for*/
        mask = 0x80;

        while(lenght > 0)                           //开始读取处理
        {
            *pbuf = 0;                              //清空缓冲区
            for(j = 0; j < 8; j++)
            {
                *pbuf <<= 1;                        //数据左移
                CLKON();                            //关联CLK时钟信息，发送数据
                CLKOFF();
                if (SOMISIGNAL)
                    *pbuf |= 1;
            }/*for*/

            pbuf++;                                 //读结束处理
            lenght--;
        }/* while */

        H_SlaveSelect();                            //SS管脚输出高，SPI停止
        /*-----------------------------------------------------------------------------*/
#endif
    } /* if (SPIMODE) */
}   /* ReadCont */

/******************************************************************************************************************
* 函数名称：DirectCommand()
* 功    能：直接命令可发送一个命令到阅读器芯片
* 入口参数：*pbuf            将要发送的命令数据           
* 出口参数：无
* 说    明：直接命令。
******************************************************************************************************************/
void DirectCommand(unsigned char *pbuf)
{
#ifdef SPI_BITBANG  
    unsigned char j;
#endif
   
    /* 并行(PAR)模式通信 */
    /*====================================================================================================*/
    if((SPIMODE) == 0)
    {
        PARStartCondition();                        //并行模式开始
        *pbuf = (0x80 | *pbuf);                     //取位B7 寄存器地址数据 命令标志位 格式为1XXXXXXX
        *pbuf = (0x9f & *pbuf);                     //命令值
        TRFWrite(*pbuf);                            //发送命令
        CLKON();
        CLKOFF();
        PARStopCondition();                         //并行模式停止
    }
    /*====================================================================================================*/
    
    /* 串行(SPI)模式通信 */
    /*====================================================================================================*/
    if (SPIMODE)
    {
#ifndef SPI_BITBANG
        /* 硬件SPI模式 */
        /*-----------------------------------------------------------------------------*/
        L_SlaveSelect();                            //SS管脚输出低，SPI启动
        *pbuf = (0x80 | *pbuf);                     //取位B7 寄存器地址数据 命令标志位 格式为1XXXXXXX
        *pbuf = (0x9f & *pbuf);                     //命令值
        while (!(IFG2 & UCB0TXIFG));                //等待 USCI_B0 TX 缓冲区准备好
        UCB0TXBUF = *pbuf;                          //将数据赋值给TX缓冲区

        //  while (!(IFG2 & UCB0RXIFG));
        temp=UCB0RXBUF;                             //读取 USCI_B0 RX 缓冲区，来清空该标志位 

        SPIStartCondition();                        //开始SPI通信 SCLK为高
        H_SlaveSelect();                            //SS管脚输出高，SPI停止
        CLKFUNset();                                //恢复原来设置
        /*-----------------------------------------------------------------------------*/
#else
        /*  SPI 位模式 */
        /*-----------------------------------------------------------------------------*/
        L_SlaveSelect();                            //SS管脚输出低，SPI启动
        *pbuf = (0x80 | *pbuf);                     //取位B7 寄存器地址数据 命令标志位 格式为1XXXXXXX
        *pbuf = (0x9f & *pbuf);                     //命令值

        for(j = 0; j < 8; j++)                      //写寄存器地址
        {
            if (*pbuf & mask)                       //设置数据位
                SIMOON();
            else
                SIMOOFF();

            CLKON();                                //关联CLK时钟信息，发送数据
            CLKOFF();
            mask >>= 1;                             //标志位右移
        }   /* for */
        mask = 0x80;

        CLKON();                                    //增加额外时钟脉冲
        CLKOFF();
        H_SlaveSelect();                            //SS管脚输出高，SPI停止
        /*-----------------------------------------------------------------------------*/
#endif
    }   /* if (SPIMODE) */
}   /* DirectCommand */


/******************************************************************************************************************
* 函数名称：RAWwrite()
* 功    能：直接写数据或命令到阅读器芯片
* 入口参数：*pbuf           将要发送的命令数据    
*           lenght          写入数据或命令的长度    
* 出口参数：无
* 说    明：直接写。
******************************************************************************************************************/
void RAWwrite(unsigned char *pbuf, unsigned char lenght)
{
#ifdef SPI_BITBANG  
    unsigned char j;
#endif
    /* 并行(PAR)模式通信 */
    /*====================================================================================================*/
    if ((SPIMODE) == 0)
    {
        PARStartCondition();                        //并行模式开始
        while(lenght > 0)
        {
            TRFWrite(*pbuf);                        //发送命令
            CLKON();
            CLKOFF();
            pbuf++;
            lenght--;
        }   /* while */

        PARStopCont();                              //并行模式停止
    }//end Parallel Mode
    /*====================================================================================================*/
    
    /* 串行(SPI)模式通信 */
    /*====================================================================================================*/
    if (SPIMODE)
    {
#ifndef SPI_BITBANG
        /* 硬件SPI模式 */
        /*-----------------------------------------------------------------------------*/
        L_SlaveSelect();                            //SS管脚输出低，SPI启动

        while(lenght > 0)
        {
            while (!(IFG2 & UCB0TXIFG));            //等待 USCI_B0 TX 缓冲区准备好
            UCB0TXBUF = *pbuf;                      //将数据赋值给TX缓冲区
            
            // while (!(IFG2 & UCB0RXIFG));
            temp=UCB0RXBUF;                         //读取 USCI_B0 RX 缓冲区，来清空该标志位 

            pbuf++;
            lenght--;

        }   /* while */

        H_SlaveSelect();                            //SS管脚输出高，SPI停止
        /*-----------------------------------------------------------------------------*/
#else
        /*  SPI 位模式 */
        /*-----------------------------------------------------------------------------*/
        L_SlaveSelect();                            //SS管脚输出低，SPI启动
        while(lenght > 0)
        {
            for(j = 0; j < 8; j++)                  //写寄存器地址
            {
                if (*pbuf & mask)                   //设置数据位
                    SIMOON();
                else
                    SIMOOFF();

                CLKON();                            //关联CLK时钟信息，发送数据
                CLKOFF();
                mask >>= 1;                         //标志位右移
            }   /*for*/
            mask = 0x80;
 
            pbuf++;
            lenght--;
        }   /* while */

        H_SlaveSelect();                            //SS管脚输出高，SPI停止
        /*-----------------------------------------------------------------------------*/
#endif
    }   /* if (SPIMODE) */
}   /* RAWwrite */

/******************************************************************************************************************
* 函数名称：DirectMode()
* 功    能：直接模式，无停止条件
* 入口参数：无 
* 出口参数：无
* 说    明：无
******************************************************************************************************************/
void DirectMode(void)
{
#ifdef SPI_BITBANG  
    unsigned char j;
#endif
    /* 并行(PAR)模式通信 */
    /*====================================================================================================*/
    if ((SPIMODE) == 0)
    {
        OOKdirOUT();                                //设置OOK管脚输出
        PARStartCondition();                        //并行模式开始
        TRFWrite(ChipStateControl);                 //写芯片状态寄存器
        CLKON();
        CLKOFF();
        TRFWrite(0x61);                             //在芯片状态寄存器0x00中BIT6置1
                        
        CLKON();
        CLKOFF();

        TRFDirIN();                                 //设置端口为三态输入
    }
    /*====================================================================================================*/
    
    /* 串行(SPI)模式通信 */
    /*====================================================================================================*/
    if (SPIMODE)
    {
#ifndef SPI_BITBANG
        /* 硬件SPI模式 */
        /*-----------------------------------------------------------------------------*/
        OOKdirOUT();                                //设置OOK管脚输出
        L_SlaveSelect();                            //SS管脚输出低，SPI启动
        while (!(IFG2 & UCB0TXIFG));                //等待 USCI_B0 TX 缓冲区准备好
        UCB0TXBUF = ChipStateControl;               //将芯片状态寄存器数据赋值给TX缓冲区

        while (!(IFG2 & UCB0RXIFG));                //等待 USCI_B0 RX 缓冲区准备好
        temp=UCB0RXBUF;                             //读取 USCI_B0 RX 缓冲区，来清空该标志位 

        while (!(IFG2 & UCB0TXIFG));                //等待 USCI_B0 TX 缓冲区准备好
        UCB0TXBUF = 0x61;                           //在芯片状态寄存器0x00中BIT6置1
        TRFDirIN();                                 //设置端口为三态输入
        /*-----------------------------------------------------------------------------*/
#else
        /*  SPI 位模式 */
        /*-----------------------------------------------------------------------------*/
        OOKdirOUT();                                //设置OOK管脚输出
        L_SlaveSelect();                            //SS管脚输出低，SPI启动
      
        for(j = 0; j < 8; j++)
        {
            if (ChipStateControl & mask)            //发送芯片状态寄存器数据
                SIMOON();
            else
                SIMOOFF();

            CLKON();
            CLKOFF();
            mask >>= 1;
        } /*for*/
        mask = 0x80;

        for(j = 0; j < 8; j++)
        {
            if (0x61 & mask)                        //发送0x61,在芯片状态寄存器0x00中BIT6置1
                SIMOON();
            else
                SIMOOFF();

            CLKON();
            CLKOFF();
            mask >>= 1;
        }/*for*/
        mask = 0x80;

        TRFDirIN();                                 //设置端口为三态输入
        /*-----------------------------------------------------------------------------*/
#endif
    }  /* if (SPIMODE) */
}   /* DirectMode */

/******************************************************************************************************************
* 函数名称：Response()
* 功    能：应答主机命令
* 入口参数：*pbuf       命令值      
*           lenght      命令值长度     
* 出口参数：无
* 说    明：发送应答主机命令字节
******************************************************************************************************************/
void Response(unsigned char *pbuf, unsigned char lenght)
{
    while(lenght > 0)
    {
        kputchar('[');                              //发送符号“[”
        Put_byte(*pbuf);                            //发送命令值
        kputchar(']');                              //发送符号”]“
        pbuf++;
        lenght--;
    }

    put_crlf();                                     //发送回车换行命令
}   /* Response */

/******************************************************************************************************************
* 函数名称：InitialSettings()
* 功    能：初始化TRF7960设置
* 入口参数：无 
* 出口参数：无
* 说    明：设置频率输出及调制深度
******************************************************************************************************************/
void InitialSettings(void)
{
    unsigned char command[2];

    command[0] = ModulatorControl;                  
    command[1] = 0x21;                              // 调制和系统时钟控制：0x21 - 6.78MHz OOK(100%)
    //command[1] = 0x31;                            // 调制和系统时钟控制：0x31 - 13.56MHz OOK(100%)

    WriteSingle(command, 2);
}

/******************************************************************************************************************
* 函数名称：ReInitialize15693Settings()
* 功    能：重新初始化ISO15693设置
* 入口参数：无 
* 出口参数：无
* 说    明：包括重新初始化接收无应答等待时间和接收等待时间
******************************************************************************************************************/
void ReInitialize15693Settings(void)
{
    unsigned char command[2];

    command[0] = RXNoResponseWaitTime;              //接收无应答等待时间
    command[1] = 0x14;
    WriteSingle(command, 2);

    command[0] = RXWaitTime;                        //接收等待时间                   
    command[1] = 0x20;
    WriteSingle(command, 2);
}

/******************************************************************************************************************
* 函数名称：InterruptHandlerReader()
* 功    能：阅读器中断处理程序
* 入口参数：*Register           中断状态寄存器 
* 出口参数：无
* 说    明：处理外部中断服务程序
*           IRQ中断状态寄存器说明如下：
*
*   位      位名称      功能                说明
*   B7      Irq_tx      TX结束而IRQ置位         指示TX正在处理中。该标志位在TX开始时被设置，但是中断请求是在TX完成时才发送。
*   B6      Irq_srx     RX开始而IRQ置位         指示RX SOF已经被接收到并且RX正在处理中。该标志位在RX开始时被设置，但是中断请求是在RX完成时才发送。
*   B5      Irq_fifo            指示FIFO为1/3>FIFO>2/3      指示FIFO高或者低（小于4或者大于8）。
*   B4      Irq_err1            CRC错误             接收CRC
*   B3      Irq_err2            奇偶校验错误                    (在ISO15693和Tag-it协议中未使用)
*   B2      Irq_err3            字节成帧或者EOF错误 
*   B1      Irq_col     冲撞错误            ISO14443A和ISO15693单副载波。
*   B0      Irq_noresp          无响应中断          指示MCU可以发送下一个槽命令。
******************************************************************************************************************/
void InterruptHandlerReader(unsigned char *Register)
{
    unsigned char len;

#if DBG
    Put_byte(*Register);                            //发送入口参数值
#endif

    if(*Register == 0xA0)                           //A0 = 10100000 指示TX发送结束，并且在FIFO中剩下3字节数据
    {                
        i_reg = 0x00;
#if DBG
        kputchar('.');                              //在传送过程中FIFO已经被填充
#endif
    }

    else if(*Register == BIT7)                      //BIT7 = 10000000 指示TX发送结束
    {            
        i_reg = 0x00;
        *Register = Reset;                          //在TX发送结束后 执行复位操作
        DirectCommand(Register);
#if DBG
        kputchar('T');                              //TX发送结束
#endif
    }

    else if((*Register & BIT1) == BIT1)             //BIT1 = 00000010 冲撞错误
    {                           
        i_reg = 0x02;                               //设置RX结束

        *Register = StopDecoders;                   //在TX发送结束后复位FIFO
        DirectCommand(Register);

        CollPoss = CollisionPosition;
        ReadSingle(&CollPoss, 1);

        len = CollPoss - 0x20;                      //获取FIFO中的有效数据字节数量

        if(!POLLING)
	{
            kputchar('{');
            Put_byte(CollPoss);                     //发送冲撞发生的位置
            kputchar('}');
        }
        
        if((len & 0x0f) != 0x00) 
            len = len + 0x10;                       //如果接收到不完整字节，则加上一个字节
        len = len >> 4;

        if(len != 0x00)
        {
            buf[RXTXstate] = FIFO;                  //将接收到的数据写到缓冲区的当前位置                               
            ReadCont(&buf[RXTXstate], len);
            RXTXstate = RXTXstate + len;
        }   /* if */

        *Register = Reset;                          //执行复位命令
        DirectCommand(Register);

        *Register = IRQStatus;                      //获取IRQ中断状态寄存器地址
        *(Register + 1) = IRQMask;

        if (SPIMODE)                                //读取寄存器
            ReadCont(Register, 2);
        else
            ReadSingle(Register, 1);

        IRQCLR();                                   //清中断
    }
    else if(*Register == BIT6)                      //BIT6 = 01000000 接收开始
    {   
        if(RXErrorFlag == 0x02)                     //RX接收标志位指示EOF已经被接收，并且指示在FIFO寄存器中未被读字节的数量
        {
            i_reg = 0x02;
            return;
        }

        *Register = FIFOStatus;
        ReadSingle(Register, 1);                    //读取在FIFO中剩下字节的数量
        *Register = (0x0F & *Register) + 0x01;
        buf[RXTXstate] = FIFO;                      //将接收到的数据写到缓冲区的当前位置
                                                                                	
        ReadCont(&buf[RXTXstate], *Register);
        RXTXstate = RXTXstate +*Register;

        *Register = TXLenghtByte2;                  //读取是否有不完整的字节及其位数量
        ReadCont(Register, 1);

        if((*Register & BIT0) == BIT0)              //00000001 无响应中断
        {
            *Register = (*Register >> 1) & 0x07;    //标记前5位
            *Register = 8 - *Register;
            buf[RXTXstate - 1] &= 0xFF << *Register;
        }   /* if */
         
#if DBG
        kputchar('E');                              //发送无响应标志 E
#endif
        *Register = Reset;                          //最后一个字节被读取后复位FIFO
        DirectCommand(Register);

        i_reg = 0xFF;                               //指示接收函数这些字节已经是最后字节
    }
    else if(*Register == 0x60)                      //0x60 = 01100000 RX已经完毕 并且有9个字节在FIFO中
    {                            
        i_reg = 0x01;                               //设置标志位
        buf[RXTXstate] = FIFO;
        ReadCont(&buf[RXTXstate], 9);               //从FIFO中读取9个字节数据
        RXTXstate = RXTXstate + 9;
#if DBG
        kputchar('F');                              //发送 F 表示FIFO缓冲区满
#endif
        if(IRQPORT & IRQPin)                        //如果中断管脚为高电平
        {
            *Register = IRQStatus;                  //获取IRQ中断状态寄存器地址
            *(Register + 1) = IRQMask;
            if (SPIMODE)                            //读取寄存器
                ReadCont(Register, 2);
            else
                ReadSingle(Register, 1);
            IRQCLR();                               //清中断

            if(*Register == 0x40)                   //0x40 = 01000000 接收结束
            {  
                *Register = FIFOStatus;
                ReadSingle(Register, 1);            //读取在FIFO中剩下字节的数量
                *Register = 0x0F & (*Register + 0x01);
                buf[RXTXstate] = FIFO;              //将接收到的数据写到缓冲区的当前位置
                                                                                	
                ReadCont(&buf[RXTXstate], *Register);
                RXTXstate = RXTXstate +*Register;

                *Register = TXLenghtByte2;          //读取是否有不完整的字节及其位数量
                ReadSingle(Register, 1);         

                if((*Register & BIT0) == BIT0)      //00000001 无响应中断
                {
                    *Register = (*Register >> 1) & 0x07;            //标记前5位
                    *Register = 8 -*Register;
                    buf[RXTXstate - 1] &= 0xFF << *Register;
                }   /* if */
#if DBG
                kputchar('E');                      //发送无响应标志 E
#endif
                i_reg = 0xFF;                       //指示接收函数这些字节已经是最后字节
                *Register = Reset;                  //在最后字节被读取后复位FIFO
                DirectCommand(Register);
            }
            else if(*Register == 0x50)              //0x50 = 01010000接收结束并且发生CRC错误
            {        
                i_reg = 0x02;
#if DBG
                kputchar('x');                      //发送CRC校验错误标志 x
#endif
            }
        }   /* if(IRQPORT & IRQPin) */
        else                                        
        {
            Register[0] = IRQStatus;                //获取IRQ中断状态寄存器地址
            Register[1] = IRQMask;
            if (SPIMODE)
                ReadCont(Register, 2);              //读取寄存器
            else
                ReadSingle(Register, 1);
            
            if(Register[0] == 0x00) 
              i_reg = 0xFF;                         //指示接收函数这些字节已经是最后字节
        }
    }
    else if((*Register & BIT4) == BIT4)             //BIT4 = 00010000 指示CRC错误
    {                      
        if((*Register & BIT5) == BIT5)              //BIT5 = 00100000 指示FIFO中有9个字节
        {
            i_reg = 0x01;                           //接收完成
            RXErrorFlag = 0x02;
        }
        else
            i_reg = 0x02;                           //停止接收        
    }
    else if((*Register & BIT2) == BIT2)             //BIT2 = 00000100  字节成帧或者EOF错误
    {                       
        if((*Register & BIT5) == BIT5)              //BIT5 = 00100000 指示FIFO中有9个字节
        {
            i_reg = 0x01;                           //接收完成
            RXErrorFlag = 0x02;
        }
        else
            i_reg = 0x02;                           //停止接收 
    }
    else if(*Register == BIT0)                      //BIT0 = 00000001 中断无应答
    {                      
        i_reg = 0x00;
#if DBG
        kputchar('N');
#endif
    }
    else
    {     
        if(!POLLING)
	{
            send_cstring("Interrupt error");        //发送中断错误
            Put_byte(*Register);
        }
        
        i_reg = 0x02;

        *Register = StopDecoders;                   //在TX发送接收后复位FIFO
        DirectCommand(Register);

        *Register = Reset;
        DirectCommand(Register);

        *Register = IRQStatus;                      //获取IRQ中断状态寄存器地址
        *(Register + 1) = IRQMask;

        if (SPIMODE)
            ReadCont(Register, 2);                  //读取寄存器
        else
            ReadSingle(Register, 1);
        
        IRQCLR();                                   //清中断
    }
}   /* InterruptHandlerReader */

/******************************************************************************************************************
* 函数名称：Port_B()
* 功    能：阅读器中断入口程序
* 入口参数：无
* 出口参数：无
* 说    明：处理外部中断服务程序
******************************************************************************************************************/
#pragma vector = PORT2_VECTOR
__interrupt void Port_B (void)      
{
    unsigned char Register[4];

    StopCounter();                                  //定时器停止

    do
    {
        IRQCLR();                                   //清端口2中断标志位
        Register[0] = IRQStatus;                    //获取IRQ中断状态寄存器地址
        Register[1] = IRQMask;                      //虚拟读 Dummy read
  
        if (SPIMODE)                                //读取寄存器
            ReadCont(Register, 2);
        else
            ReadSingle(Register, 1); 

        if(*Register == 0xA0)                       //A0 = 10100000 指示TX发送结束，并且在FIFO中剩下3字节数据
        {   
            goto FINISH;                            //跳转到FINISH处，进入低功耗模式
        }
        
        InterruptHandlerReader(&Register[0]);       //执行中断服务程序
    }while((IRQPORT & IRQPin) == IRQPin);           //条件执行
FINISH:
    __low_power_mode_off_on_exit();
}



