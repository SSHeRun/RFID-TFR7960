//**************************************************************************************
#ifndef _7960_H_INCLUDED_
#define _7960_H_INCLUDED_
//**************************************************************************************
//全局变量
extern char buffer[12];
extern char rec_length;
extern char tagtype[2];
extern char UID[5];
//TI Transceive IC接口函数声明
extern void spi_delay (char loop);
extern void start_bit (void);
extern void stop_bit (void);
extern void byte_send(char sendbyte);
extern char spi_receive(void);
extern void initial_7960 (void);
extern char signle_register_read(char reg_ad);
extern void signle_command_send(char command_code);
extern void signle_register_write(char reg_ad,char reg_data);
extern char FIFO_length( void );
extern char Clear_FIFO(void);
extern char Read_FIFO(char * buff);
extern char IRQ_status(void);
extern char data_transceive(char byte_count,char broken_bit,char any_crc,char * buff);
extern char Request(char mode);
extern char AntiColl(void);
//*******************函数错误代码定义********************
#define		TRF7960_OK			0		//正确
#define		TRF7960_NOTAGERR		1		//无卡
#define		TRF7960_CRCERR			2		//卡片CRC校验错误
#define		TRF7960_EMPTY			3		//数值溢出错误
#define		TRF7960_AUTHERR			4		//验证不成功
#define		TRF7960_PARITYERR		5		//卡片奇偶校验错误
#define		TRF7960_CODEERR			6		//通讯错误(BCC校验错)
#define		TRF7960_SERNRERR		8		//卡片序列号错误(anticol错误)
#define		TRF7960_SELECTERR		9		//卡片数据长度字节错误(SELECT错误)
#define		TRF7960_NOTAUTHERR		10		//卡片没有通过验证
#define		TRF7960_BITCOUNTERR		11		//从卡片接收到的位数错误
#define		TRF7960_BYTECOUNTERR		12		//从卡片接收到的字节数错误（仅仅读函数有效）
#define		TRF7960_RESTERR			13		//调用restore函数出错
#define		TRF7960_TRANSERR		14		//调用transfer函数出错
#define		TRF7960_WRITEERR		15		//调用write函数出错
#define		TRF7960_INCRERR			16		//调用increment函数出错
#define		TRF7960_DECRERR			17		//调用decrement函数出错
#define		TRF7960_READERR			18		//调用read函数出错
#define 	TRF7960_LOADKEYERR	        19		//调用LOADKEY函数出错
#define         TRF7960_FRAMINGERR		20              //TRF7960帧错误
#define         TRF7960_REQERR			21              //调用req函数出错
#define  	TRF7960_SELERR		        22		//调用sel函数出错
#define        	TRF7960_ANTICOLLERR		23              //调用anticoll函数出错
#define 	TRF7960_INTIVALERR	        24		//调用初始化函数出错
#define 	TRF7960_READVALERR	        25		//调用高级读块值函数出错
#define		TRF7960_CMD_ERR			42		//命令错误
#define         TRF7960_COLLERR                 26

//TRF7960 管脚定义  
    //EN
    #define TRF_7960_EN_SEL     P2SEL &= ~BIT1
    #define TRF_7960_EN_OUT     P2DIR |= BIT1
    #define TRF_7960_EN_IN      P2DIR &= ~BIT1
    #define TRF_7960_EN_HIGH    P2OUT |= BIT1
    #define TRF_7960_EN_LOW     P2OUT &= ~BIT1
    /*//EN2
    #define TRF_7960_EN_SEL     P1SEL &= ~BIT5
    #define TRF_7960_EN_OUT     P1DIR |= BIT5
    #define TRF_7960_EN_IN      P1DIR &= ~BIT5
    #define TRF_7960_EN_HIGH    P1OUT |= BIT5
    #define TRF_7960_EN_LOW     P1OUT &= ~BIT5*/
    
    //IRQ
    #define TRF_7960_IRQ_SEL    P1SEL &= ~BIT3
    #define TRF_7960_IRQ_IN     P1DIR &= ~BIT3
    #define TRF_7960_IRQ        (P1IN & BIT3)
    
    //MOD
    #define TRF_7960_MOD_SEL    P2SEL &=~ BIT0
    #define TRF_7960_MOD_OUT    P2DIR |= BIT0
    #define TRF_7960_MOD_NO_DIRECT   P2OUT &= ~BIT0
    #define TRF_7960_MOD_DIRECT      P2OUT |= BIT0
    
    //OOK
    #define TRF_7960_OOK_SEL    P3SEL &= ~BIT6
    #define TRF_7960_00K_OUT    P3DIR |= BIT6
    #define TRF_7960_OOK_IN     P3DIR &= ~BIT6
    #define TRF_7960_OOK        P3OUT |= BIT6
    #define TRF_7960_ASK        P3OUT &= ~BIT6
    
    // I/O
    /*  //IO0
    #define TRF_7960_IO0_SEL    P2SEL &= ~BIT0
    #define TRF_7960_IO0_OUT    P2DIR |= BIT0
    #define TRF_7960_IO0_HIGH   P2OUT |= BIT0
    #define TRF_7960_IO0_LOW    P2OUT &= ~BIT0
    
    //IO1
    #define TRF_7960_IO1_SEL    P2SEL &= ~BIT1
    #define TRF_7960_IO1_OUT    P2DIR |= BIT1
    #define TRF_7960_IO1_HIGH   P2OUT |= BIT1
    #define TRF_7960_IO1_LOW    P2OUT &= ~BIT1
    
    //IO2
    #define TRF_7960_IO2_SEL    P2SEL &= ~BIT2
    #define TRF_7960_IO2_OUT    P2DIR |= BIT2
    #define TRF_7960_IO2_HIGH   P2OUT |= BIT2
    #define TRF_7960_IO2_LOW    P2OUT &= ~BIT2
    
    //IO3
    #define TRF_7960_IO3_SEL    P2SEL &= ~BIT3
    #define TRF_7960_IO3_OUT    P2DIR |= BIT3
    #define TRF_7960_IO3_HIGH   P2OUT |= BIT3
    #define TRF_7960_IO3_LOW    P2OUT &= ~BIT3
    
    //IO4
    #define TRF_7960_IO4_SEL    P2SEL &= ~BIT4
    #define TRF_7960_IO4_OUT    P2DIR |= BIT4
    #define TRF_7960_IO4_HIGH   P2OUT |= BIT4
    #define TRF_7960_IO4_LOW    P2OUT &= ~BIT4
    */
    //IO5
    #define TRF_7960_IO5_SEL    P3SEL &= ~BIT0
    #define TRF_7960_IO5_OUT    P3DIR |= BIT0
    #define TRF_7960_IO5_IN     P3DIR &= ~BIT0
    #define TRF_7960_IO5_HIGH   P3OUT |= BIT0
    #define TRF_7960_IO5_LOW    P3OUT &= ~BIT0
    #define TRF_7960_IO5        P3IN  & BIT0
     
    //IO6
    #define TRF_7960_IO6_SEL    P3SEL &= ~BIT2
    #define TRF_7960_IO6_OUT    P3DIR |= BIT2
    #define TRF_7960_IO6_IN     P3DIR &= ~BIT2
    #define TRF_7960_IO6_HIGH   P3OUT |= BIT2
    #define TRF_7960_IO6_LOW    P3OUT &= ~BIT2
    #define TRF_7960_IO6        P3IN  & BIT2
    
    //IO7
    #define TRF_7960_IO7_SEL    P3SEL &= ~BIT1
    #define TRF_7960_IO7_OUT    P3DIR |= BIT1
    #define TRF_7960_IO7_HIGH   P3OUT |= BIT1
    #define TRF_7960_IO7_LOW    P3OUT &= ~BIT1
    
    //SPI 通讯接口
    //CLK
    #define SPI_CLK_SEL         P3SEL &= ~BIT3
    #define SPI_CLK_OUT         P3DIR |= BIT3
    #define SPI_CLK_HIGH        P3OUT |= BIT3
    #define SPI_CLK_LOW         P3OUT &= ~BIT3   
   
    //DIN
    #define SPI_IN_SEL          TRF_7960_IO6_SEL
    #define SPI_IN_IN           TRF_7960_IO6_IN
    #define SPI_IN              TRF_7960_IO6
    
    //DOUT
    #define SPI_OUT_SEL         TRF_7960_IO7_SEL
    #define SPI_OUT_OUT         TRF_7960_IO7_OUT
    #define SPI_OUT_HIGH        TRF_7960_IO7_HIGH
    #define SPI_OUT_LOW         TRF_7960_IO7_LOW
    
    
    //STROB
    #define SPI_STROBE_SEL      TRF_7960_IO5_SEL    
    #define SPI_STROBE_OUT      TRF_7960_IO5_OUT
    #define SPI_STROBE_IN       TRF_7960_IO5_IN
    #define SPI_STROBE_HIGH     TRF_7960_IO5_HIGH
    #define SPI_STROBE_LOW      TRF_7960_IO5_LOW
    #define SPI_STROBE          TRF_7960_IO5
    //for TEST
    #define SPI_TX_SEL       P3SEL&=~BIT4
    #define SPI_TX_OUT       P3DIR|=BIT4
    #define SPI_TX_HIGH      P3OUT|=BIT4
    #define SPI_TX_LOW       P3OUT&=~BIT4  
    
    #define SPI_RX_SEL       P3SEL&=~BIT5
    #define SPI_RX_OUT       P3DIR|=BIT5
    #define SPI_RX_HIGH      P3OUT|=BIT5
    #define SPI_RX_LOW       P3OUT&=~BIT5  
     
//TRF7960 命令定义

#define comm_IDLE                0x00
#define comm_SOFTINIT            0x03
#define comm_REST                0x0F
#define comm_TRANSMITNOCRC       0x10
#define comm_TRANSMITCRC         0x11
#define comm_DELAYTRNOCRC        0x12
#define comm_DELAYTRCRC          0x13
#define comm_TRANSMITNEXT        0x14       //Transmit next slot(15693,Tag-it)
#define comm_CLOSESLOT           0x15
#define comm_BLOCKRX             0x16
#define comm_ENRX                0x17
#define comm_CHKINTERRF          0X18
#define comm_CHKOUTRF            0X19
#define comm_ADJUSTGAIN          0X1A


//*******************ISO/IEC FDIS 14443 相关定义 *******************
   // TYPE A
#define REQA                    0x26
#define WUPA                    0x52
#define ATQA_BYTE0              0x04
#define ATQA_BYTE1              0x00
#define SAK_BYTE0               0x80
#define SAK_BYTE1               0x88
#define HLTA_BYTE0              0x50
#define HLTA_BYTE1              0x00
  // TYPE B


//射频卡通信命令码定义
#define		RF_CMD_REQUEST_STD 		0x26
#define		RF_CMD_REQUEST_ALL 		0x52
#define		RF_CMD_ANTICOL 			0x93
#define		RF_CMD_SELECT 			0x93
#define		RF_CMD_AUTH_LA 			0x60
#define		RF_CMD_AUTH_LB 			0x61
#define		RF_CMD_READ				0x30
#define		RF_CMD_WRITE			0xa0
#define		RF_CMD_INC				0xc1
#define		RF_CMD_DEC				0xc0
#define		RF_CMD_RESTORE			0xc2
#define		RF_CMD_TRANSFER			0xb0
#define		RF_CMD_HALT				0x50
//**************************************************************************************
#endif
//**************************************************************************************
