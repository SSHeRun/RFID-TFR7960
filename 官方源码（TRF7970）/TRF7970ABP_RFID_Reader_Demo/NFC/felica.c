/*
 * {felica.c}
 *
 * {FeliCa specific functions & Anti-collision}
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

#include "felica.h"

//*****************************************************************************
//
//! \addtogroup felica_api FeliCa Reader API's
//! @{
//!
//! A FeliCa Reader issues commands based on the FeliCa specifications which
//! includes compliance with the JIS: X6319-4 specification. FeliCa
//! communication can occur at over-the-air bitrates of 212 or 424 kbps.
//! NFC Forum Type 3 standards are based on the FeliCa standards.
//!
//! For more information on FeliCa Readers and Tags please read the FeliCa
//! Specifications.
//
//*****************************************************************************

extern uint8_t	g_pui8TrfBuffer[NFC_FIFO_SIZE];
static volatile tTrfStatus g_sTrfStatus;

static uint8_t g_pui8IDm[8];
static uint32_t g_ui32NdefLength;

//*****************************************************************************
//
//! FeliCa_pollSingleSlot - Polls for FeliCa tags.
//!
//! This function issues the polling command for FeliCa with 1 slot
//! specified and then waits for a response from a FeliCa tag.
//!
//! \return ui8TagFound returns whether or not a FeliCa tag has
//! been detected or not.
//
//*****************************************************************************

uint8_t FeliCa_pollSingleSlot(void)
{
	uint8_t ui8Offset = 0;
	uint8_t ui8TagFound = STATUS_FAIL;
	uint8_t ui8LoopCount = 0;
	uint8_t ui8SlotNumber = 0;

	ui8SlotNumber = 0x00; 			// Set the number of slots to 1 slot.

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;				// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x90;				// Send without CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;				// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;				// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x60;				// Length of packet in bytes - lower and broken nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x06;				// Number of bytes that card will be expecting (including this byte)
	g_pui8TrfBuffer[ui8Offset++] = FELICA_POLLING;	// FeliCa Command Code:  Polling
	g_pui8TrfBuffer[ui8Offset++] = 0xFF;				// 0x12 = NFC T3T spec, 0xFF = wildcard
	g_pui8TrfBuffer[ui8Offset++] = 0xFF;				// 0xFC = NFC T3T spec, 0xFF = wildcard
	g_pui8TrfBuffer[ui8Offset++] = 0x01;				// RC: 0x00=no request, 0x01 to get SYS_CODE out of tag, 0x02 to get data rate capability
	g_pui8TrfBuffer[ui8Offset++] = ui8SlotNumber;	// 0x00 = 1 slot, 0x01 = 2 slots, 0x03 = 4 slots, 0x07 = 8 slots, 0x0F = 16 slots

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);

	g_sTrfStatus = TRF79xxA_waitRxData(10,30);		// 10 millisecond TX timeout, 30 millisecond RX timeout

	if(g_sTrfStatus == RX_COMPLETE)	// If received block data in buffer
	{
		if (g_pui8TrfBuffer[1] == 0x01)	// If the expected response code for the polling command has been received
		{
			ui8TagFound = STATUS_SUCCESS;	// Mark that a tag has been found

			for(ui8LoopCount = 2; ui8LoopCount < 10; ui8LoopCount++)
			{
				g_pui8IDm[ui8LoopCount-2] = g_pui8TrfBuffer[ui8LoopCount];   //array which copies T3T NFCID2 to g_pui8IDm
			}
		}
		else
		{
			// Did not receive a proper reply from the tag.
		}
	}
	else
	{
		// Unknown error occurred.
	}

#ifdef ENABLE_STANDALONE
	if(ui8TagFound == STATUS_SUCCESS)
	{
		LED_15693_ON;
		LED_14443B_ON;
	}
	else
	{
		LED_15693_OFF;
		LED_14443B_OFF;
	}
#endif

	return ui8TagFound;
}

//*****************************************************************************
//
//! FeliCa_ReadSingleBlock - Issue an addressed read command without encryption
//! to read out a single block of data from a FeliCa tag.
//!
//! \param ui8BlockNumber is the block number to be read
//!
//! This function issues command to read block data from FeliCa tags without
//! using encryption. This command includes the IDm of the tag which makes it
//! an addressed command.
//!
//! \return None.
//!
//*****************************************************************************

void FeliCa_readSingleBlock(uint8_t ui8BlockNumber)
{
	uint8_t ui8Offset = 0;
#ifdef ENABLE_HOST
	uint8_t ui8LoopCount = 0;
#endif

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;		// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x90;		// Send without CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;		// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x01;		// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x00;		// Length of packet in bytes - lower and broken nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x10;		// Number of bytes that card will be expecting (including this byte)
	g_pui8TrfBuffer[ui8Offset++] = FELICA_READ_NO_AUTH;		// FeliCa Command Code: Read Without Encryption
	g_pui8TrfBuffer[ui8Offset++] = g_pui8IDm[0];			// IDm : Manufacturing Code (g_pui8IDm is 8 byte array which is copied from T3T NFCID2 into g_pui8IDm)
	g_pui8TrfBuffer[ui8Offset++] = g_pui8IDm[1];			// IDm : Manufacturing Code
	g_pui8TrfBuffer[ui8Offset++] = g_pui8IDm[2];			// IDm : Card ID #
	g_pui8TrfBuffer[ui8Offset++] = g_pui8IDm[3]; 	  		// IDm : Card ID #
	g_pui8TrfBuffer[ui8Offset++] = g_pui8IDm[4];			// IDm : Card ID #
	g_pui8TrfBuffer[ui8Offset++] = g_pui8IDm[5];			// IDm : Card ID #
	g_pui8TrfBuffer[ui8Offset++] = g_pui8IDm[6];			// IDm : Card ID #
	g_pui8TrfBuffer[ui8Offset++] = g_pui8IDm[7];			// IDm : Card ID #
	g_pui8TrfBuffer[ui8Offset++] = 0x01;		// Number of services (m = 1)
	g_pui8TrfBuffer[ui8Offset++] = 0x0B;		// Service Code List (Little Endian, lower byte), 0x0B = Random Service, RO/RW permission, 0x09 = Random Service, RW permission
	g_pui8TrfBuffer[ui8Offset++] = 0x00;		// Service Code List (Little Endian, upper byte), block set to RO/RW permission, 0x00 = Service Number
	g_pui8TrfBuffer[ui8Offset++] = 0x01;		// Number of blocks that will be read
	g_pui8TrfBuffer[ui8Offset++] = 0x80;				// Block List Element
	g_pui8TrfBuffer[ui8Offset++] = ui8BlockNumber;		// Block # to read

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);		// Issue the Read Without Encrytion command

	g_sTrfStatus = TRF79xxA_waitRxData(10,30);		// 10 millisecond TX timeout, 30 millisecond RX timeout

	if (g_sTrfStatus == RX_COMPLETE)		// If data has been received
	{
		if (g_pui8TrfBuffer[1] == FELICA_READ_NO_AUTH_RESP)	// Check for expected response code
		{
			if ((g_pui8TrfBuffer[10] == 0x00) && (g_pui8TrfBuffer[11] == 0x00))	// Check that both status flags have no error
			{
				// If the Attribute Information Block was read, store the NDEF message length
				if (ui8BlockNumber == 0x00)
				{
					g_ui32NdefLength = (uint32_t)g_pui8TrfBuffer[26];			// NDEF Length LSByte
					g_ui32NdefLength |= ((uint32_t)g_pui8TrfBuffer[25]) << 8;
					g_ui32NdefLength |= ((uint32_t)g_pui8TrfBuffer[24]) << 16;	// NDEF Length MSByte
				}

#ifdef ENABLE_HOST
				UART_sendCString("Type 3 Block ");
				UART_putByte(ui8BlockNumber);				// Output block number
				UART_sendCString(" Data:  ");
				UART_putChar('[');

				for (ui8LoopCount = 13; ui8LoopCount < 29; ui8LoopCount++)
				{
					UART_putByte(g_pui8TrfBuffer[ui8LoopCount]);		// Send out data read from tag to host
				}

				UART_putChar(']');
				UART_putNewLine();
#endif
			}
			else
			{
				// Error occurred
#ifdef ENABLE_HOST
				UART_sendCString("Error, Block ");
				UART_putByte(ui8BlockNumber);
				UART_putNewLine();
				UART_sendCString("Status Flag1: ");
				UART_putByte(g_pui8TrfBuffer[10]);
				UART_putNewLine();
				UART_sendCString("Status Flag2: ");
				UART_putByte(g_pui8TrfBuffer[11]);
				UART_putNewLine();
#endif
			}
		}
		else
		{
			// Error occurred
#ifdef ENABLE_HOST
			UART_sendCString("Error, Block ");
			UART_putByte(ui8BlockNumber);
			UART_putNewLine();
			UART_sendCString("Resp. Code: ");
			UART_putByte(g_pui8TrfBuffer[1]);
			UART_putNewLine();
#endif
		}

	}
	else
	{
		// no response
	}
}

//*****************************************************************************
//
//! FeliCa_ReadFourBlocks - Issue an addressed read command without encryption
//! to read out four blocks of data from a FeliCa tag.
//!
//! \param ui8StartBlock is the first of the four blocks to be read.
//!
//! This function issues command to read four blocks of data from FeliCa tags
//! without using encryption. This command includes the IDm of the tag which
//! makes it an addressed command.
//!
//! \return None.
//
//*****************************************************************************

void FeliCa_readFourBlocks(uint8_t ui8StartBlock)
{
	uint8_t ui8Offset = 0;
#ifdef ENABLE_HOST
	uint8_t ui8LoopCount1 = 0;
	uint8_t ui8LoopCount2 = 0;
#endif

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;		// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x90;		// Send without CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;		// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x01;		// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x60;		// Length of packet in bytes - lower and broken nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x16;		// Number of bytes that card will be expecting (including this byte)
	g_pui8TrfBuffer[ui8Offset++] = FELICA_READ_NO_AUTH;		// FeliCa Command Code: Read Without Encryption
	g_pui8TrfBuffer[ui8Offset++] = g_pui8IDm[0];			// IDm : Manufacturing Code (g_pui8IDm is 8 byte array which is copied from T3T NFCID2 into g_pui8IDm)
	g_pui8TrfBuffer[ui8Offset++] = g_pui8IDm[1];			// IDm : Manufacturing Code
	g_pui8TrfBuffer[ui8Offset++] = g_pui8IDm[2];			// IDm : Card ID #
	g_pui8TrfBuffer[ui8Offset++] = g_pui8IDm[3]; 	  		// IDm : Card ID #
	g_pui8TrfBuffer[ui8Offset++] = g_pui8IDm[4];			// IDm : Card ID #
	g_pui8TrfBuffer[ui8Offset++] = g_pui8IDm[5];			// IDm : Card ID #
	g_pui8TrfBuffer[ui8Offset++] = g_pui8IDm[6];			// IDm : Card ID #
	g_pui8TrfBuffer[ui8Offset++] = g_pui8IDm[7];			// IDm : Card ID #
	g_pui8TrfBuffer[ui8Offset++] = 0x01;		// Number of services (m = 1)
	g_pui8TrfBuffer[ui8Offset++] = 0x0B;		// Service Code List (Little Endian, lower byte), 0x0B = Random Service, RO/RW permission, 0x09 = Random Service, RW permission
	g_pui8TrfBuffer[ui8Offset++] = 0x00;		// Service Code List (Little Endian, upper byte), block set to RO/RW permission, 0x00 = Service Number
	g_pui8TrfBuffer[ui8Offset++] = 0x04;		// Number of blocks that will be read
	g_pui8TrfBuffer[ui8Offset++] = 0x80;				// Block List Element
	g_pui8TrfBuffer[ui8Offset++] = ui8StartBlock;		// Block # to read
	g_pui8TrfBuffer[ui8Offset++] = 0x80;				// Block List Element
	g_pui8TrfBuffer[ui8Offset++] = ui8StartBlock+1;		// Block # to read
	g_pui8TrfBuffer[ui8Offset++] = 0x80;				// Block List Element
	g_pui8TrfBuffer[ui8Offset++] = ui8StartBlock+2;		// Block # to read
	g_pui8TrfBuffer[ui8Offset++] = 0x80;				// Block List Element
	g_pui8TrfBuffer[ui8Offset++] = ui8StartBlock+3;		// Block # to read

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);		// Issue the Read Without Encrytion command

	g_sTrfStatus = TRF79xxA_waitRxData(10,30);		// 10 millisecond TX timeout, 30 millisecond RX timeout

	if (g_sTrfStatus == RX_COMPLETE)
	{
		if (g_pui8TrfBuffer[1] == FELICA_READ_NO_AUTH_RESP)	// Check for expected response code
		{
			if ((g_pui8TrfBuffer[10] == 0x00) && (g_pui8TrfBuffer[11] == 0x00))	// Check that both status flags have no error
			{
				if (ui8StartBlock == 0x00)
				{
					g_ui32NdefLength = (uint32_t)g_pui8TrfBuffer[26];			// NDEF Length LSByte
					g_ui32NdefLength |= ((uint32_t)g_pui8TrfBuffer[25]) << 8;
					g_ui32NdefLength |= ((uint32_t)g_pui8TrfBuffer[24]) << 16;	// NDEF Length MSByte
				}

#ifdef ENABLE_HOST
				for (ui8LoopCount2 = 0; ui8LoopCount2 < 4; ui8LoopCount2++)	// Loop to output all 4 blocks of data
				{
					UART_sendCString("Type 3 Block ");
					UART_putByte(ui8StartBlock+ui8LoopCount2);				// Output block number
					UART_sendCString(" Data:  ");
					UART_putChar('[');

					for (ui8LoopCount1 = 0; ui8LoopCount1 < 16; ui8LoopCount1++)
					{
						UART_putByte(g_pui8TrfBuffer[ui8LoopCount1+(16*ui8LoopCount2)+13]);		// Send out data read from tag to host
					}

					UART_putChar(']');
					UART_putNewLine();
				}
#endif
			}
			else
			{
				// Error occurred
#ifdef ENABLE_HOST
				UART_sendCString("Error, Block ");
				UART_putByte(ui8StartBlock);
				UART_putNewLine();
				UART_sendCString("Status Flag1: ");
				UART_putByte(g_pui8TrfBuffer[10]);
				UART_putNewLine();
				UART_sendCString("Status Flag2: ");
				UART_putByte(g_pui8TrfBuffer[11]);
				UART_putNewLine();
#endif
			}
		}
		else
		{
			// Error occurred
#ifdef ENABLE_HOST
			UART_sendCString("Error, Block ");
			UART_putByte(ui8StartBlock);
			UART_putNewLine();
			UART_sendCString("Resp. Code: ");
			UART_putByte(g_pui8TrfBuffer[1]);
			UART_putNewLine();
#endif
		}
	}
	else
	{
		// Did not receive a proper response from tag
	}
}

//*****************************************************************************
//
//! FeliCa_putTagInformation - Print out information about FeliCa tag to a
//! host via UART interface.
//!
//! This function will print out information received through the response
//! of the FeliCa polling command including the tag ID and what (known)
//! FeliCa tag has been presented to the Reader.
//!
//! This function can ONLY be used directly after the POLLING command, or else
//! the global buffer will not contain the correct information anymore.
//!
//! \return None.
//
//*****************************************************************************

void FeliCa_putTagInformation(void)
{
	uint8_t ui8LoopCount = 0;

	if (g_pui8TrfBuffer[1] == 0x01)	// If the expected response code for the polling command has been received, then the tag information will be valid.
	{
#ifdef ENABLE_HOST
		UART_putNewLine();
		// http://www.sony.net/Products/felica/business/tech-support/list.html
		if (g_pui8TrfBuffer[11] == 0xF0)
		{
			UART_sendCString("RC-S965 (FeliCa Lite)"); 	// Tested with RC-S886 (RCFeliCa Lite family includes: RC-S978F, RC-S886, RC-S887, RC-S701)
		}
		else if (g_pui8TrfBuffer[11] == 0xF1)
		{
			UART_sendCString("RC-S966 (FeliCa Lite S)");	// Tested
		}
		else if (g_pui8TrfBuffer[11] == 0x01)
		{
			UART_sendCString("RC-S915 FeliCa"); 			// Tested with RC-860 (FeliCa RC-S915 family includes: RC-S934F, RC-S935, RC-S938, RC-S860, RC-S862, RC-S864, RC-S864, RC-S891)
		}
		else if (g_pui8TrfBuffer[11] == 0x0D)
		{
			UART_sendCString("RC-S960 FeliCa"); 			// Tested with RC-S889 (FeliCa RC-S960 family includes: RC-S970F, RC-S880, RC-S889)
		}
		else if (g_pui8TrfBuffer[11] == 0x20)
		{
			UART_sendCString("RC-S962 FeliCa IC"); 		// Tested with RC-S885 (FeliCa IC family includes: RC-S975F, RC-S885, RC-S888)
		}
		else if (g_pui8TrfBuffer[11] == 0x32)
		{
			UART_sendCString("RC-SA00/1 FeliCa Series"); // Tested with RC-S100CS
		}
		else if (g_pui8TrfBuffer[11] == 0x35)
		{
			UART_sendCString("RC-SA01/2 FeliCa Series"); // Untested
		}
		else if (g_pui8TrfBuffer[11] == 0xF2)
		{
			UART_sendCString("RC-S967 Series (FeliCa Link in Lite-S/Lite-S HT mode)");	// Tested
		}
		else if (g_pui8TrfBuffer[11] == 0xE1)
		{
			UART_sendCString("RC-S967 Series (FeliCa Link in Plug Mode)");
		}
		else if (g_pui8TrfBuffer[11] == 0xFF)
		{
			UART_sendCString("RC-S967 Series (FeliCa Link in NFC-DEP mode)");			// Tested
		}
		else if (g_pui8TrfBuffer[11] == 0xE0)
		{
			UART_sendCString("FeliCa Plug Series"); 		// FeliCa Plug family includes: RC-S801, RC-S802
		}
		else if ((g_pui8TrfBuffer[11] == 0x06) || (g_pui8TrfBuffer[11] == 0x07))
		{
			UART_sendCString("Mobile FeliCa IC Chip Ver. 1.0"); // Untested
		}
		else if ((g_pui8TrfBuffer[11] >= 0x10) && (g_pui8TrfBuffer[11] <= 0x13))
		{
			UART_sendCString("Mobile FeliCa IC Chip Ver. 2.0"); // Tested with AU Arrows Fujitsu FJL21
		}
		else if ((g_pui8TrfBuffer[11] >= 0x14) && (g_pui8TrfBuffer[11] <= 0x1F))
		{
			UART_sendCString("Mobile FeliCa IC Chip Ver. 3.0"); // Untested
		}
		else if (g_pui8TrfBuffer[11] == 0x0B)
		{
			UART_sendCString("Suica");					// Tested with Suica Card
		}
		else if (g_pui8TrfBuffer[11] == 0xC1)
		{
			UART_sendCString("NFCLink FeliCa CE");		// Tested with NFCLink Version 8.1.0.60
		}
		UART_putNewLine();
		UART_sendCString("NFC T3T ID2: ");
#endif
		for(ui8LoopCount = 2; ui8LoopCount < 10; ui8LoopCount++)
		{
#ifdef ENABLE_HOST
			UART_putByte(g_pui8TrfBuffer[ui8LoopCount]);
#endif
			g_pui8IDm[ui8LoopCount-2] = g_pui8TrfBuffer[ui8LoopCount];   //array which copies T3T NFCID2 to g_pui8IDm
		}
#ifdef ENABLE_HOST
		UART_putNewLine();
#endif
	}
	else
	{
		// Function was not called after a polling command
	}
}

//*****************************************************************************
//
//! FeliCa_getNDEFLength - Fetches g_ui32NdefLength value
//!
//! This function allows for higher layers to fetch the size of the NDEF
//! message length for NFC Forum Type 3 Tag Platform applications.
//!
//! \return g_ui32NdefLength returns the NDEF Length for the FeliCa tag.
//
//*****************************************************************************

uint32_t FeliCa_getNDEFLength(void)
{
	return g_ui32NdefLength;
}

//*****************************************************************************
//
//! FeliCa_getIDm - Fetches the FeliCa Tag IDm.
//!
//! This function allows for higher layers to fetch the tag IDm of a FeliCa
//! tag which has responded to the POLLING command.
//!
//! \return g_pui8IDm returns a pointer to the location of the IDm buffer.
//
//*****************************************************************************

uint8_t * FeliCa_getIDm(void)
{
	return g_pui8IDm;
}


//*****************************************************************************
//
//! FeliCa_init - Initialize all Global Variables for the FeliCa layer.
//!
//! This function will initialize all the global variables for the FeliCa layer
//! with the appropriate starting values or empty values/buffers.
//!
//! \return None
//
//*****************************************************************************

void FeliCa_init(void)
{
	uint8_t ui8LoopCount;

	g_ui32NdefLength = 0;

	for (ui8LoopCount = 0; ui8LoopCount < 8; ui8LoopCount++)
	{
		g_pui8IDm[ui8LoopCount] = 0x00;
	}
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
