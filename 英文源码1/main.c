#include "msp430x12x.h"
#include "TRF7960.h"
/****************************************************************
*  函数名：Intial_Mcu										
*  功能说明：MCU初始化程序		
*  入口参数：N/A											
*  出口参数：											
*  时间:  	2004.12.09											
*  作者:  		wang											
****************************************************************/

void Intial_Mcu(void)
{ 
      char i;
      WDTCTL = WDTPW + WDTHOLD;             // Stop WDT
//osc set
      BCSCTL1 |= XTS;                       		// ACLK = LFXT1 = HF XTAL
	  do 
	  {
		  IFG1 &= ~OFIFG;                       	// Clear OSCFault flag
		  for (i = 0xFF; i > 0; i--);           	// Time for flag to set
	  }
	  while ((IFG1 & OFIFG) != 0);          		// OSCFault flag still set?                

	  BCSCTL2 |= SELM_3;                    		// MCLK = LFXT1 (safe)
//time set
	
	  //CCTL0 = CCIE;                         		// CCR0 interrupt enabled
	  //CCR0 = 0x1000;//10ms
	  //TACTL = TASSEL_2+ MC_1;             	 // SMCLK, contmode
	
//uart set 	  
	  //P3SEL |= 0x30;                        		// P3.4,5 = USART0 TXD/RXD
	  //ME2 |= UTXE0 + URXE0;                 		// Enabled USART0 TXD/RXD
	/*  UCTL0 |= CHAR;                        		// 8-bit character
	  UTCTL0 |= SSEL0;                      		// UCLK = ACLK
	  //UBR00 =0xe8;		              		 // 7.37Mhz/9600 - 372
	  //UBR10 =0x02;    
	  //change for small version of VRF7960   		//
	  UBR00 = 0xC2;                // 6.78Mhz/9600 =706.25
	  UBR10 = 0x02;                //
	  U0MCTL= 0x20;                //8/0.25
	  
	  UMCTL0 = 0x00;                        		// no modulation
	  UCTL0 &= ~SWRST;             */         		// Initalize USART state machine
	  //IE2 |= URXIE0+UTXIE0;                  		// Enabled USART0 RX interrupt
//io set
	// P1DIR |= 0x04;                        		// P1.2 output
}

int main( void )
{ char i;
  //char a[30];
  for(i=0;i<12;i++)
  {
     buffer[i]=14+i;
     
  }

  WDTCTL = WDTPW + WDTHOLD;             // Stop WDT
  initial_7960();
  Intial_Mcu();
  while(1)
  {
    _NOP();
    i=Request(0x26);
    _NOP();
   i=AntiColl();
  _NOP();
  }
  return 0;
}
