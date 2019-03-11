/**********************************************************************************************************************************************************
* �� �� ����LCD.C
* ��    �ܣ�HO12864FPD-14CSBE��3S��λͼ��Һ��ģ�飨ST7565P��������
* Ӳ�����ӣ�HO12864FPD-14CSBE��3S��λͼ��Һ��ģ�飨ST7565P����MSP430F2370��Ӳ�����ӹ�ϵ������ʾ��
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
* ��    �ߣ�EMDOOR
* ��    �ڣ�2011��04��13��
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
* �������ƣ�Delay_lcm()
* ��    �ܣ������ʱ
* ��ڲ�����count    ��ʱ������ֵԽ����ʱԽ��
* ���ڲ�������
* ˵    ������ʱ�ȴ��á�
******************************************************************************************************************/
void Delay_lcm(unsigned int count)
{
    unsigned char i;
  
    for(; count > 0; count--)
        for(i = 0; i < 15; i++);
}
 
/******************************************************************************************************************
* �������ƣ�Data_Send()
* ��    �ܣ�LCD����ģʽ��������
* ��ڲ�����data    Ҫ���͵�����
* ���ڲ�������
* ˵    ����MSP430΢��������LCD��������������
******************************************************************************************************************/
void Data_Send(unsigned char data)
{
    unsigned char s, tmp, i;                        //�������
    
    L_LCM_SCL();                                    //����SCL������
    s = data;                                       //�������ݴ������s
  
    for(i = 8; i > 0; i--)                          //����ģʽ�£���Ҫѭ��8�Σ����ܽ����ݴ��ͳ�ȥ
    {
        L_LCM_SCL();                                //����SCL������
        Delay_lcm(2);
        tmp = s & 0x80;                             //ȡ����s�����λB7
      
        if(tmp)                                     //�жϸߵ͵�ƽ��������
            H_LCM_SID();
        else
            L_LCM_SID();
        H_LCM_SCL();                                //����SCL������
        
        s = s << 1;                                 //����һλ����������
    }             
}

/******************************************************************************************************************
* �������ƣ�Write_CMD()
* ��    �ܣ�LCD����ģʽ��������
* ��ڲ�����command    ����ֵ
* ���ڲ�������
* ˵    ����MSP430΢��������LCD��������������
******************************************************************************************************************/
void Write_CMD(unsigned char command)
{
    L_LCM_A0();                                     //����A0������
    L_LCM_nCS();                                    //����CSƬѡ�ź���
    
    Data_Send(command);                             //��������ֵ����
}

/******************************************************************************************************************
* �������ƣ�Write_Data()
* ��    �ܣ�д����
* ��ڲ�����data    ����
* ���ڲ�������
* ˵    ����MSP430΢��������LCD��������������
******************************************************************************************************************/
void Write_Data(unsigned char data)
{
    H_LCM_A0();                                     //����A0������
    L_LCM_nCS();                                    //����CSƬѡ�ź���
    
    Data_Send(data);                                //��������ֵ����            
}

/******************************************************************************************************************
* �������ƣ�Display_Clear()
* ��    �ܣ����LCD��ʾ
* ��ڲ�������
* ���ڲ�������
* ˵    ����ִ�иú��������������Ļ�κ���ʾ����
******************************************************************************************************************/
void Display_Clear(void)
{
    unsigned char seg;                              //����LCD��ʾ��
    unsigned char page;                             //����LCD��ʾҳ

    for(page = 0xb0; page < 0xb9; page++)           //ҳ��ַ��8ҳ��0xb0 - 0xb8
    {
        Write_CMD(page);                            //����ҳ��ַ
        Write_CMD(0x10);                            //�����е�ַ���ߵ��ֽ�����д�룬�ӵ�0�п�ʼ
        Write_CMD(0x00);
        
        for(seg = 0; seg < 132; seg++)              //д128�У����ڱ�Һ��Ϊ����װ���䷶ΧΪ0-131
            Write_Data(0x00);
    }
}

/******************************************************************************************************************
* �������ƣ�Display_Picture()
* ��    �ܣ���ʾһ��128*64����ͼƬ
* ��ڲ�����*p       ͼƬ���ݵ��׵�ַָ��
* ���ڲ�������
* ˵    ����ִ�иú�����������ָ������ʾ��ͼƬ�׵�ַ��
******************************************************************************************************************/
void Display_Picture(unsigned char *p)
{
    unsigned char seg;                              //����LCD��ʾ��
    unsigned char page;                             //����LCD��ʾҳ

    for(page = 0xb0; page < 0xb9; page++)           //ҳ��ַ��8ҳ��0xb0 - 0xb8
    {
        Write_CMD(page);                            //����ҳ��ַ
        Write_CMD(0x10);                            //�����е�ַ���ߵ��ֽ�����д�룬�ӵ�0�п�ʼ
        Write_CMD(0x00);
        for(seg = 0; seg < 128; seg++)              //д128������
            Write_Data(*p++);
    }
}

/******************************************************************************************************************
* �������ƣ�Display_Chinese()
* ��    �ܣ���ָ����λ���ϣ���ʾһ������
* ��ڲ�����pag       ��ʾ���ֵ�ҳ��ַ
*           col       ��ʾ���ֵ��е�ַ
*           *hzk      ��ʾ���ֵĵ�ַָ��
* ���ڲ�������
* ˵    �������ú����������ɹ�����������������������
******************************************************************************************************************/
void Display_Chinese(unsigned char pag, unsigned char col, unsigned char const *hzk)
{
    unsigned char j = 0, i = 0;
    unsigned char page; 
    unsigned char row;
   
    /* �������� */
    /*===========================================================================*/
    if(pag > 7)return;                              // pag����Խ�磬����
    if(col > 7)return;                              // col����Խ�磬����
    /*===========================================================================*/  
    
    row = col * 16 + 4;                             //���¼���õ���ʾ�е�ַ������LCD������װ������ +4 
    page = 0xb0 + pag;                              //�õ�ҳ��ַ
    
    for(j = 0; j < 2; j++)                          //��2ҳ����д��                     
    {
        Write_CMD(page + j);                        //��j=0ʱ��Ϊ��1ҳ����j=1ʱ����2ҳ
        Write_CMD(0x10 + ((row >> 4) & 0x0f));      //�����е�ַ
        Write_CMD(row & 0x0f);                      //�ߵ��ֽ�����д��  
        
        for(i = 0; i < 16; i++) 
            Write_Data(hzk[16 * j + i]);            //д���������
    }
}

/******************************************************************************************************************
* �������ƣ�Display_Char()
* ��    �ܣ���ָ����λ���ϣ���ʾһ�����ֻ�����ĸ
* ��ڲ�����pag       ��ʾ���ֵ�ҳ��ַ
*           col       ��ʾ���ֵ��е�ַ
*           *hzk      ��ʾ���ֵĵ�ַָ��
* ���ڲ�������
* ˵    �������ú����������ɹ�����������������������
******************************************************************************************************************/
void Display_Char(unsigned char pag,unsigned char col, unsigned char const *hzk)
{
    unsigned char j = 0, i = 0;
    unsigned char page; 
    unsigned char row;
  
    /* �������� */
    /*===========================================================================*/
    if(pag > 7)return;                              // pag����Խ�磬����
    if(col > 15)return;                             // col����Խ�磬����
    /*===========================================================================*/  
    
    row = col * 8 + 4;                              //���¼���õ���ʾ�е�ַ������LCD������װ������ +4 
    page = 0xb0 + pag;                              //�õ�ҳ��ַ
    
    for(j = 0; j < 2; j++)                          //��2ҳ����д��   
    {
        Write_CMD(page + j);                        //��j=0ʱ��Ϊ��1ҳ����j=1ʱ����2ҳ
        Write_CMD(0x10 + ((row >> 4) & 0x0f));      //�����е�ַ
        Write_CMD(row & 0x0f);                      //�ߵ��ֽ�����д��  
        
        for(i = 0; i < 8; i++)                      //д���������
            Write_Data(hzk[8 * j + i]);
    }
}

/******************************************************************************************************************
* �������ƣ�LCM12864_Init()
* ��    �ܣ�LCM12864ģ���ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵    ������ʼ��LCD��ʾ��
******************************************************************************************************************/
void LCM12864_Init(void)
{ 
    /* LCM���ƹܽŷ������� */
    /*===========================================================================*/
    LCMA0Set();                                     //A0�������
    LCMnCSSet();                                    //CS�������
    LCMnRSTSet();                                   //RST��λ���
    /*===========================================================================*/
    
    L_LCM_nRST();
    Delay_lcm(2);
    H_LCM_nRST();                                   //LCM��λ
  
    Write_CMD(0xaf);                                //AF��ʾ������AE��ʾ����
    Write_CMD(0x40);                                //0x40��ָ����ʾRAM ����ʼ�е�ַ
    //Write_CMD(0xa0);                              //A0 ģ������װ���е�ַ������Ϊ0~127
    Write_CMD(0xa1);                                //A1 ģ�鷴��װ���е�ַ������Ϊ131~4
    Write_CMD(0xa6);                                //A6������ʾ��A7������ʾ
    Write_CMD(0xa4);                                //A4ȫ����ʾ�����ܸı�RAM���ݣ�A5���Ըı�
    Write_CMD(0xa2);                                //����LCD ��ƫѹ��1/9
    //Write_CMD(0xc8);                              //ģ������װʱ,�϶�Ϊ��0��
    Write_CMD(0xc0);                                //ģ�鷴��װʱ,�¶�Ϊ��0��
    Write_CMD(0x2f);                                //���ÿ����ڲ���·�ĵ�Դ
    Write_CMD(0x24);                                //24��81��24�ֵ��Աȶ�
    Write_CMD(0x81);
    Write_CMD(0x24);

    Write_CMD(0xac);                                //static indicator off  
    Write_CMD(0x00);
}

/******************************************************************************************************************
* �������ƣ�Display_Put_Tag()
* ��    �ܣ�LCM12864ģ����ʾ����뿨Ƭ��Ϣ
* ��ڲ�������
* ���ڲ�������
* ˵    ������ʾ��ʾ��Ϣ
******************************************************************************************************************/
void Display_Put_Tag(void)
{
    Display_Clear();                                //����
    
    Display_Chinese(3,1,qing);                      //��ʾ������뿨Ƭ����Ϣ
    Display_Chinese(3,2,fang);
    Display_Chinese(3,3,ru);
    Display_Chinese(3,4,ka);
    Display_Chinese(3,5,pian);
    Display_Char(3,12,point);
    Display_Char(3,13,point);
    Display_Char(3,14,point);
}

/******************************************************************************************************************
* �������ƣ�Display_Start()
* ��    �ܣ�LCM12864ģ����ʾϵͳ������Ϣ
* ��ڲ�������
* ���ڲ�������
* ˵    ������ʾ������Ϣ
******************************************************************************************************************/
void Display_Start(void)
{
    unsigned char i;

    Display_Chinese(0,2,(unsigned char *)huan);     //��ʾ����ӭʹ�á���Ϣ
    Display_Chinese(0,3,(unsigned char *)ying);
    Display_Chinese(0,4,(unsigned char *)shi);
    Display_Chinese(0,5,(unsigned char *)yong);

    Display_Char(2,1,(unsigned char *)CR);          //��ʾ��RFID(13.56MHz)��
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
    
    Display_Chinese(4,2,(unsigned char *)jiao);      //��ʾ����ѧƽ̨����Ϣ              
    Display_Chinese(4,3,(unsigned char *)xue);
    Display_Chinese(4,4,(unsigned char *)ping);
    Display_Chinese(4,5,(unsigned char *)tai2);
    
    for (i = 0; i < 16; i++)                        //��ʾ������
    {
        Display_Char(6, i, (unsigned char *)pro);
        delay_ms(100);
    }
}

/******************************************************************************************************************
* �������ƣ�Display_Connect()
* ��    �ܣ�����ģʽ��LCM12864ģ����ʾϵͳ��Ϣ
* ��ڲ�������
* ���ڲ�������
* ˵    ������ʾ����ģʽ��Ϣ
******************************************************************************************************************/
void Display_Connect(void)
{

    Display_Chinese(0,2,(unsigned char *)yi2);     //��ʾ���ڵ����ӡ���Ϣ
    Display_Chinese(0,3,(unsigned char *)dao);
    Display_Chinese(0,4,(unsigned char *)dian);
    Display_Chinese(0,5,(unsigned char *)zi2);

    Display_Char(2,2,(unsigned char *)CR);          //��ʾ��RFID��ѧƽ̨��
    Display_Char(2,3,(unsigned char *)Num+0x0F*0x10);
    Display_Char(2,4,(unsigned char *)CI);
    Display_Char(2,5,(unsigned char *)Num+0x0D*0x10);
    Display_Chinese(2,3,(unsigned char *)jiao);              
    Display_Chinese(2,4,(unsigned char *)xue);
    Display_Chinese(2,5,(unsigned char *)ping);
    Display_Chinese(2,6,(unsigned char *)tai2);
    
    Display_Chinese(4,2,(unsigned char *)lian);      //��ʾ������ģʽ����Ϣ              
    Display_Chinese(4,3,(unsigned char *)ji);
    Display_Chinese(4,4,(unsigned char *)mo);
    Display_Chinese(4,5,(unsigned char *)shi2);
    
}
/******************************************************************************************************************
* �������ƣ�Display_find_tag()
* ��    �ܣ�LCM12864ģ����ʾϵͳ������Ϣ
* ��ڲ�����pro     ISOЭ����趨ֵ
*                   0ΪISO15693��1ΪISO14443A��2ΪISO14443B
* ���ڲ�������
* ˵    ������ʾ������Ϣ
******************************************************************************************************************/
void Display_find_tag(unsigned char pro)
{
    Display_Clear();                                //����
    Display_Chinese(0,0,ka);                      //��ʾ��������ͣ�������Ϣ
    Display_Chinese(0,1,pian);
    Display_Chinese(0,2,lei);
    Display_Chinese(0,3,xing);
    Display_Chinese(0,4,maoh);
    
    switch(pro)
    {
        case 0://ISO15693
        Display_Char(2,2,CI);                       //��ʾ��ISO15693����Ϣ
        Display_Char(2,3,CS);
        Display_Char(2,4,CO);
        Display_Char(2,5,(unsigned char *)Num+0x01*0x10);
        Display_Char(2,6,(unsigned char *)Num+0x05*0x10);
        Display_Char(2,7,(unsigned char *)Num+0x06*0x10);
        Display_Char(2,8,(unsigned char *)Num+0x09*0x10);
        Display_Char(2,9,(unsigned char *)Num+0x03*0x10);
        break;
    case 1:  // ISO14443A
        Display_Char(2,2,CI);                       //��ʾ��ISO14443A����Ϣ
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
        Display_Char(2,2,CI);                       //��ʾ��ISO14443B����Ϣ
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
        Display_Char(2,2,CT);                       //��ʾ��TAGIT����Ϣ
        Display_Char(2,3,(unsigned char *)Num+0x0A*0x10);
        Display_Char(2,4,CG);
        Display_Char(2,5,CI);
        Display_Char(2,6,CT);
        break;
    default:break;      
    }   /* switch */

    Display_Chinese(4,0,ka);                      //��ʾ����ƬUID[PUPI]:��
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
* �������ƣ�Display_Rssi()
* ��    �ܣ�LCM12864ģ����ʾ��⵽�ľ�꿨Ƭ�ź�ǿ��ͼ��
* ��ڲ�����M_rssi    ���ŵ��ź�ǿ��
* ���ڲ�������
* ˵    ������ʾ������Ϣ
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
* �������ƣ�Display_pro()
* ��    �ܣ�LCM12864ģ����ʾ��д����ʱ������
* ��ڲ�������
* ���ڲ�������
* ˵    ������ʾ������Ϣ
******************************************************************************************************************/
void Display_pro(void)
{
    unsigned char i;

    for(i=0;i<16;i++)
    {
        Display_Char(4, i, point);                  //��ʾ....
        delay_ms(10);
    }
    for(i=0;i<8;i++)
    {
        Display_Char(6, i, point);
        delay_ms(10);
    }
}

