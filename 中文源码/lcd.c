/**********************************************************************************************************************************************************
* 文 件 名：LCD.C
* 功    能：HO12864FPD-14CSBE（3S）位图形液晶模块（ST7565P）驱动。
* 硬件连接：HO12864FPD-14CSBE（3S）位图形液晶模块（ST7565P）与MSP430F2370的硬件连接关系如下所示：
*           HO12864FPD-14CSBE(3S)                MSP430F2370
*
*               SCL(PIN7)                         P1.6
*               SID(PIN8)                         P1.7
*               A0(PIN14)                         P3.6
*               CSn(PIN12)                        P2.4
*               RESETn(PIN13)                     P2.5
*               VDD(PIN9)
*               VSS(PIN10)
*               LED+(PIN11)
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
#include "data.h"


/******************************************************************************************************************
* 函数名称：Delay_lcm()
* 功    能：软件延时
* 入口参数：count    延时参数，值越大，延时越长
* 出口参数：无
* 说    明：延时等待用。
******************************************************************************************************************/
void Delay_lcm(unsigned int count)
{
    unsigned char i;
  
    for(; count > 0; count--)
        for(i = 0; i < 15; i++);
}
 
/******************************************************************************************************************
* 函数名称：Data_Send()
* 功    能：LCD串行模式发送数据
* 入口参数：data    要发送的数据
* 出口参数：无
* 说    明：MSP430微控制器向LCD缓冲区发送数据
******************************************************************************************************************/
void Data_Send(unsigned char data)
{
    unsigned char s, tmp, i;                        //定义变量
    
    L_LCM_SCL();                                    //拉低SCL控制线
    s = data;                                       //将数据暂存给变量s
  
    for(i = 8; i > 0; i--)                          //串行模式下，需要循环8次，方能将数据传送出去
    {
        L_LCM_SCL();                                //拉低SCL控制线
        Delay_lcm(2);
        tmp = s & 0x80;                             //取变量s的最高位B7
      
        if(tmp)                                     //判断高低电平，并发送
            H_LCM_SID();
        else
            L_LCM_SID();
        H_LCM_SCL();                                //拉高SCL控制线
        
        s = s << 1;                                 //左移一位，依次类推
    }             
}

/******************************************************************************************************************
* 函数名称：Write_CMD()
* 功    能：LCD串行模式发送命令
* 入口参数：command    命令值
* 出口参数：无
* 说    明：MSP430微控制器向LCD控制器发送命令
******************************************************************************************************************/
void Write_CMD(unsigned char command)
{
    L_LCM_A0();                                     //拉低A0控制线
    L_LCM_nCS();                                    //拉低CS片选信号线
    
    Data_Send(command);                             //发送命令值数据
}

/******************************************************************************************************************
* 函数名称：Write_Data()
* 功    能：写数据
* 入口参数：data    数据
* 出口参数：无
* 说    明：MSP430微控制器向LCD控制器发送数据
******************************************************************************************************************/
void Write_Data(unsigned char data)
{
    H_LCM_A0();                                     //拉高A0控制线
    L_LCM_nCS();                                    //拉低CS片选信号线
    
    Data_Send(data);                                //发送命令值数据            
}

/******************************************************************************************************************
* 函数名称：Display_Clear()
* 功    能：清空LCD显示
* 入口参数：无
* 出口参数：无
* 说    明：执行该函数，可以清除屏幕任何显示内容
******************************************************************************************************************/
void Display_Clear(void)
{
    unsigned char seg;                              //定义LCD显示列
    unsigned char page;                             //定义LCD显示页

    for(page = 0xb0; page < 0xb9; page++)           //页地址共8页：0xb0 - 0xb8
    {
        Write_CMD(page);                            //设置页地址
        Write_CMD(0x10);                            //设置列地址，高低字节两次写入，从第0列开始
        Write_CMD(0x00);
        
        for(seg = 0; seg < 132; seg++)              //写128列，由于本液晶为反向安装故其范围为0-131
            Write_Data(0x00);
    }
}

/******************************************************************************************************************
* 函数名称：Display_Picture()
* 功    能：显示一张128*64整屏图片
* 入口参数：*p       图片数据的首地址指针
* 出口参数：无
* 说    明：执行该函数，首先需指定欲显示的图片首地址。
******************************************************************************************************************/
void Display_Picture(unsigned char *p)
{
    unsigned char seg;                              //定义LCD显示列
    unsigned char page;                             //定义LCD显示页

    for(page = 0xb0; page < 0xb9; page++)           //页地址共8页：0xb0 - 0xb8
    {
        Write_CMD(page);                            //设置页地址
        Write_CMD(0x10);                            //设置列地址，高低字节两次写入，从第0列开始
        Write_CMD(0x00);
        for(seg = 0; seg < 128; seg++)              //写128列数据
            Write_Data(*p++);
    }
}

/******************************************************************************************************************
* 函数名称：Display_Chinese()
* 功    能：在指定的位置上，显示一个汉字
* 入口参数：pag       显示汉字的页地址
*           col       显示汉字的列地址
*           *hzk      显示汉字的地址指针
* 出口参数：无
* 说    明：若该函数操作不成功，是由于输入错误坐标造成
******************************************************************************************************************/
void Display_Chinese(unsigned char pag, unsigned char col, unsigned char const *hzk)
{
    unsigned char j = 0, i = 0;
    unsigned char page; 
    unsigned char row;
   
    /* 参数过滤 */
    /*===========================================================================*/
    if(pag > 7)return;                              // pag坐标越界，返回
    if(col > 7)return;                              // col坐标越界，返回
    /*===========================================================================*/  
    
    row = col * 16 + 4;                             //重新计算得到显示列地址，由于LCD被反向安装，故需 +4 
    page = 0xb0 + pag;                              //得到页地址
    
    for(j = 0; j < 2; j++)                          //分2页进行写入                     
    {
        Write_CMD(page + j);                        //当j=0时，为第1页，当j=1时，第2页
        Write_CMD(0x10 + ((row >> 4) & 0x0f));      //设置列地址
        Write_CMD(row & 0x0f);                      //高低字节两次写入  
        
        for(i = 0; i < 16; i++) 
            Write_Data(hzk[16 * j + i]);            //写入点阵数据
    }
}

/******************************************************************************************************************
* 函数名称：Display_Char()
* 功    能：在指定的位置上，显示一个数字或者字母
* 入口参数：pag       显示汉字的页地址
*           col       显示汉字的列地址
*           *hzk      显示汉字的地址指针
* 出口参数：无
* 说    明：若该函数操作不成功，是由于输入错误坐标造成
******************************************************************************************************************/
void Display_Char(unsigned char pag,unsigned char col, unsigned char const *hzk)
{
    unsigned char j = 0, i = 0;
    unsigned char page; 
    unsigned char row;
  
    /* 参数过滤 */
    /*===========================================================================*/
    if(pag > 7)return;                              // pag坐标越界，返回
    if(col > 15)return;                             // col坐标越界，返回
    /*===========================================================================*/  
    
    row = col * 8 + 4;                              //重新计算得到显示列地址，由于LCD被反向安装，故需 +4 
    page = 0xb0 + pag;                              //得到页地址
    
    for(j = 0; j < 2; j++)                          //分2页进行写入   
    {
        Write_CMD(page + j);                        //当j=0时，为第1页，当j=1时，第2页
        Write_CMD(0x10 + ((row >> 4) & 0x0f));      //设置列地址
        Write_CMD(row & 0x0f);                      //高低字节两次写入  
        
        for(i = 0; i < 8; i++)                      //写入点阵数据
            Write_Data(hzk[8 * j + i]);
    }
}

/******************************************************************************************************************
* 函数名称：LCM12864_Init()
* 功    能：LCM12864模块初始化
* 入口参数：无
* 出口参数：无
* 说    明：初始化LCD显示器
******************************************************************************************************************/
void LCM12864_Init(void)
{ 
    /* LCM控制管脚方向设置 */
    /*===========================================================================*/
    LCMA0Set();                                     //A0设置输出
    LCMnCSSet();                                    //CS设置输出
    LCMnRSTSet();                                   //RST复位输出
    /*===========================================================================*/
    
    L_LCM_nRST();
    Delay_lcm(2);
    H_LCM_nRST();                                   //LCM复位
  
    Write_CMD(0xaf);                                //AF显示器开；AE显示器关
    Write_CMD(0x40);                                //0x40来指定显示RAM 的起始行地址
    //Write_CMD(0xa0);                              //A0 模块正向安装，列地址从左到右为0~127
    Write_CMD(0xa1);                                //A1 模块反向安装，列地址从左到右为131~4
    Write_CMD(0xa6);                                //A6正常显示，A7反白显示
    Write_CMD(0xa4);                                //A4全屏显示，不能改变RAM数据；A5可以改变
    Write_CMD(0xa2);                                //设置LCD 的偏压比1/9
    //Write_CMD(0xc8);                              //模块正向安装时,上端为第0行
    Write_CMD(0xc0);                                //模块反向安装时,下端为第0行
    Write_CMD(0x2f);                                //设置开关内部电路的电源
    Write_CMD(0x24);                                //24，81，24粗调对比度
    Write_CMD(0x81);
    Write_CMD(0x24);

    Write_CMD(0xac);                                //static indicator off  
    Write_CMD(0x00);
}

/******************************************************************************************************************
* 函数名称：Display_Put_Tag()
* 功    能：LCM12864模块显示请放入卡片信息
* 入口参数：无
* 出口参数：无
* 说    明：显示提示信息
******************************************************************************************************************/
void Display_Put_Tag(void)
{
    Display_Clear();                                //清屏
    
    Display_Chinese(3,1,qing);                      //显示“请放入卡片”信息
    Display_Chinese(3,2,fang);
    Display_Chinese(3,3,ru);
    Display_Chinese(3,4,ka);
    Display_Chinese(3,5,pian);
    Display_Char(3,12,point);
    Display_Char(3,13,point);
    Display_Char(3,14,point);
}

/******************************************************************************************************************
* 函数名称：Display_Start()
* 功    能：LCM12864模块显示系统启动信息
* 入口参数：无
* 出口参数：无
* 说    明：显示启动信息
******************************************************************************************************************/
void Display_Start(void)
{
    unsigned char i;

    Display_Chinese(0,2,(unsigned char *)huan);     //显示“欢迎使用”信息
    Display_Chinese(0,3,(unsigned char *)ying);
    Display_Chinese(0,4,(unsigned char *)shi);
    Display_Chinese(0,5,(unsigned char *)yong);

    Display_Char(2,1,(unsigned char *)CR);          //显示“RFID(13.56MHz)”
    Display_Char(2,2,(unsigned char *)Num+0x0F*0x10);
    Display_Char(2,3,(unsigned char *)CI);
    Display_Char(2,4,(unsigned char *)Num+0x0D*0x10);
    Display_Char(2,5,(unsigned char *)lkuo);
    Display_Char(2,6,(unsigned char *)Num+0x01*0x10);
    Display_Char(2,7,(unsigned char *)Num+0x03*0x10);
    Display_Char(2,8,(unsigned char *)point);
    Display_Char(2,9,(unsigned char *)Num+0x05*0x10);
    Display_Char(2,10,(unsigned char *)Num+0x06*0x10);
    Display_Char(2,11,(unsigned char *)CM);
    Display_Char(2,12,(unsigned char *)CH);
    Display_Char(2,13,(unsigned char *)Cz);
    Display_Char(2,14,(unsigned char *)rkuo);
    
    Display_Chinese(4,2,(unsigned char *)jiao);      //显示“教学平台”信息              
    Display_Chinese(4,3,(unsigned char *)xue);
    Display_Chinese(4,4,(unsigned char *)ping);
    Display_Chinese(4,5,(unsigned char *)tai2);
    
    for (i = 0; i < 16; i++)                        //显示进度条
    {
        Display_Char(6, i, (unsigned char *)pro);
        delay_ms(100);
    }
}

/******************************************************************************************************************
* 函数名称：Display_Connect()
* 功    能：连机模式下LCM12864模块显示系统信息
* 入口参数：无
* 出口参数：无
* 说    明：显示连机模式信息
******************************************************************************************************************/
void Display_Connect(void)
{

    Display_Chinese(0,2,(unsigned char *)yi2);     //显示“亿道电子”信息
    Display_Chinese(0,3,(unsigned char *)dao);
    Display_Chinese(0,4,(unsigned char *)dian);
    Display_Chinese(0,5,(unsigned char *)zi2);

    Display_Char(2,2,(unsigned char *)CR);          //显示“RFID教学平台”
    Display_Char(2,3,(unsigned char *)Num+0x0F*0x10);
    Display_Char(2,4,(unsigned char *)CI);
    Display_Char(2,5,(unsigned char *)Num+0x0D*0x10);
    Display_Chinese(2,3,(unsigned char *)jiao);              
    Display_Chinese(2,4,(unsigned char *)xue);
    Display_Chinese(2,5,(unsigned char *)ping);
    Display_Chinese(2,6,(unsigned char *)tai2);
    
    Display_Chinese(4,2,(unsigned char *)lian);      //显示“连机模式”信息              
    Display_Chinese(4,3,(unsigned char *)ji);
    Display_Chinese(4,4,(unsigned char *)mo);
    Display_Chinese(4,5,(unsigned char *)shi2);
    
}
/******************************************************************************************************************
* 函数名称：Display_find_tag()
* 功    能：LCM12864模块显示系统启动信息
* 入口参数：pro     ISO协议的设定值
*                   0为ISO15693；1为ISO14443A；2为ISO14443B
* 出口参数：无
* 说    明：显示启动信息
******************************************************************************************************************/
void Display_find_tag(unsigned char pro)
{
    Display_Clear();                                //清屏
    Display_Chinese(0,0,ka);                      //显示“卷标类型：”等信息
    Display_Chinese(0,1,pian);
    Display_Chinese(0,2,lei);
    Display_Chinese(0,3,xing);
    Display_Chinese(0,4,maoh);
    
    switch(pro)
    {
        case 0://ISO15693
        Display_Char(2,2,CI);                       //显示“ISO15693”信息
        Display_Char(2,3,CS);
        Display_Char(2,4,CO);
        Display_Char(2,5,(unsigned char *)Num+0x01*0x10);
        Display_Char(2,6,(unsigned char *)Num+0x05*0x10);
        Display_Char(2,7,(unsigned char *)Num+0x06*0x10);
        Display_Char(2,8,(unsigned char *)Num+0x09*0x10);
        Display_Char(2,9,(unsigned char *)Num+0x03*0x10);
        break;
    case 1:  // ISO14443A
        Display_Char(2,2,CI);                       //显示“ISO14443A”信息
        Display_Char(2,3,CS);
        Display_Char(2,4,CO);
        Display_Char(2,5,(unsigned char *)Num+0x01*0x10);
        Display_Char(2,6,(unsigned char *)Num+0x04*0x10);
        Display_Char(2,7,(unsigned char *)Num+0x04*0x10);
        Display_Char(2,8,(unsigned char *)Num+0x04*0x10);
        Display_Char(2,9,(unsigned char *)Num+0x03*0x10);
        Display_Char(2,10,(unsigned char *)Num+0x0A*0x10);
        break;
    case 2: //ISO14443B
        Display_Char(2,2,CI);                       //显示“ISO14443B”信息
        Display_Char(2,3,CS);
        Display_Char(2,4,CO);
        Display_Char(2,5,(unsigned char *)Num+0x01*0x10);
        Display_Char(2,6,(unsigned char *)Num+0x04*0x10);
        Display_Char(2,7,(unsigned char *)Num+0x04*0x10);
        Display_Char(2,8,(unsigned char *)Num+0x04*0x10);
        Display_Char(2,9,(unsigned char *)Num+0x03*0x10);
        Display_Char(2,10,(unsigned char *)Num+0x0B*0x10);
        break;
    case 3://Tag-it
        Display_Char(2,2,CT);                       //显示“TAGIT”信息
        Display_Char(2,3,(unsigned char *)Num+0x0A*0x10);
        Display_Char(2,4,CG);
        Display_Char(2,5,CI);
        Display_Char(2,6,CT);
        break;
    default:break;      
    }   /* switch */

    Display_Chinese(4,0,ka);                      //显示“卡片UID[PUPI]:”
    Display_Chinese(4,1,pian);
    Display_Char(4,4,CU);
    Display_Char(4,5,CI);
    Display_Char(4,6,(unsigned char *)Num+0x0D*0x10);
    Display_Char(4,7,ZK);
    Display_Char(4,8,CP);
    Display_Char(4,9,CU);
    Display_Char(4,10,CP);
    Display_Char(4,11,CI);
    Display_Char(4,12,YK);
    Display_Chinese(4,7,maoh);
}

/******************************************************************************************************************
* 函数名称：Display_Rssi()
* 功    能：LCM12864模块显示检测到的卷标卡片信号强度图案
* 入口参数：M_rssi    主信道信号强度
* 出口参数：无
* 说    明：显示启动信息
******************************************************************************************************************/
void Display_Rssi(unsigned char M_rssi)
{
    switch(M_rssi)
    {
    case 7:
    case 6: Display_Char(0, 15, ant3);
    case 5:
    case 4: Display_Char(0, 14, ant2);
    case 3:
    case 2: Display_Char(0, 13, ant1);
    case 1:
    case 0: Display_Char(0, 12, ant);
    default:break;
    }
}

/******************************************************************************************************************
* 函数名称：Display_pro()
* 功    能：LCM12864模块显示读写操作时进度条
* 入口参数：无
* 出口参数：无
* 说    明：显示启动信息
******************************************************************************************************************/
void Display_pro(void)
{
    unsigned char i;

    for(i=0;i<16;i++)
    {
        Display_Char(4, i, point);                  //显示....
        delay_ms(10);
    }
    for(i=0;i<8;i++)
    {
        Display_Char(6, i, point);
        delay_ms(10);
    }
}

