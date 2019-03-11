//-----------------------------------------------------------
//-----------------------------------------------------------


//#ifndef SPI_BITBANG
//#define SPI_BITBANG


#include <MSP430x23x0.h>     	//can't be greater than 256+13 	
#include <stdio.h>
#include "hardware.h"
#include "globals.h"
#include "host.h"

//NFC and tag emulation settings-----------------------------
#define	NFC106		0x21
#define NFC212		0x22
#define NFC424		0x23

#define TAG14443A	0x24
#define TAG14443B	0x25
#define TAG15693	0x26
#define TAGFelica	0x27

#define TAG106		0x00
#define TAG212		0x21
#define TAG424		0x42
#define TAG848		0x63



//function definitions--------------------------------------
void STARTconditionReader(void);
void STOPconditionReader(void);
void SPIStartCondition(void);
void SPIStopCondition(void);
void InitialSettings(void);
void ReInitialize15693Settings(void);
void ReaderSet(char mode);
void TRFset(unsigned char *pbuf);
void PARset(void);
void STOPcondition(void);
void STOPcont(void);
void STARTcondition(void);
void WriteSingle(unsigned char *pbuf, unsigned char lenght);
void WriteCont(unsigned char *pbuf, unsigned char lenght);
void ReadSingle(unsigned char *pbuf, unsigned char lenght);
void ReadCont(unsigned char *pbuf, unsigned char lenght);
void DirectCommand(unsigned char *pbuf);
void RAWwrite(unsigned char *pbuf, unsigned char lenght);
void DirectMode(void);
void Response(unsigned char *pbuf, unsigned char lenght);
void InterruptHandlerReader(unsigned char *Register);
void InterruptHandlerNFC(unsigned char Register);
void InterruptHandlerNFCtarget(unsigned char Register);

#pragma vector=PORT2_VECTOR
__interrupt void Port_B(void);





//#endif      // SPI_BITBANG