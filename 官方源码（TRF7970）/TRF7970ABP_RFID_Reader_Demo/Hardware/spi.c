/*
 * File Name: spi.c
 *
 * Description: Contains functions to initialize SPI interface using
 * USCI_B0 and communicate with the TRF797x via this interface.
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "spi.h"
#include "trf79xxa.h"

//===============================================================

//===============================================================
// NAME: void SPI_directCommand (uint8_t *pui8Buffer)
//
// BRIEF: Is used in SPI mode to transmit a Direct Command to
// reader chip.
//
// INPUTS:
//	Parameters:
//		uint8_t		*pui8Buffer		Direct Command
//
// OUTPUTS:
//
// PROCESS:	[1] transmit Direct Command
//
// CHANGE:
// DATE  		WHO	DETAIL
// 24Nov2010	RP	Original Code
// 07Dec2010	RP	integrated wait while busy loops
//===============================================================

void 
SPI_directCommand(uint8_t ui8Command)
{	
	SLAVE_SELECT_LOW; 						// Start SPI Mode

	// set Address/Command Word Bit Distribution to command
	ui8Command = (0x80 | ui8Command);					// command
	ui8Command = (0x9f & ui8Command);					// command code

	SPI_sendByte(ui8Command);
#if (TRF79xxA_VERSION == 60)
	SPI_sendByte(0x00);					// Dummy TX Write for Direct Commands per TRF796xA SPI Design Tips (sloa140)
#endif

	SLAVE_SELECT_HIGH; 						//Stop SPI Mode
}

//===============================================================
// NAME: void SPI_directMode (void)
//
// BRIEF: Is used in SPI mode to start Direct Mode.
//
// INPUTS:
//
// OUTPUTS:
//
// PROCESS:	[1] start Direct Mode
//
// NOTE: No stop condition
//
//===============================================================

void
SPI_directMode(void)
{		
	uint8_t pui8Command[2];

	pui8Command[0] = TRF79XXA_CHIP_STATUS_CONTROL;
	pui8Command[1] = TRF79XXA_CHIP_STATUS_CONTROL;
	SPI_readSingle(&pui8Command[1]);
	pui8Command[1] |= 0x60;						// RF on and BIT 6 in Chip Status Control Register set
	SPI_writeSingle(pui8Command);
}  	

//===============================================================
// NAME: void SPI_rawWrite (uint8_t *pui8Buffer, uint8_t length)
//
// BRIEF: Is used in SPI mode to write direct to the reader chip.
//
// INPUTS:
//	Parameters:
//		uint8_t		*pui8Buffer		raw data
//		uint8_t		length		number of data bytes
//
// OUTPUTS:
//
// PROCESS:	[1] send raw data to reader chip
//
//===============================================================

void
SPI_rawWrite(uint8_t * pui8Buffer, uint8_t ui8Length, bool bContinuedSend)
{
	//Start SPI Mode
	SLAVE_SELECT_LOW;

	if (bContinuedSend)
	{
		SPI_sendByte(0x3F);
	}

	while(ui8Length-- > 0)
	{
		// Check if USCI_B0 TX buffer is ready
		while (!(IFG2 & UCB0TXIFG));

		// Transmit data
		UCB0TXBUF = *pui8Buffer;

		while(UCB0STAT & UCBUSY);	// Wait while SPI state machine is busy

		pui8Buffer++;
	}

	// Stop SPI Mode
	SLAVE_SELECT_HIGH;
}
//===============================================================
// NAME: void SPI_readCont (uint8_t *pui8Buffer, uint8_t length)
//
// BRIEF: Is used in SPI mode to read a specified number of
// reader chip registers from a specified address upwards.
//
// INPUTS:
//	Parameters:
//		uint8_t		*pui8Buffer		address of first register
//		uint8_t		length		number of registers
//
// OUTPUTS:
//
// PROCESS:	[1] read registers
//			[2] write contents to *pui8Buffer
//
//===============================================================

void
SPI_readCont(uint8_t * pui8Buffer, uint8_t ui8Length)
{	
	SLAVE_SELECT_LOW; 							//Start SPI Mode

	// Address/Command Word Bit Distribution
	*pui8Buffer = (0x60 | *pui8Buffer); 					// address, read, continuous
	*pui8Buffer = (0x7f &*pui8Buffer);						// register address

	SPI_sendByte(*pui8Buffer);

#if (TRF79xxA_VERSION == 60)
	UCB0CTL1 |= UCSWRST;
	UCB0CTL0 &= ~UCCKPH;					// Switch Clock Polarity for Read (TRF7960A)
	UCB0CTL1 &= ~UCSWRST;
#endif

	while(ui8Length-- > 0)
	{
		*pui8Buffer = SPI_receiveByte();
		pui8Buffer++;
	}
	while(UCB0STAT & UCBUSY)
	{
	}

#if (TRF79xxA_VERSION == 60)
	UCB0CTL0 |= UCCKPH;						// Switch Clock Polarity back
#endif
	SLAVE_SELECT_HIGH; 						// Stop SPI Mode
}

//===============================================================
// NAME: void SPI_readSingle (uint8_t *pui8Buffer)
//
// BRIEF: Is used in SPI mode to read specified reader chip
// registers.
//
// INPUTS:
//	Parameters:
//		uint8_t		*pui8Buffer		addresses of the registers
//
// OUTPUTS:
//
// PROCESS:	[1] read registers
//			[2] write contents to *pui8Buffer
//
//===============================================================

void
SPI_readSingle(uint8_t * pui8Buffer)
{			
	SLAVE_SELECT_LOW; 						// Start SPI Mode

	// Address/Command Word Bit Distribution
	*pui8Buffer = (0x40 | *pui8Buffer); 			// address, read, single
	*pui8Buffer = (0x5f & *pui8Buffer);				// register address


	SPI_sendByte(*pui8Buffer);					// Previous data to TX, RX
#if (TRF79xxA_VERSION == 60)
	UCB0CTL0 &= ~UCCKPH;					// Switch Clock Polarity for Read (TRF7960A)
#endif
	*pui8Buffer = SPI_receiveByte();
#if (TRF79xxA_VERSION == 60)
	UCB0CTL0 |= UCCKPH;						// Switch Clock Polarity back
#endif

	SLAVE_SELECT_HIGH; 						// Stop SPI Mode
}

//===============================================================
// NAME: uint8_t SPI_receiveByte(void)
//===============================================================

uint8_t SPI_receiveByte(void)
{
	UCB0TXBUF = 0x00;

	while (UCB0STAT & UCBUSY);

	return UCB0RXBUF;
}

//===============================================================
// NAME: void SPI_sendByte(uint8_t ui8TxByte)
//===============================================================

void SPI_sendByte(uint8_t ui8TxByte)
{
	UCB0TXBUF = ui8TxByte;

	while (UCB0STAT & UCBUSY);
}

//===============================================================
// NAME: void SPI_usciSet (void)
//
// BRIEF: Is used to set USCI B0 for SPI communication
//
// INPUTS:
//
// OUTPUTS:
//
// PROCESS:	[1] make settings
//
// CHANGE:
// DATE  		WHO	DETAIL
// 24Nov2010	RP	Original Code
// 07Dec2010	RP	reduced SPI clock frequency
//===============================================================

void
SPI_usciSet(void)								//Uses USCI_B0
{
	UCB0CTL1 |= UCSWRST;						// Enable SW reset
	UCB0CTL0 |= UCMSB + UCMST + UCSYNC;			// 3-pin, 8-bit SPI master
#if (TRF79xxA_VERSION == 60)
	UCB0CTL0 |= UCCKPH;
#endif
	UCB0CTL1 |= UCSSEL_2;						// Source from SMCLK

	UCB0BR0 = 0x04;
	UCB0BR1 = 0;
	P1SEL |= BIT5 + BIT6 + BIT7;				// P1.5,1.6,1.7 UCBOCLK,UCB0SIMO,UCB0SOMI, option select
	P1SEL2 |= BIT5 + BIT6 + BIT7;				// P1.5,1.6,1.7 UCBOCLK,UCB0SIMO,UCB0SOMI, option select

	SLAVE_SELECT_PORT_SET;						// Set the Slave Select Port
	SLAVE_SELECT_HIGH;							// Slave Select => inactive (high)

	UCB0CTL1 &= ~UCSWRST;						// **Initialize USCI state machine**
}


//===============================================================
// NAME: void SPI_writeCont (uint8_t *pui8Buffer, uint8_t length)
//
// BRIEF: Is used in SPI mode to write to a specific number of
// reader chip registers from a specific address upwards.
//
// INPUTS:
//	uint8_t	*pui8Buffer	address of first register followed by the
//					contents to write
//	uint8_t	length	number of registers + 1
//
// OUTPUTS:
//
// PROCESS:	[1] write to the registers
//
// CHANGE:
// DATE  		WHO	DETAIL
// 24Nov2010	RP	Original Code
// 07Dec2010	RP	integrated wait while busy loops
//===============================================================

void
SPI_writeCont(uint8_t * pui8Buffer, uint8_t ui8Length)
{	
	SLAVE_SELECT_LOW; 						// Start SPI Mode

	// Address/Command Wort Bit Distribution
	*pui8Buffer = (0x20 | *pui8Buffer); 				// address, write, continuous
	*pui8Buffer = (0x3f & *pui8Buffer);					// register address

	while(ui8Length-- > 0)
	{	
		SPI_sendByte(*pui8Buffer++);
	}

	SLAVE_SELECT_HIGH; 						// Stop SPI Mode
}

//===============================================================
// NAME: void SPI_writeSingle (uint8_t *pui8Buffer, uint8_t length)
//
// BRIEF: Is used in SPI mode to write to a specified reader chip
// registers.
//
// INPUTS:
//	uint8_t	*pui8Buffer	addresses of the registers followed by the
//					contends to write
//	uint8_t	length	number of registers * 2
//
// OUTPUTS:
//
// PROCESS:	[1] write to the registers
//
// CHANGE:
// DATE  		WHO	DETAIL
// 24Nov2010	RP	Original Code
// 07Dec2010	RP	integrated wait while busy loops
//===============================================================

void
SPI_writeSingle(uint8_t * pui8Buffer)
{
	SLAVE_SELECT_LOW; 						// Start SPI Mode

	// Address/Command Word Bit Distribution
	// address, write, single (fist 3 bits = 0)
	*pui8Buffer = (0x1f & *pui8Buffer);				// register address


	SPI_sendByte(*pui8Buffer++);
	SPI_sendByte(*pui8Buffer++);

	SLAVE_SELECT_HIGH; 						// Stop SPI Mode
}

//===============================================================
// Settings for SPI Mode                                        ;
// 02DEC2010	RP	Original Code
//===============================================================

void
SPI_setup(void)
{
	TRF_ENABLE_SET;

	IRQ_PIN_SET;
	IRQ_EDGE_SET;								// rising edge interrupt

	SPI_usciSet();								// Set the USART

	LED_ALL_OFF;
	LED_PORT_SET;
}
