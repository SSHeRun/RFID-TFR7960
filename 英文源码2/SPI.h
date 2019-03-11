//-----------------------------------------------------------
//-----------------------------------------------------------

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

void SPIWriteSingle(unsigned char *pbuf, unsigned char lenght);
void SPIWriteCont(unsigned char *pbuf, unsigned char lenght);
void SPIReadSingle(unsigned char *pbuf, unsigned char lenght);
void SPIReadCont(unsigned char *pbuf, unsigned char lenght);
void SPIDirectCommand(unsigned char *pbuf);
void SPIRAWwrite(unsigned char *pbuf, unsigned char lenght);
void SPIDirectMode(void);
void SERset(void);
/*void USARTset(void);*/


