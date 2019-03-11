//----------------------------------------------//
//Here are the processor specific functions.	//
//						//
//----------------------------------------------//
#include <MSP430x23x0.h>     		
#include <stdio.h>
#include "globals.h"


/*#ifndef HIGHSPEED
#define HIGHSPEED
#endif*/

//========MCU constants===================================

#define TRFWrite 	P4OUT		//port4 is connected to the
#define TRFRead  	P4IN		//TRF796x IO port.
#define TRFDirIN	P4DIR = 0x00
#define TRFDirOUT	P4DIR = 0xFF
#define TRFSerial	P4DIR = 0x9F  //Note sure !!!! - Harsha
#define TRFFunc		P4SEL = 0x00

#define EnableSet	P1DIR |= BIT0
#define	TRFEnable	P1OUT |= BIT0		//EN pin on the TRF796x
#define TRFDisable	P1OUT &= ~BIT0

//PIN operations---------------------------------------------
#define clkPOUTset 	P3DIR |= BIT3		//DATA_CLK on P3.3 (P3.3/UCB0CLK used in GPIO Mode for Parallel operation)
#define clkON		P3OUT |= BIT3
#define clkOFF		P3OUT &= ~BIT3

#define SIMOSet         P3DIR |= BIT1;          //SIMO on P3.1
#define SIMOON          P3OUT |= BIT1
#define SIMOOFF         P3OUT &= ~BIT1
#define SOMISIGNAL      P3IN & BIT2


#define irqPINset	P2DIR &= ~BIT1;
#define irqPIN		BIT1
#define irqPORT		P2IN
#define irqON		P2IE |= BIT1		//IRQ on P2.1
#define irqOFF		P2IE &= ~BIT1		//IRQ on P2.1
#define irqEDGEset	P2IES &= ~BIT1		//rising edge interrupt
#define irqCLR		P2IFG = 0x00
#define irqREQreg	P2IFG

#define LEDportSET	P1DIR |= 0xF8;
#define LEDallOFF	P1OUT &= ~0xF8;
#define LEDallON	P1OUT |= 0xF8;
#define LEDpowerON	P1OUT |= BIT7;
#define LEDpowerOFF	P1OUT &= ~BIT7;
#define LEDtypeAON	P1OUT |= BIT6;
#define LEDtypeAOFF	P1OUT &= ~BIT6;
#define LEDtypeBON	P1OUT |= BIT5;
#define LEDtypeBOFF	P1OUT &= ~BIT5;
#define LED15693ON	P1OUT |= BIT4;
#define LED15693OFF	P1OUT &= ~BIT4;
#define LEDtagitON	P1OUT |= BIT3;
#define LEDtagitOFF	P1OUT &= ~BIT3;

#define SPIMODE             P2IN & BIT3   //This is set to Vcc for SPI mode and GND for Parallel Mode using a separate jumper
#define SlaveSelectPortSet  P3DIR |= BIT0;
#define SlaveSelectHIGH       P3OUT |= BIT0;
#define SlaveSelectLOW      P3OUT &= ~BIT0;

#define OOKdirIN	P2DIR &= ~BIT2;
#define OOKdirOUT	P2DIR |= BIT2
#define OOKoff		P2OUT &= ~BIT2
#define OOKon		P2OUT |= BIT2
//Counter-timer constants------------------------------------
#define countValue	TACCR0			//counter register
//#define startCounter	TACTL |= MC0 + MC1	//start counter in up/down mode
#define startCounter	TACTL |=  MC1	//start counter in up mode

#define stopCounter	TACTL &= ~(MC0 + MC1)	//stops the counter

#ifndef HIGHSPEED
  #define count1ms	847
#else
  #define count1ms	1694
#endif

//=======TRF definitions=========================
//Reader addresses
#define ChipStateControl	0x00
#define ISOControl		0x01
#define ISO14443Boptions	0x02
#define ISO14443Aoptions	0x03
#define TXtimerEPChigh		0x04
#define TXtimerEPClow		0x05
#define TXPulseLenghtControl	0x06
#define RXNoResponseWaitTime	0x07
#define RXWaitTime		0x08
#define ModulatorControl	0x09
#define RXSpecialSettings	0x0A
#define RegulatorControl	0x0B
#define IRQStatus		0x0C
#define IRQMask			0x0D
#define	CollisionPosition	0x0E
#define RSSILevels		0x0F
#define RAMStartAddress		0x10	//RAM is 7 bytes long (0x10 - 0x16)
#define NFCID			0x17
#define NFCTargetLevel		0x18
#define NFCTargetProtocol	0x19
#define TestSetting1		0x1A
#define TestSetting2		0x1B
#define FIFOStatus		0x1C
#define TXLenghtByte1		0x1D
#define TXLenghtByte2		0x1E
#define FIFO			0x1F

//Reader commands-------------------------------------------
#define Idle			0x00
#define SoftInit		0x03
#define InitialRFCollision	0x04
#define ResponseRFCollisionN	0x05
#define ResponseRFCollision0	0x06
#define	Reset			0x0F
#define TransmitNoCRC		0x10
#define TransmitCRC		0x11
#define DelayTransmitNoCRC	0x12
#define DelayTransmitCRC	0x13
#define TransmitNextSlot	0x14
#define CloseSlotSequence	0x15
#define StopDecoders		0x16
#define RunDecoders		0x17
#define ChectInternalRF		0x18
#define CheckExternalRF		0x19
#define AdjustGain		0x1A
//==========================================================


void delay_ms(unsigned int n_ms);
void CounterSet(void);
void OSCsel(unsigned char mode);

#pragma vector=TIMERA0_VECTOR
__interrupt void TimerAhandler(void);
