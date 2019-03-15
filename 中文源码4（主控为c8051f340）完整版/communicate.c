/********************************************************************************************************
* 文 件 名：COMMUNICATE.H
* 功    能：RFID阅读器TRF7960与C8051F340微控制器之间通信方式头文件。
* 硬件连接：C8051F340与TRF7960之间通信硬件连接关系如下所示：
*                C8051F340                 TRF7960
*********************    PARALLEL INTERFACE    ******************************************         
*               P0.7   				 IRQ
*			    P0.3                 Slave_select
*               P0.2                 SIMO
*               P0.1                 SOMI
*               P0.0                 DATA_CLK
*				P4.0		       	 MOD
*				P4.2				 ASK/OOK
*				P4.3				 EN
*
* 版    本：V1.0
* 作    者：EMDOOR
* 日    期：2011-9-29
*********************************************************************************************************/
#include <communicate.h>
#include <globals.h>
#include <hardware.h>
#include <c8051f340.h>
#define DBG  0
static unsigned char temp;
static unsigned int mask = 0x80;

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
	unsigned char i,j;
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
    
    unsigned char j;
     
    /*====================================================================================================*/     
    /* 串行(SPI)模式通信 */
    /*====================================================================================================*/
		/*-----------------------------------------------------------------------------*/
		 /* 硬件SPI模式 */
		/*-----------------------------------------------------------------------------*/
		L_SlaveSelect();                             //SS管脚输出低，SPI启动
		CLKOFF();                                   //CLK时钟关闭（低）

		*pbuf = (0x20 | *pbuf);                     //取位B5 寄存器地址数据 连续标志位 格式为001XXXXX
		*pbuf = (0x3f &*pbuf);                      //取低6位B0-B5 寄存器地址数据
		while(lenght > 0)
		{
				for(j=0;j<8;j++)
				{
						if (*pbuf & mask)                    //设置数据位
								SIMOON();
						else
								SIMOOFF();

						CLKON();                             //关联CLK时钟信息，发送数据
						CLKOFF();
						mask >>= 1;                        //标志位右移
				}/*for*/

				mask = 0x80;                            
				pbuf++;
				lenght--;
		}/*while*/

		H_SlaveSelect();                            //SS管脚输出高，SPI停止
		/*-----------------------------------------------------------------------------*/
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
 unsigned char j;
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
}   /* ReadCont */

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
		
}

/******************************************************************************************************************
* 函数名称：DirectCommand()
* 功    能：直接命令可发送一个命令到阅读器芯片
* 入口参数：*pbuf            将要发送的命令数据           
* 出口参数：无
* 说    明：直接命令。
******************************************************************************************************************/
void DirectCommand(unsigned char *pbuf)
{
	unsigned char j;
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
	 unsigned char j;
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

}   /* RAWwrite */

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
    //command[1] = 0x21;                              // 调制和系统时钟控制：0x21 - 6.78MHz OOK(100%)
    command[1] = 0x31;
	WriteSingle(command,2);

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
*   B5      Irq_fifo    指示FIFO为1/3>FIFO>2/3      指示FIFO高或者低（小于4或者大于8）。
*   B4      Irq_err1    CRC错误             接收CRC
*   B3      Irq_err2    奇偶校验错误                    (在ISO15693和Tag-it协议中未使用)
*   B2      Irq_err3    字节成帧或者EOF错误 
*   B1      Irq_col     冲撞错误            ISO14443A和ISO15693单副载波。
*   B0      Irq_noresp  无响应中断          指示MCU可以发送下一个槽命令。
******************************************************************************************************************/
void InterruptHandlerReader(unsigned char *Register)
{
    unsigned char len;

#if DBG
    send_byte(*Register);                            //发送入口参数值
#endif

    if(*Register == 0xA0)                           //A0 = 10100000 指示TX发送结束，并且在FIFO中剩下3字节数据
    {                
        i_reg = 0x00;
#if DBG
        sendchar('.');                              //在传送过程中FIFO已经被填充
#endif
    }

    else if(*Register == 0x80)                      //BIT7 = 10000000 指示TX发送结束
    {            
        i_reg = 0x00;
        *Register = Reset;                          //在TX发送结束后 执行复位操作
        DirectCommand(Register);
#if DBG
        sendchar('T');                              //TX发送结束
#endif
    }

    else if((*Register & 0x02) == 0x02)             //BIT1 = 00000010 冲撞错误
    {                           
        i_reg = 0x02;                               //设置RX结束

        *Register = StopDecoders;                   //在TX发送结束后复位FIFO
        DirectCommand(Register);

        CollPoss = CollisionPosition;
        ReadSingle(&CollPoss, 1);

        len = CollPoss - 0x20;                      //获取FIFO中的有效数据字节数量
#if DBG
        sendchar('{');
        send_byte(CollPoss);                     //发送冲撞发生的位置
        sendchar('}');
#endif     
        
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

            ReadCont(Register, 2);
   
        IRQCLR();                                   //清中断
    }
    else if(*Register == 0x40)                      //BIT6 = 01000000 接收开始
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

        if((*Register & 0x01) == 0x01)              //00000001 无响应中断
        {
            *Register = (*Register >> 1) & 0x07;    //标记前5位
            *Register = 8 - *Register;
            buf[RXTXstate - 1] &= 0xFF << *Register;
        }   /* if */
         
#if DBG
        sendchar('E');                              //发送无响应标志 E
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
        sendchar('F');                              //发送 F 表示FIFO缓冲区满
#endif
        if(IRQPORT & IRQPin)                        //如果中断管脚为高电平
        {
            *Register = IRQStatus;                  //获取IRQ中断状态寄存器地址
            *(Register + 1) = IRQMask;
                            //读取寄存器
                ReadCont(Register, 2);
         
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

                if((*Register & 0x01) == 0x01)      //00000001 无响应中断
                {
                    *Register = (*Register >> 1) & 0x07;            //标记前5位
                    *Register = 8 -*Register;
                    buf[RXTXstate - 1] &= 0xFF << *Register;
                }   /* if */
#if DBG
                sendchar('E');                      //发送无响应标志 E
#endif
                i_reg = 0xFF;                       //指示接收函数这些字节已经是最后字节
                *Register = Reset;                  //在最后字节被读取后复位FIFO
                DirectCommand(Register);
            }
            else if(*Register == 0x50)              //0x50 = 01010000接收结束并且发生CRC错误
            {        
                i_reg = 0x02;
#if DBG
                sendchar('x');                      //发送CRC校验错误标志 x
#endif
            }
        }   /* if(IRQPORT & IRQPin) */
        else                                        
        {
            Register[0] = IRQStatus;                //获取IRQ中断状态寄存器地址
            Register[1] = IRQMask;
            
                ReadCont(Register, 2);              //读取寄存器
          
            
            if(Register[0] == 0x00) 
              i_reg = 0xFF;                         //指示接收函数这些字节已经是最后字节
        }
    }
    else if((*Register & 0x10) == 0x10)             //BIT4 = 00010000 指示CRC错误
    {                      
        if((*Register & 0x20) == 0x20)              //BIT5 = 00100000 指示FIFO中有9个字节
        {
            i_reg = 0x01;                           //接收完成
            RXErrorFlag = 0x02;
        }
        else
            i_reg = 0x02;                           //停止接收        
    }
    else if((*Register & 0x04) == 0x04)             //BIT2 = 00000100  字节成帧或者EOF错误
    {                       
        if((*Register & 0x20) == 0x20)              //BIT5 = 00100000 指示FIFO中有9个字节
        {
            i_reg = 0x01;                           //接收完成
            RXErrorFlag = 0x02;
        }
        else
            i_reg = 0x02;                           //停止接收 
    }
    else if(*Register == 0x01)                      //BIT0 = 00000001 中断无应答
    {                      
        i_reg = 0x00;
#if DBG
        sendchar('N');
#endif
    }
    else
    {     
#if DBG    
        send_cstring("Interrupt error");        //发送中断错误
        send_byte(*Register);
#endif        
        i_reg = 0x02;

        *Register = StopDecoders;                   //在TX发送接收后复位FIFO
        DirectCommand(Register);

        *Register = Reset;
        DirectCommand(Register);

        *Register = IRQStatus;                      //获取IRQ中断状态寄存器地址
        *(Register + 1) = IRQMask;

       
            ReadCont(Register, 2);                  //读取寄存器
        
        IRQCLR();                                   //清中断
    }
}   /* InterruptHandlerReader */

/********************************************************************************************************
* 函数名称：Port_0()
* 功    能：阅读器中断入口程序
* 入口参数：无
* 出口参数：无
* 说    明：处理外部中断服务程序
*********************************************************************************************************/
void Port_0(void) interrupt 0     
{
    unsigned char Register[4];

    StopCounter();                                  //定时器停止
		
    do
    {
        IRQCLR();                                   //清端口2中断标志位
        Register[0] = IRQStatus;                    //获取IRQ中断状态寄存器地址
        Register[1] = IRQMask;                      //虚拟读 Dummy read                                 
        ReadCont(Register, 2); 											//读取寄存器
        if(*Register == 0xA0)                       //A0 = 10100000 指示TX发送结束，并且在FIFO中剩下3字节数据
        {   
            goto FINISH;                            //跳转到FINISH处，进入低功耗模式
        }
        
        InterruptHandlerReader(Register);       //执行中断服务程序
				
    }while((IRQPORT & IRQPin) == IRQPin);           //条件执行
FINISH:
   PCON &= ~0X03;                                   //退出idle和stop的状态

}



