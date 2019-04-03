/*
 * File Name: type_4_ndef.c
 *
 * Description: Type 4 NDEF Functions
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
#include "ndef.h"

//*****************************************************************************
//
//! \addtogroup ndef_api NFC Data Exchange Format (NDEF) Compliant API's
//! @{
//!
//! An NFC Forum Compliant Reader uses NDEF to read and write data to NFC
//! Forum compliant Tag Platforms. This firmware example only provides NDEF
//! read/write examples for NFC Forum Type 4A and Type 4B Tag Platforms.
//! Type 4A/4B use the same set of commands and have the same memory
//! structure.
//!
//! This firmware example hardcodes many variables, and has no error recovery
//! processes in place. It is just a rudimentary example for the purposes of
//! a simplistic demo. For a full NFC Forum Compliant NDEF reader example,
//! please see the Texas Instruments NFC Reader/Writer Application Report:
//! http://www.ti.com/lit/an/sloa227/sloa227.pdf
//!
//! For more information on NDEF compliant Readers and Tag Platforms, see
//! the documentation by the NFC Forum.
//
//*****************************************************************************

extern uint8_t g_pui8TrfBuffer[NFC_FIFO_SIZE];
static volatile tTrfStatus g_sTrfStatus;

static bool g_bBlockNumberBit;

//*****************************************************************************

//*****************************************************************************
//
//! NDEF_selectApplication - Issues an Application Select for NDEF.
//!
//! This function issues an Application Select command for NDEF compliant tags.
//! The NDEF Application ID is 0xD2760000850101.
//!
//! \return ui8SelectSuccess returns whether or not the command was processed
//! successfully by an NDEF compliant tag.
//
//*****************************************************************************

uint8_t NDEF_selectApplication(void)
{
	uint8_t ui8SelectSuccess = STATUS_FAIL;
	uint8_t ui8Offset = 0;

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;		// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;		// Send with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;		// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;		// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0xE0; 		// Length of packet in bytes - lower and broken nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x02 | g_bBlockNumberBit;	// I-Block PCB: Read Block 0 or Block 1, with CID = 0, NAD = 0, no chaining
	g_pui8TrfBuffer[ui8Offset++] = 0x00;		// CLA
	g_pui8TrfBuffer[ui8Offset++] = 0xA4;		// INS = Select (Application in this case)
	g_pui8TrfBuffer[ui8Offset++] = 0x04;		// P1
	g_pui8TrfBuffer[ui8Offset++] = 0x00;		// P2
	g_pui8TrfBuffer[ui8Offset++] = 0x07;		// Lc
	g_pui8TrfBuffer[ui8Offset++] = 0xD2;		// Data = 0xD2760000850101 - per NFC Forum Type 4 Tag Operation
	g_pui8TrfBuffer[ui8Offset++] = 0x76;
	g_pui8TrfBuffer[ui8Offset++] = 0x00;
	g_pui8TrfBuffer[ui8Offset++] = 0x00;
	g_pui8TrfBuffer[ui8Offset++] = 0x85;
	g_pui8TrfBuffer[ui8Offset++] = 0x01;
	g_pui8TrfBuffer[ui8Offset++] = 0x01;
	g_pui8TrfBuffer[ui8Offset++] = 0x00;		// Le

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);	// Issue the NDEF Command

	g_sTrfStatus = TRF79xxA_waitRxData(10,30);			// 10 millisecond TX timeout, 30 millisecond RX Timeout

	// If data received, should return same PCB, SW1 = 0x90, SW2 = 0x00
	if (g_sTrfStatus == RX_COMPLETE)
	{
		if((g_pui8TrfBuffer[0] == 0x02 | g_bBlockNumberBit) && (g_pui8TrfBuffer[1] == 0x90) && (g_pui8TrfBuffer[2] == 0x00))
		{
			ui8SelectSuccess = STATUS_SUCCESS;

			g_bBlockNumberBit = !g_bBlockNumberBit; 	// Toggle the PCB Block Number
		}
	}

	return ui8SelectSuccess;
}

//*****************************************************************************
//
//! NDEF_selectCapabilityContainer - Issues a File Select for the Capability
//! Container (CC) of an NDEF compliant tag.
//!
//! This function issues a File Select command in order to select the File ID
//! for the Capability Container (CC). The CC is always stored at File ID =
//! 0xE103.
//!
//! \return ui8SelectSuccess returns whether or not the command was processed
//! successfully by an NDEF compliant tag.
//
//*****************************************************************************

uint8_t NDEF_selectCapabilityContainer(void)
{
	uint8_t ui8SelectSuccess = STATUS_FAIL;
	uint8_t ui8Offset = 0;

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;	// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;	// Send with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;	// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;	// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x80;	// Length of packet in bytes - lower and broken nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x02 | g_bBlockNumberBit;	// I-Block PCB: Read Block 0 or Block 1, with CID = 0, NAD = 0, no chaining
	g_pui8TrfBuffer[ui8Offset++] = 0x00;	// CLA
	g_pui8TrfBuffer[ui8Offset++] = 0xA4;	// INS = Read CC
	g_pui8TrfBuffer[ui8Offset++] = 0x00; 	// P1
	g_pui8TrfBuffer[ui8Offset++] = 0x0C; 	// P2
	g_pui8TrfBuffer[ui8Offset++] = 0x02; 	// Lc
	g_pui8TrfBuffer[ui8Offset++] = 0xE1; 	// Data = 0xE103 - per NFC Forum Type 4 Tag Operation
	g_pui8TrfBuffer[ui8Offset++] = 0x03;

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);	// Issue the NDEF Command

	g_sTrfStatus = TRF79xxA_waitRxData(10,30);			// 10 millisecond TX timeout, 30 millisecond RX Timeout

	// If data received, should return same PCB, SW1 = 0x90, SW2 = 0x00
	if (g_sTrfStatus == RX_COMPLETE)
	{
		if((g_pui8TrfBuffer[0] == 0x02 | g_bBlockNumberBit) && (g_pui8TrfBuffer[1] == 0x90) && (g_pui8TrfBuffer[2] == 0x00))
		{
			ui8SelectSuccess = STATUS_SUCCESS;

			g_bBlockNumberBit = !g_bBlockNumberBit; 	// Toggle the PCB Block Number
		}
	}

	return ui8SelectSuccess;
}

//*****************************************************************************
//
//! NDEF_selectFile - Issues a File Select for NDEF.
//!
//! \param ui16FileID is the 2 byte File ID to select.
//!
//! This function issues a File Select command for NDEF compliant tags. To
//! select an NDEF File, the file ID of 0xE104 would be passed via the
//! ui16FileID parameter.
//!
//! \return ui8SelectSuccess returns whether or not the command was processed
//! successfully by an NDEF compliant tag.
//
//*****************************************************************************

uint8_t NDEF_selectFile(uint16_t ui16FileID)
{
	uint8_t ui8SelectSuccess = STATUS_FAIL;
	uint8_t ui8Offset = 0;

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;	// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;	// Send with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;	// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;	// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x80;	// Length of packet in bytes - lower and broken nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x02 | g_bBlockNumberBit;	// I-Block PCB: Read Block 0 or Block 1, with CID = 0, NAD = 0, no chaining
	g_pui8TrfBuffer[ui8Offset++] = 0x00;	// CLA
	g_pui8TrfBuffer[ui8Offset++] = 0xA4;	// INS = Select (File in this case)
	g_pui8TrfBuffer[ui8Offset++] = 0x00;
	g_pui8TrfBuffer[ui8Offset++] = 0x0C;
	g_pui8TrfBuffer[ui8Offset++] = 0x02;
	g_pui8TrfBuffer[ui8Offset++] = ((ui16FileID >> 8) & 0xFF);
	g_pui8TrfBuffer[ui8Offset++] = (ui16FileID & 0x00FF);

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);	// Issue the NDEF Command

	g_sTrfStatus = TRF79xxA_waitRxData(15,50);	// 15 millisecond TX timeout, 50 millisecond RX timeout

	// If data received, should return same PCB, SW1 = 0x90, SW2 = 0x00
	if (g_sTrfStatus == RX_COMPLETE)
	{
		if((g_pui8TrfBuffer[0] == 0x02 | g_bBlockNumberBit) && (g_pui8TrfBuffer[1] == 0x90) && (g_pui8TrfBuffer[2] == 0x00))
		{
			ui8SelectSuccess = STATUS_SUCCESS;

			g_bBlockNumberBit = !g_bBlockNumberBit; 	// Toggle the PCB Block Number
		}
	}

	return ui8SelectSuccess;
}

//*****************************************************************************
//
//! NDEF_readBinary - Issues a Read Binary for NDEF.
//!
//! \param ui16FileOffset is a 2 byte value that indicates where to start
//! reading data from a selected NDEF file.
//! \param ui8ReadLength is the amount of data to read out.
//!
//! This function issues a Read Binary command for NDEF compliant tags. The
//! NDEF compliant tag must have a file selected with the File Select command
//! before any data can be read with the Read Binary command.
//!
//! \return ui16NLEN returns either the NLEN variable when reading the first
//! two bytes of an NDEF file, 0x0000 for a successful Read Binary, or 0xFFFF
//! for a failed Read Binary.
//
//*****************************************************************************

uint16_t NDEF_readBinary(uint16_t ui16FileOffset, uint8_t ui8ReadLength)
{
	uint8_t ui8Offset = 0;
	uint16_t ui16NLEN = 0;

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;	// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;	// Send with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;	// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;	// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x60;	// Length of packet in bytes - lower and broken nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x02 | g_bBlockNumberBit;	// I-Block PCB: Read Block 0 or Block 1, with CID = 0, NAD = 0, no chaining
	g_pui8TrfBuffer[ui8Offset++] = 0x00;	// CLA
	g_pui8TrfBuffer[ui8Offset++] = 0xB0;	// INS = Read Binary
	g_pui8TrfBuffer[ui8Offset++] = ((ui16FileOffset >> 8) & 0xFF);	// File Offset where to start reading data
	g_pui8TrfBuffer[ui8Offset++] = (ui16FileOffset & 0x00FF);		// File Offset where to start reading data
	g_pui8TrfBuffer[ui8Offset++] = ui8ReadLength;					// Read Length

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);	// Issue the NDEF Command

	g_sTrfStatus = TRF79xxA_waitRxData(20,100);	// 20 millisecond TX timeout, 100 millisecond RX timeout

	if(g_sTrfStatus == RX_COMPLETE)
	{
		g_bBlockNumberBit = !g_bBlockNumberBit; 	// Toggle the PCB Block Number

		if ((ui16FileOffset == 0x00) && (ui8ReadLength == 0x02))
		{
			ui16NLEN = (g_pui8TrfBuffer[1] << 8) + g_pui8TrfBuffer[2];
		}
		else
		{
			ui16NLEN = 0x0000;
		}
	}
	else
	{
		ui16NLEN = 0xFFFF;
	}

	return ui16NLEN;
}

//*****************************************************************************
//
//! NDEF_readCapabilityContainer - Read the CC of a Type 4 Compliant NDEF Tag
//!
//! This function goes through the series of steps necessary to read out the
//! Capability Container (CC) from an NDEF Application File.
//!
//! \return ui16Status returns whether or not the commands were processed
//! successfully by an NDEF compliant tag.
//
//*****************************************************************************

uint16_t NDEF_readCapabilityContainer(void)
{
	uint16_t ui16Status = STATUS_FAIL;

	ui16Status = NDEF_selectCapabilityContainer();		// Selects the Capability Container

	if (ui16Status == STATUS_SUCCESS)
	{
		MCU_delayMillisecond(1);						// Short delay before sending next command

		ui16Status = NDEF_readBinary(0, 15);					// Read the contents of the capability container
		// Check if the Read Binary was successful.
		if (ui16Status == 0xFFFF)
		{
			ui16Status = STATUS_FAIL;
		}
		else
		{
			ui16Status = STATUS_SUCCESS;
		}
	}

	return ui16Status;
}

//*****************************************************************************
//
//! NDEF_readApplication - Read NDEF data from a Type 4 Compliant NDEF Tag
//!
//! This function goes through the series of steps necessary to read out NDEF
//! data from an NDEF Application File and print it out over UART.
//!
//! \return ui16Status returns whether or not the commands were processed
//! successfully by an NDEF compliant tag.
//
//*****************************************************************************

uint16_t NDEF_readApplication(void)
{
	uint16_t ui16Status = STATUS_FAIL;
	uint16_t ui16NdefLength = 0;
	uint8_t ui8NdefReadLength = 0;
#ifdef ENABLE_HOST
	uint8_t ui8NdefMessageOffset = 0;
	uint8_t ui8LoopCount = 0;
#endif

	NDEF_selectFile(0xE104);					// Selects NDEF Application
	ui16NdefLength = NDEF_readBinary(0, 2);		// Reads NDEF Application for length of message
	if (ui16NdefLength == 0xFFFF)
	{
		// Exit function
		return ui16Status = STATUS_FAIL;
	}

	if (ui16NdefLength > NFC_FIFO_SIZE)
	{
		ui8NdefReadLength = NFC_FIFO_SIZE;
#ifdef ENABLE_HOST
		UART_sendCString("NDEF Message Size Too Large, Displaying Partial Message Contents.");
		UART_putNewLine();
#endif
	}
	else
	{
		ui8NdefReadLength = ui16NdefLength;
	}

#ifndef ENABLE_HOST
	MCU_delayMillisecond(1);						// Short delay before sending next command
#endif

	ui16Status = NDEF_readBinary(2, ui8NdefReadLength);		// Reads NDEF Application for the NDEF content
	if (ui16NdefLength == 0xFFFF)
	{
		// Exit function
		return ui16Status = STATUS_FAIL;
	}
	else
	{
		ui16Status = STATUS_SUCCESS;
	}

#ifdef ENABLE_HOST
	UART_sendCString("NDEF Message: ");
	UART_putChar('[');
	if (g_pui8TrfBuffer[4] == 0x54)
	{
		ui8NdefMessageOffset = 8;
	}
	else if (g_pui8TrfBuffer[4] == 0x55)
	{
		ui8NdefMessageOffset = 6;
		if (g_pui8TrfBuffer[5] == 0x01)
		{
			UART_sendCString("http://wwww.");
		}
		else if (g_pui8TrfBuffer[5] == 0x03)
		{
			UART_sendCString("http://");
		}
	}
	else
	{
		ui8NdefMessageOffset = 0x00;
	}

	if (ui16NdefLength > (NFC_FIFO_SIZE-ui8NdefMessageOffset))
	{
		ui8NdefReadLength = NFC_FIFO_SIZE-ui8NdefMessageOffset;
	}
	else
	{
		ui8NdefReadLength = ui16NdefLength-ui8NdefMessageOffset+1;
	}

	for (ui8LoopCount = 0; ui8LoopCount < ui8NdefReadLength; ui8LoopCount++)
	{
		UART_putChar(g_pui8TrfBuffer[ui8NdefMessageOffset++]);
	}
	UART_putChar(']');
	UART_putNewLine();
	UART_putNewLine();
#endif

	return ui16Status;
}

//*****************************************************************************
//
//! NDEF_setBlockNumberBit - Sets the Block Number Bit for the NDEF layer.
//!
//! \param bValue is the new Block Number Bit (either 1 or 0).
//!
//! This function allows higher layers to set the Block Number Bit. This is
//! should be used when an NDEF process is re-started as each individual
//! NDEF function will toggle the Bit as needed automatically.
//!
//! \return None.
//
//*****************************************************************************

void NDEF_setBlockNumberBit(bool bValue)
{
	g_bBlockNumberBit = bValue;
}

//*****************************************************************************
//
//! NDEF_updateBinaryLength - Issues an Update Binary specifically to update
//! the NLEN of an NDEF file.
//!
//! \param ui16NLEN is the new NLEN value.
//!
//! This function issues an Update Binary command to update the NLEN variable
//! of the the NDEF File in an NFC Type 4 Tag Platform compliant tag.
//!
//! \return None.
//
//*****************************************************************************

void NDEF_updateBinaryLength(uint16_t ui16NLEN)
{
	uint8_t ui8Offset = 0;

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;	// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;	// Send with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;	// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;	// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x80;	// Length of packet in bytes - lower and broken nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x02 | g_bBlockNumberBit;	// I-Block PCB: Read Block 0 or Block 1, with CID = 0, NAD = 0, no chaining
	g_pui8TrfBuffer[ui8Offset++] = 0x00;	// CLA
	g_pui8TrfBuffer[ui8Offset++] = 0xD6;	// INS = Update Binary
	g_pui8TrfBuffer[ui8Offset++] = 0x00;	// Offset, P1
	g_pui8TrfBuffer[ui8Offset++] = 0x00;	// Offset, P2
	g_pui8TrfBuffer[ui8Offset++] = 0x02;	// Length, Lc
	g_pui8TrfBuffer[ui8Offset++] = ((ui16NLEN >> 8) & 0xFF);	// MSByte NLEN being set to 0
	g_pui8TrfBuffer[ui8Offset++] = (ui16NLEN & 0xFF);		// LSByte NLEN being set to 0

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);	// Issue the NDEF Command

	g_sTrfStatus = TRF79xxA_waitRxData(10,30);			// 10 millisecond TX timeout, 30 millisecond RX Timeout

	if (g_sTrfStatus == RX_COMPLETE)
	{
		g_bBlockNumberBit = !g_bBlockNumberBit; 	// Toggle the PCB Block Number
	}
}

//*****************************************************************************
//
//! NDEF_updateBinaryText - Issues an Update Binary with a hard coded Text RTD
//!
//! This function writes a new Text RTD NDEF message with the Update Binary
//! command. An NDEF File must be selected before using this function.
//!
//! \return None.
//
//*****************************************************************************

void NDEF_updateBinaryText(void)
{
	uint8_t ui8Offset = 0;

#ifdef ENABLE_HOST
	UART_sendCString("Writing New NDEF Message.");
	UART_putNewLine();
	UART_putNewLine();
#endif

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;	// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;	// Send with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;	// Write Continuous

	g_pui8TrfBuffer[ui8Offset++] = 0x01;	// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0xF0; // Length of packet in bytes - lower and broken nibbles of transmit byte length

	g_pui8TrfBuffer[ui8Offset++] = 0x02 | g_bBlockNumberBit;	// I-Block PCB: Read Block 0 or Block 1, with CID = 0, NAD = 0, no chaining
	g_pui8TrfBuffer[ui8Offset++] = 0x00; // CLA
	g_pui8TrfBuffer[ui8Offset++] = 0xD6;	// INS = update Binary
	g_pui8TrfBuffer[ui8Offset++] = 0x00;	// Offset, P1
	g_pui8TrfBuffer[ui8Offset++] = 0x02;	// Offset, P2

	g_pui8TrfBuffer[ui8Offset++] = 0x19;	// Lc, length being written (all bytes)

	g_pui8TrfBuffer[ui8Offset++] = 0xD1;	// MB = 1, ME = 1, Short Record, TNF = NFC Forum Well Known Type
	g_pui8TrfBuffer[ui8Offset++] = 0x01;	// Length of Record Type

	g_pui8TrfBuffer[ui8Offset++] = 0x15;	// Length of Text being written (21 bytes, hardcoded for now)
	g_pui8TrfBuffer[ui8Offset++] = 0x54;	// Text

	g_pui8TrfBuffer[ui8Offset++] = 0x02;	// Language Length
	g_pui8TrfBuffer[ui8Offset++] = 0x65;	// 'e' - For English
	g_pui8TrfBuffer[ui8Offset++] = 0x6E;	// 'n' - For English

	g_pui8TrfBuffer[ui8Offset++] = 0x4E;	// N
	g_pui8TrfBuffer[ui8Offset++] = 0x46;	// F
	g_pui8TrfBuffer[ui8Offset++] = 0x43;	// C
	g_pui8TrfBuffer[ui8Offset++] = 0x20;	//
	g_pui8TrfBuffer[ui8Offset++] = 0x50;	// P
	g_pui8TrfBuffer[ui8Offset++] = 0x6F;	// o
	g_pui8TrfBuffer[ui8Offset++] = 0x77;	// w
	g_pui8TrfBuffer[ui8Offset++] = 0x65;	// e
	g_pui8TrfBuffer[ui8Offset++] = 0x72;	// r
	g_pui8TrfBuffer[ui8Offset++] = 0x65;	// e
	g_pui8TrfBuffer[ui8Offset++] = 0x64;	// d
	g_pui8TrfBuffer[ui8Offset++] = 0x20;	//
	g_pui8TrfBuffer[ui8Offset++] = 0x42;	// B
	g_pui8TrfBuffer[ui8Offset++] = 0x79;	// y
	g_pui8TrfBuffer[ui8Offset++] = 0x20;	//
	g_pui8TrfBuffer[ui8Offset++] = 0x54;	// T
	g_pui8TrfBuffer[ui8Offset++] = 0x49;	// I
	g_pui8TrfBuffer[ui8Offset++] = 0x21;	// !

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);	// Issue the NDEF Command

	g_sTrfStatus = TRF79xxA_waitRxData(10,30);			// 10 millisecond TX timeout, 30 millisecond RX Timeout

	if (g_sTrfStatus == RX_COMPLETE)
	{
		g_bBlockNumberBit = !g_bBlockNumberBit; 	// Toggle the PCB Block Number

		MCU_delayMillisecond(5);

		NDEF_updateBinaryLength(0x19);
	}
}

//*****************************************************************************
//
//! NDEF_updateBinaryText - Issues an Update Binary with a hard coded URI RTD
//!
//! This function writes a new URI RTD NDEF message with the Update Binary
//! command. An NDEF File must be selected before using this function.
//!
//! \return None.
//
//*****************************************************************************

void NDEF_updateBinaryURI(void)
{
	uint8_t ui8Offset = 0;

#ifdef ENABLE_HOST
	UART_sendCString("Writing New NDEF Message.");
	UART_putNewLine();
	UART_putNewLine();
#endif

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;	// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;	// Send with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;	// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x01;	// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x50; // Length of packet in bytes - lower and broken nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x02 | g_bBlockNumberBit;	// I-Block PCB: Read Block 0 or Block 1, with CID = 0, NAD = 0, no chaining
	g_pui8TrfBuffer[ui8Offset++] = 0x00; // CLA
	g_pui8TrfBuffer[ui8Offset++] = 0xD6;	// INS = update Binary
	g_pui8TrfBuffer[ui8Offset++] = 0x00;	// Offset, P1
	g_pui8TrfBuffer[ui8Offset++] = 0x02;	// Offset, P2
	g_pui8TrfBuffer[ui8Offset++] = 0x0F;	// Lc, length being written (all bytes)
	g_pui8TrfBuffer[ui8Offset++] = 0xD1;	// MB = 1, ME = 1, Short Record, TNF = NFC Forum Well Known Type
	g_pui8TrfBuffer[ui8Offset++] = 0x01;	// Length of Record Type
	g_pui8TrfBuffer[ui8Offset++] = 0x0B;	// Length of URI being written (11 bytes, hardcoded for now)
	g_pui8TrfBuffer[ui8Offset++] = 0x55;	// URI
	g_pui8TrfBuffer[ui8Offset++] = 0x01;	// URI Identifier: http://wwww.
	g_pui8TrfBuffer[ui8Offset++] = 0x74;	// t
	g_pui8TrfBuffer[ui8Offset++] = 0x69;	// i
	g_pui8TrfBuffer[ui8Offset++] = 0x2E;	// .
	g_pui8TrfBuffer[ui8Offset++] = 0x63;	// c
	g_pui8TrfBuffer[ui8Offset++] = 0x6F;	// o
	g_pui8TrfBuffer[ui8Offset++] = 0x6D;	// m
	g_pui8TrfBuffer[ui8Offset++] = 0x2F;	// /
	g_pui8TrfBuffer[ui8Offset++] = 0x6E;	// n
	g_pui8TrfBuffer[ui8Offset++] = 0x66;	// f
	g_pui8TrfBuffer[ui8Offset++] = 0x63;	// c

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);	// Issue the NDEF Command

	g_sTrfStatus = TRF79xxA_waitRxData(10,30);			// 10 millisecond TX timeout, 30 millisecond RX Timeout

	if (g_sTrfStatus == RX_COMPLETE)
	{
		g_bBlockNumberBit = !g_bBlockNumberBit; 	// Toggle the PCB Block Number

		MCU_delayMillisecond(5);

		NDEF_updateBinaryLength(0x0F);
	}
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
