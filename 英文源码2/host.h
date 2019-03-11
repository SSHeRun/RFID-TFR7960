//----------------------------------
//Header file with functions for
//host communication.
//It uses the hardware USART
//in the MSP430.
//----------------------------------
#include <MSP430x23x0.h>     	//can't be greater than 256+13
#include <stdio.h>
#include "hardware.h"
#include "globals.h"

//#define BAUD0	0x75	//baud rate generator = 115200 for 13.56 MHz Clock
#define BAUD0	0x3A	//baud rate generator = 115200 for 6.78 MHz Clock
#define BAUD1	0x00
#define BAUD0EN 0x2B	//baud rate for DCO = 4.9MHz
#define BAUD1EN 0x00

void USARTset(void);
void USARTEXTCLKset(void);
void UARTset(void);
void BaudSet(unsigned char mode);
void kputchar(char TXchar);
void put_bksp(void);
void put_space(void);
void put_crlf(void);
void send_cstring(char *pstr);
unsigned char Nibble2Ascii (unsigned char anibble);
void Put_byte(unsigned char abyte);
unsigned char Get_nibble(void);
unsigned char Get_line (unsigned char *pline);
void HostCommands(void);

#pragma vector=USCIAB0RX_VECTOR
__interrupt void RXhandler(void);

#pragma vector = USCIAB0TX_VECTOR
__interrupt void TXhandler (void);
