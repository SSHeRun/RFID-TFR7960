/*
 * File Name: uart.c
 *
 * Description: Contains functions to initialize UART interface using
 * USCI_A0 and communicate with the host via this interface.
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

#include "uart.h"

//===============================================================

#ifdef ENABLE_HOST

//===============================================================
//
// UART_nibble2Ascii - Converts a nibble to ASCII Hex Byte
//
// \param ui8AsciiNibble is the nibble for conversion
//
// This function will take any inputted nibble and turn it into
// an Ascii character.
//
// \return ui8AsciiOutput returns the resulting Ascii character
// after the conversion
//
//===============================================================

uint8_t
UART_nibble2Ascii(uint8_t ui8AsciiNibble)
{
	uint8_t	ui8AsciiOutput = ui8AsciiNibble;

	if(ui8AsciiNibble > 9)							// If req ASCII A-F then add 7(hex)
	{	
		ui8AsciiOutput = ui8AsciiOutput + 0x07;
	}

	// Add offset to convert to Ascii
	ui8AsciiOutput = ui8AsciiOutput + 0x30;

	return ui8AsciiOutput;
}

//===============================================================
//
// UART_putByte - Output data to UART in Ascii form
//
// \param ui8TxByte is the byte to convert and output
//
// Output data bytes to the UART in Ascii form
//
// \return None.
//
//===============================================================

void
UART_putByte(uint8_t ui8TxByte)
{
	uint8_t	ui8TempVar1 = 0, ui8TempVar2 = 0;

	ui8TempVar1 = (ui8TxByte >> 4) & 0x0F;			// get high nibble
	ui8TempVar2 = UART_nibble2Ascii(ui8TempVar1);		// convert to ASCII
	UART_putChar(ui8TempVar2);						// output */

	ui8TempVar1 = ui8TxByte & 0x0F;					// get low nibble
	ui8TempVar2 = UART_nibble2Ascii(ui8TempVar1);		// convert to ASCII
	UART_putChar(ui8TempVar2);						// output
}

//===============================================================
//
// UART_putChar - Output raw hex data to UART
//
// \param uint8_t ui8TxByte is the byte of data to be outputted
//
// Output raw hexadecimal data bytes to the UART
//
// \return None.
//
//===============================================================

void
UART_putChar(uint8_t ui8TxChar)
{
	while(!(IFG2 & UCA0TXIFG))				// till UART Transmit buffer is empty
	{
	}

	UCA0TXBUF = ui8TxChar;						// send the character
}

//===============================================================
//
// UART_putNewLine - Make a new line on UART output
//
// Used to create new lines between strings of UART data
//
// \return None.
//
//===============================================================

void
UART_putNewLine(void)
{
	UART_putChar('\r');
	UART_putChar('\n');
}

//===============================================================
//
// UART_putSpace - Output a space character to UART
//
// Places a space character (0x20) between strings of UART data
//
// \return None.
//
//===============================================================

void
UART_putSpace(void)
{
	UART_putChar(' ');
}

//===============================================================
//
// UART_response - Output characters to UART with frames
//
// \param pui8Buffer is the buffer which contains the message
// content
// \param ui8Length is the length of message
//
// Output bytes to UART with [ and ] framing them
//
// \return None.
//
//===============================================================

void
UART_response(uint8_t * pui8Buffer, uint8_t ui8Length)
{
	while(ui8Length > 0)
	{	
		UART_putChar('[');
		UART_putByte(*pui8Buffer);
		UART_putChar(']');
		pui8Buffer++;
		ui8Length--;
	}
	UART_putNewLine();
}

//===============================================================
//
// UART_putBuffer - Output characters to UART
//
// \param pui8Buffer is the buffer which contains the message
// content
// \param ui8Length is the length of message
//
// Outputs an inputted buffer to UART based on the length of the
// message
//
// \return None.
//
//===============================================================

void
UART_putBuffer(const uint8_t * pui8Buffer, uint8_t ui8Length)
{
	while(ui8Length > 0)
	{
		UART_putChar(*pui8Buffer);
		pui8Buffer++;
		ui8Length--;
	}
}

//===============================================================
//
// UART_putBufferAscii - Output characters to UART in Ascii
//
// \param pui8Buffer is the buffer which contains the message
// content
// \param ui8Length is the length of message
//
// Outputs an inputted buffer to UART based on the length of the
// message. Each byte will be converted to ASCII prior to output.
//
// \return None.
//
//===============================================================

void
UART_putBufferAscii(const uint8_t * pui8Buffer, uint8_t ui8Length)
{
	while(ui8Length > 0)
	{
		UART_putByte(*pui8Buffer);
		pui8Buffer++;
		ui8Length--;
	}
}

//===============================================================
//
// UART_sendCString - Output character string to UART, ending at
// the terminator character '\0'
//
// \param pui8Buffer is the buffer which contains the message
// content
//
// Outputs an inputted character string straight to UART. This
// function does not convert the buffer to ASCII. No length
// is required since it searches for terminator character
// instead.
//
// \return None.
//
//===============================================================

void
UART_sendCString(uint8_t * pui8Buffer)
{
	while(*pui8Buffer != '\0')
	{	
		UART_putChar(*pui8Buffer++);
	}
}

//===============================================================
//
// UART_setup - Initialize UART
//
// Function will initialize UART GPIO's and registers for
// MSP430G2553 Microcontroller that runs at 8 MHz.
//
// \return None.
//
//===============================================================

void
UART_setup(void)							// uses USCI_A0
{
    // modified for using the ez430-RF256x on the -G2553 LaunchPad
    P1SEL |= BIT2;						// P1.2=TXD
    // this is for TX only... (short TX & RX on the board P1.1 to P1.2)
    P1SEL2 |= BIT2;						// P1.2=TXD

    UCA0CTL1 |= UCSWRST;				// disable UART

    UCA0CTL0 = 0x00;

    UCA0CTL1 |= UCSSEL_2 + UCSWRST;		// Source from SMCLK, keep SW Reset

    UCA0BR0 = 0x41;						// Baud Rate = 9600
    UCA0BR1 = 0x03;

    UCA0MCTL = UCBRS_2;					// Modulation UCBRSx = 2

    UCA0CTL1 &= ~UCSWRST;				// Initialize USCI state machine
}

//===============================================================
//
// UART_putIntroReaderMsg - Introduction message for UART terminal
//
// \param pui8VersionNumber is the pointer to current software
// version number
// \param pui8VersionDate is the pointer to current software
// version date
//
// Introduction message when connecting to UART terminal
// Includes version number/date to help users identify if they are
// using the latest firmware version.
//
// \return None.
//
//===============================================================

void UART_putIntroReaderMsg(uint8_t * pui8VersionNumber, uint8_t * pui8VersionDate)
{
	UART_putNewLine();
	UART_sendCString("[*] TRF7970A NFC/RFID Tag Reader Enabled [*]"); 	// Board is alive and UART is up
	UART_putNewLine();
	UART_sendCString("     *  Firmware Version: ");
	UART_sendCString(pui8VersionNumber);
	UART_putNewLine();
	UART_sendCString("     *  Last Updated: ");
	UART_sendCString(pui8VersionDate);
	UART_putNewLine();
	UART_sendCString("[!] Disclaimer: This example project is meant to be used for demonstration purposes only. [!]");
	UART_putNewLine();
	UART_putNewLine();
}

//===============================================================
//
// UART_putByteDecimalValue - Prints out decimal version of uint8
// hex values.
//
// \param ui8HexByte is the uint8 hexadecimal value to print out.
//
// Function takes the inputted uint8 hexadecimal value, converts
// it, and prints the result out over UART in order to display
// the value in decimal form on any UART terminal.
//
// \return None.
//
//===============================================================

void UART_putByteDecimalValue(uint8_t ui8HexByte)
{
	uint8_t ui8DecimalByteHundreds = 0;
	uint8_t ui8DecimalByteTens = 0;
	uint8_t ui8DecimalByteOnes = 0;
	uint8_t ui8TempVar;

	while (ui8HexByte > 0x09)
	{
		if (ui8DecimalByteTens < 9)
		{
			ui8DecimalByteTens++;
		}
		else
		{
			ui8DecimalByteHundreds++;
			ui8DecimalByteTens = 0;
		}
		ui8HexByte = ui8HexByte - 0x0A;
	}
	ui8DecimalByteOnes = ui8HexByte;

	if (ui8DecimalByteHundreds > 0)
	{
		ui8TempVar = UART_nibble2Ascii(ui8DecimalByteHundreds);
		UART_putChar(ui8TempVar);
	}
	if (ui8DecimalByteTens > 0)
	{
		ui8TempVar = UART_nibble2Ascii(ui8DecimalByteTens);
		UART_putChar(ui8TempVar);
	}
	ui8TempVar = UART_nibble2Ascii(ui8DecimalByteOnes);
	UART_putChar(ui8TempVar);
}

#endif
