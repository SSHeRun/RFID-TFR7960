/*
 * File Name: iso15693.c
 *
 * Description: ISO15693 Specific Functions
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

#include "iso15693.h"


//*****************************************************************************
//
//! \addtogroup iso15693_api ISO15693 Read/Write API's
//! @{
//!
//! A ISO15693 Reader/Writer issues commands based on the ISO15693 specifications.
//! The ISO15693 specification is for Vicinity Integrated Circuit Cards (VICCs)
//! and communication with VICCs that are Type V tags occur at bitrates of
//! 6.62 kbps (low data rate) or 26.48 kbps (high data rate).
//!
//! For more information on ISO15693 Readers and Tags please read the ISO15693
//! Specifications.
//
//*****************************************************************************

//*****************************************************************************
//		Global Variables
//*****************************************************************************

extern uint8_t g_pui8TrfBuffer[NFC_FIFO_SIZE];
static volatile tTrfStatus g_sTrfStatus;

static uint8_t g_pui8Iso15693UId[8];
static uint8_t g_pui8AnticollisionMaskBuffer[8];
static uint8_t g_ui8TagDetectedCount;
static uint8_t g_ui8RecursionCount;

//*****************************************************************************
//
//! ISO15693_sendSingleSlotInventory - Issue a single slot Inventory command
//! for ISO15693 tags.
//!
//! This function issues a single slot Inventory command for tag detection
//! of ISO15693 tags. If a tag is found, the UID is stored in the
//! g_pui8Iso15693UId buffer.
//!
//! If UART is enabled, the tag ID is sent out to a host via UART.
//!
//! \return ui8Status returns either STATUS_SUCCESS or STATUS_FAIL to indicate
//! if the Inventory command resulted in a successful tag detection or not.
//
//*****************************************************************************

uint8_t ISO15693_sendSingleSlotInventory(void)
{
	uint8_t ui8Offset = 0;
	uint8_t ui8LoopCount = 0;
	uint8_t ui8Status = STATUS_FAIL;

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;		// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;		// Send with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;		// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;		// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x30;		// Length of packet in bytes - lower and broken nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x26;		// ISO15693 flags
	g_pui8TrfBuffer[ui8Offset++] = 0x01;		// Inventory command code
	g_pui8TrfBuffer[ui8Offset++] = 0x00;		// Mask Length = 0 (Also not sending AFI)

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);		// Issue the ISO15693 Inventory Command

	g_sTrfStatus = TRF79xxA_waitRxData(5,15);			// 5 millisecond TX timeout, 15 millisecond RX timeout

	if (g_sTrfStatus == RX_COMPLETE)		// If data has been received
	{
		if (g_pui8TrfBuffer[0] == 0x00)		// Confirm "no error" in response flags byte
		{
			ui8Status = STATUS_SUCCESS;

			// UID Starts at the 3rd received bit (1st is flags and 2nd is DSFID)
			for (ui8LoopCount = 2; ui8LoopCount < 10; ui8LoopCount++)
			{
				g_pui8Iso15693UId[ui8LoopCount-2] = g_pui8TrfBuffer[ui8LoopCount];	// Store UID into a Buffer
			}

			g_ui8TagDetectedCount = 1;
#ifdef ENABLE_HOST
			// Print out UID to UART Host
			UART_putNewLine();
			UART_sendCString("ISO15693 UID: ");
			UART_putChar('[');
			for (ui8LoopCount = 0; ui8LoopCount < 8; ui8LoopCount++)
			{
				UART_putByte(g_pui8Iso15693UId[7-ui8LoopCount]);		// Send UID to host
			}
			UART_putChar(']');
			UART_putNewLine();
#endif
		}
	}
	else
	{
		ui8Status = STATUS_FAIL;
	}

#ifdef ENABLE_STANDALONE
	if (ui8Status == STATUS_SUCCESS)
	{
		LED_15693_ON;		// LEDs indicate detected ISO15693 tag
	}
	else
	{
		LED_15693_OFF;
	}
#endif

	return ui8Status;
}

//*****************************************************************************
//
//! ISO15693_runAnticollision - Issue an Inventory command for either
//! 1 or 16 slot anticollision of ISO15693 tags.
//!
//! \param ui8ReqFlag are the request flags for ISO15693 commands.
//! \param ui8MaskLength is the number of significant bits in the mask value.
//! \param ui8Afi is the AFI to be issued with command (if AFI flag is
//! included in ui8ReqFlag).
//!
//! Function issues a sixteen slot Inventory command for the detection of
//! ISO15693 tags. If a tag is found, the UID is stored in the g_ui8Iso15693UId
//! buffer. The process will run until all ISO15693 detected have responded
//! with their UID's.
//!
//! The function uses a recursive call for the anticollision process. In order
//! to avoid stack overflows, a recursion counter is used to limit the maximum
//! number of recursions.
//!
//! The number of UID's which can be stored is set by g_ui8MaximumTagCount
//! and the declaration of the g_ui8Iso15693UId buffer. Once the buffer for
//! UID's is filled then no other UID's responses will be stored.
//!
//! If UART is enabled, the UID of each ISO15693 tag detected is sent out to a
//! host via UART.
//!
//! \return ui8Status returns STATUS_SUCCESS if the anticollision function
//! resulted in a successful tag detection. Otherwise, returns STATUS_FAIL.
//
//*****************************************************************************

uint8_t ISO15693_runAnticollision(uint8_t ui8ReqFlags, uint8_t ui8MaskLength, uint8_t ui8Afi)
{
	uint8_t ui8Offset = 0;
	uint8_t ui8LoopCount1;
	uint8_t ui8LoopCount2;
	uint16_t ui16TransmitByteLength;
	uint16_t ui16SlotNumber = 0x0000;
	uint8_t ui8MaskValue;
	uint8_t ui8MaskByteCount;
	uint8_t ui8Status = STATUS_FAIL;

	ui8ReqFlags &= ~BIT5;				// Clear Bit 5 to ensure 16 slot inventory is used.

	ui8MaskByteCount = (((ui8MaskLength >> 2) + 1) >> 1);	// Set ui8MaskByteCount based on the inputted Mask Length
															// ui8MaskByteCount will be 1 for length = 4 or 8,
															// ui8MaskByteCount will be 2 for length = 12 or 16,
															// and so on

	// Calculate how long the output byte will be
	if (ui8ReqFlags & 0x10) 							// Check if AFI will be included or not
	{
		ui16TransmitByteLength = ui8MaskByteCount + 4;	// Set packet size = Mask Value + Mask Length + AFI Byte + ISO15693 Command Code + ISO15693 Request Flags
	}
	else
	{
		ui16TransmitByteLength = ui8MaskByteCount + 3;	// Set packet size = Mask Value + Mask Length + ISO15693 Command Code + ISO15693 Request Flags
	}

	// Format Anti-collision command packet
	g_pui8TrfBuffer[ui8Offset++] = 0x8F;					// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;					// Send with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;					// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = (uint8_t) (ui16TransmitByteLength >> 8);		// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = (uint8_t) (ui16TransmitByteLength << 4);		// Length of packet in bytes - lower and broken nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = ui8ReqFlags;			// ISO15693 Request Flags
	g_pui8TrfBuffer[ui8Offset++] = 0x01;					// Inventory Request Command Code

	if (ui8ReqFlags & 0x10)								// Check if AFI will be included or not
	{
		g_pui8TrfBuffer[ui8Offset++] = ui8Afi;			// Optional AFI Byte
		g_pui8TrfBuffer[ui8Offset++] = ui8MaskLength;	// Mask Length
		if (ui8MaskLength > 0)
		{
			for (ui8LoopCount1 = 0; ui8LoopCount1 < ui8MaskByteCount; ui8LoopCount1++)
			{
				g_pui8TrfBuffer[ui8Offset++] = g_pui8AnticollisionMaskBuffer[(ui8MaskByteCount-ui8LoopCount1)];		// Fill in inputted Mask Values
			}
		}
	}
	else
	{
		g_pui8TrfBuffer[ui8Offset++] = ui8MaskLength;	// Mask Length
		if (ui8MaskLength > 0)
		{
			for (ui8LoopCount1 = 0; ui8LoopCount1 < ui8MaskByteCount; ui8LoopCount1++)
			{
				g_pui8TrfBuffer[ui8Offset++] = g_pui8AnticollisionMaskBuffer[((ui8MaskByteCount-1)-ui8LoopCount1)];		// Fill in inputted Mask Values
			}
		}
	}

	TRF79xxA_enableSlotCounter();

	TRF79xxA_resetIrqStatus();

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);		// Issue the ISO15693 Inventory Command

	g_sTrfStatus = TRF79xxA_getTrfStatus();

	if (g_sTrfStatus == TRF_IDLE || g_sTrfStatus == TX_WAIT)
	{
		TRF79xxA_waitTxIRQ(5);				// 5 millisecond TX timeout
	}

	for (ui8LoopCount2 = 1; ui8LoopCount2 <= 16; ui8LoopCount2++)
	{
		TRF79xxA_waitRxIRQ(15);		// 15 millisecond RX timeout

		g_sTrfStatus = TRF79xxA_getTrfStatus();	// Get the TRF797x Status

		switch (g_sTrfStatus)
		{
		case RX_COMPLETE:						// If data has been received, then UID is in the buffer
			if (g_pui8TrfBuffer[0] == 0x00)		// Confirm "no error" in response flags byte
			{
				ui8Status = STATUS_SUCCESS;

				// UID Starts at the 3rd received bit (1st is flags and 2nd is DSFID)
				for (ui8LoopCount1 = 2; ui8LoopCount1 < 10; ui8LoopCount1++)
				{
					g_pui8Iso15693UId[ui8LoopCount1-2] = g_pui8TrfBuffer[ui8LoopCount1];	// Store UID to a Buffer
				}

				g_ui8TagDetectedCount++;
#ifdef ENABLE_HOST
				// Print out UID to UART Host
				UART_putNewLine();
				UART_sendCString("ISO15693 UID: ");
				UART_putChar('[');
				for (ui8LoopCount1 = 0; ui8LoopCount1 < 8; ui8LoopCount1++)
				{
					UART_putByte(g_pui8Iso15693UId[7-ui8LoopCount1]);		// Send UID to host
				}
				UART_putChar(']');
				UART_putNewLine();
#endif
			}
			break;

		case COLLISION_ERROR:		// A collision has occurred for this slot
			ui16SlotNumber |= (0x01 << (ui8LoopCount2-1));	// Mark a collision occurred in the correct Slot Number bit.
			MCU_delayMillisecond(5);	// Allow time for tag to finish responding before issuing EOF
			break;

		case NO_RESPONSE_RECEIVED:	// No Response was received, break out of function as there is no tag for this slot
			break;

		case NO_RESPONSE_RECEIVED_15693:	// No Response was received, break out of function as there is no tag for this slot
			break;

		case PROTOCOL_ERROR:
			// Protocol Error occurred, exit out of anticollision
#ifdef ENABLE_HOST
			UART_sendCString("ISO15693 Anticollision Error");
			UART_putNewLine();
#endif
			TRF79xxA_setupInitiator(0x02);
			return ui8Status = STATUS_FAIL;

		default:
			break;
		}

		TRF79xxA_resetFIFO();				// FIFO has to be reset before receiving the next response

		if (ui8LoopCount2 < 16)		// If 16 slots used, and the last slot as not been reached, then send EOF (i.e. next slot indicator)
		{
			TRF79xxA_sendDirectCommand(TRF79XXA_STOP_DECODERS_CMD);
			TRF79xxA_sendDirectCommand(TRF79XXA_RUN_DECODERS_CMD);
			TRF79xxA_sendDirectCommand(TRF79XXA_TRANSMIT_NEXT_SLOT_CMD);
		}
		else if (ui8LoopCount2 == 16)	// Once at the end of slot 16, then stop the slot counter
		{
			TRF79xxA_sendDirectCommand(TRF79XXA_STOP_DECODERS_CMD);
			TRF79xxA_disableSlotCounter();
		}
	}

	TRF79xxA_disableSlotCounter();

	ui8MaskLength = ui8MaskLength + 4; 						// The mask length is a multiple of 4 bits

	ui8MaskByteCount = (((ui8MaskLength >> 2) + 1) >> 1);	// Set ui8MaskByteCount based on the inputted Mask Length
															// ui8MaskByteCount is 1 for length = 4 or 8,
															// ui8MaskByteCount is 2 for length = 12 or 16,
															// and so on

	// If the slot number pointer is not 0, the slot count is 16 (to indicate anticollision is needed),
	// the mask length doesn't exceed 60 bits, and the slot number is not 16 then proceed to recursive function call
	while ((ui16SlotNumber != 0x00)
			&& (ui8MaskLength < 61))
	{
		ui8MaskValue = 0x00;
		ui8LoopCount1 = 0;

		while (ui8LoopCount1 < 16)
		{
			if ((ui16SlotNumber & (0x01 << ui8LoopCount1)) != 0x00)
			{
				ui8MaskValue = ui8LoopCount1;

				ui16SlotNumber &= ~(0x01 << ui8LoopCount1); 				// Clear that slot bit from the array

				break;
			}
			ui8LoopCount1++;
		}

		if ((ui8MaskLength & 0x04) == 0x00)
		{
			ui8MaskValue = ui8MaskValue << 4;								// Shift slot pointer if mask length doesn't have Bit 2 (0x04) set (since it is a multiple of 4 bits)
		}
		else
		{																	// Otherwise re-copy the mask values
			for (ui8LoopCount1 = 7; ui8LoopCount1 > 0; ui8LoopCount1--)
			{
				g_pui8AnticollisionMaskBuffer[ui8LoopCount1] = g_pui8AnticollisionMaskBuffer[ui8LoopCount1 - 1];
			}
			g_pui8AnticollisionMaskBuffer[0] &= 0x00;									// And set the mask value for the first byte in the array = 0
		}

		g_pui8AnticollisionMaskBuffer[0] |= ui8MaskValue;								// Now update the mask value of the first byte based on the slot number pointer

		MCU_delayMillisecond(2);

		if (g_ui8RecursionCount < ISO15693_MAX_RECURSION_COUNT)
		{
			g_ui8RecursionCount++;
			ui8Status = ISO15693_runAnticollision(ui8ReqFlags, ui8MaskLength, ui8Afi); // Issue a recursive call with new Mask
		}
		else
		{
#ifdef ENABLE_HOST
			UART_sendCString("Error: Max Anticollision Recursions");
			UART_putNewLine();
#endif
			return ui8Status = STATUS_FAIL;
		}

		// Restore the Global AnticollisionMaskBuffer with the values from the current anticollision function.
		if ((ui8MaskLength & 0x04) == 0x00)
		{
			// If mask length doesn't have Bit 2 (0x04) set (since it is a multiple of 4 bits) - clear the upper nibble which is where the new mask value was placed
			g_pui8AnticollisionMaskBuffer[0] &= 0x0F;
		}
		else
		{	// Otherwise re-shift the mask values
			for (ui8LoopCount1 = 0; ui8LoopCount1 < 7; ui8LoopCount1++)
			{
				g_pui8AnticollisionMaskBuffer[ui8LoopCount1] = g_pui8AnticollisionMaskBuffer[ui8LoopCount1 + 1];
			}
			g_pui8AnticollisionMaskBuffer[7] = 0x00;									// And set the mask value for the first byte in the array = 0
		}
	}

#ifdef ENABLE_STANDALONE
	if (ui8Status == STATUS_SUCCESS)
	{
		LED_15693_ON;		// LEDs indicate detected ISO15693 tag
	}
	else
	{
		LED_15693_OFF;
	}
#endif

	// Clear any IRQs
	TRF79xxA_resetIrqStatus();

	// Reduce the recursion count as stack space will be freed on function exit
	if (g_ui8RecursionCount > 0)
	{
		g_ui8RecursionCount--;
	}

	return ui8Status;
}

//*****************************************************************************
//
//! ISO15693_sendGetSystemInfo - Issue the Get System Information command
//! for ISO15693 tags.
//!
//! \param ui8ReqFlag are the request flags for ISO15693 commands.
//!
//! This function issues a Get System Information command for ISO15693 tags.
//! This can be used to determine how many blocks of data can be read from
//! the tag.
//!
//! If UART is enabled, the contents of the Get System Information response
//! is output via UART.
//!
//! \return ui16NumberOfBlocks returns the number of blocks contained
//! in the ISO15693 tag.
//
//*****************************************************************************

uint16_t ISO15693_sendGetSystemInfo(uint8_t ui8ReqFlag)
{
	uint8_t ui8Offset = 0;
	uint16_t ui16NumberOfBlocks = 0x00;
#ifdef ENABLE_HOST
	uint8_t ui8LoopCount = 1;
	uint8_t ui8RxLength = 0;
#endif

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;		// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;		// Send with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;		// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;		// Length of packet in bytes - upper and middle nibbles of transmit byte length
	if (ui8ReqFlag & 0x20)
	{
		g_pui8TrfBuffer[ui8Offset++] = 0xA0;		// Length of packet in bytes - lower and broken nibbles of transmit byte length
	}
	else
	{
		g_pui8TrfBuffer[ui8Offset++] = 0x20;		// Length of packet in bytes - lower and broken nibbles of transmit byte length
	}
	g_pui8TrfBuffer[ui8Offset++] = ui8ReqFlag;	// ISO15693 flags
	g_pui8TrfBuffer[ui8Offset++] = 0x2B;			// Get System Information command code

	if (ui8ReqFlag & 0x20)
	{
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[0];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[1];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[2];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[3];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[4];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[5];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[6];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[7];	// Tag UID
	}

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);		// Issue the Get System Information command

	g_sTrfStatus = TRF79xxA_waitRxData(10,30);	// 10 millisecond TX timeout, 30 millisecond RX timeout

	if (g_sTrfStatus == RX_COMPLETE)		// If data has been received
	{
		if (g_pui8TrfBuffer[0] == 0x00)		// Confirm "no error" in response flags byte
		{
#ifdef ENABLE_HOST
			// Output received data to UART
			UART_sendCString("Get Sys Info Data: ");
			UART_putChar('[');

			ui8RxLength = TRF79xxA_getRxBytesReceived();

			for (ui8LoopCount = 1; ui8LoopCount < ui8RxLength; ui8LoopCount++)
			{
				UART_putByte(g_pui8TrfBuffer[ui8LoopCount]);		// Send Get System Info data to host
			}

			UART_putChar(']');
			UART_putNewLine();
#endif
			// Check to see that no error flags were sent and that there is a block number data available
			if (g_pui8TrfBuffer[0] == 0x00 && ((g_pui8TrfBuffer[1] & 0x07) == 0x07))
			{
				ui16NumberOfBlocks = g_pui8TrfBuffer[12];
			}
			else if (g_pui8TrfBuffer[0] == 0x00 && (((g_pui8TrfBuffer[1] & 0x07) == 0x06) || ((g_pui8TrfBuffer[1] & 0x07) == 0x05)))
			{
				ui16NumberOfBlocks = g_pui8TrfBuffer[11];
			}
			else if (g_pui8TrfBuffer[0] == 0x00 && ((g_pui8TrfBuffer[1] & 0x07) == 0x04))
			{
				ui16NumberOfBlocks = g_pui8TrfBuffer[10];
			}
		}
	}
	else if ((g_sTrfStatus == NO_RESPONSE_RECEIVED) || (g_sTrfStatus == NO_RESPONSE_RECEIVED_15693))
	{
		// Case for TI HF-I Pro and Standard tags
		ui16NumberOfBlocks = 0x0A;
	}

	return ui16NumberOfBlocks;
}

//*****************************************************************************
//
//! ISO15693_sendGetSystemInfoExtended - Issue the Get System Information
//! command for ISO15693 tags with the protocol extention flag set.
//!
//! \param ui8ReqFlag are the request flags for ISO15693 commands.
//!
//! This function issues a Get System Information command for ISO15693 tags
//! with the Protocol Extension bit set in the request flags. This can be
//! used to determine how many blocks of data can be read from the tag.
//!
//! If UART is enabled, the contents of the Get System Information
//! response is output via UART.
//!
//! \return ui16NumberOfBlocks returns the number of blocks contained in
//! the ISO15693 tag.
//
//*****************************************************************************

uint16_t ISO15693_sendGetSystemInfoExtended(uint8_t ui8ReqFlag)
{
	uint8_t ui8Offset = 0;
	uint16_t ui16NumberOfBlocks = 0x00;
#ifdef ENABLE_HOST
	uint8_t ui8LoopCount = 1;
	uint8_t ui8RxLength = 0;
#endif

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;		// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;		// Send with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;		// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;		// Length of packet in bytes - upper and middle nibbles of transmit byte length
	if (ui8ReqFlag & 0x20)
	{
		g_pui8TrfBuffer[ui8Offset++] = 0xA0;		// Length of packet in bytes - lower and broken nibbles of transmit byte length
	}
	else
	{
		g_pui8TrfBuffer[ui8Offset++] = 0x20;		// Length of packet in bytes - lower and broken nibbles of transmit byte length
	}
	g_pui8TrfBuffer[ui8Offset++] = ui8ReqFlag | 0x08;		// ISO15693 flags + protocol extension bit
	g_pui8TrfBuffer[ui8Offset++] = 0x2B;			// Get System Information command code

	if (ui8ReqFlag & 0x20)
	{
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[0];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[1];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[2];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[3];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[4];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[5];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[6];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[7];	// Tag UID
	}

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);		// Issue the Get System Information command

	g_sTrfStatus = TRF79xxA_waitRxData(10,30);	// 10 millisecond TX timeout, 30 millisecond RX timeout

	if (g_sTrfStatus == RX_COMPLETE)		// If data has been received
	{
		if (g_pui8TrfBuffer[0] == 0x00)		// Confirm "no error" in response flags byte
		{
#ifdef ENABLE_HOST
			UART_sendCString("Get Sys Info Data: ");
			UART_putChar('[');

			ui8RxLength = TRF79xxA_getRxBytesReceived();

			// Output received data to UART
			for (ui8LoopCount = 1; ui8LoopCount < ui8RxLength; ui8LoopCount++)
			{
				UART_putByte(g_pui8TrfBuffer[ui8LoopCount]);		// Send Get System Info data to host
			}

			UART_putChar(']');
			UART_putNewLine();
#endif
			// Check to see that no error flags were sent and that there is a block number data available
			if (g_pui8TrfBuffer[0] == 0x00 && ((g_pui8TrfBuffer[1] & 0x07) == 0x07))
			{
				ui16NumberOfBlocks = ((g_pui8TrfBuffer[13] << 8) | (g_pui8TrfBuffer[12])) ;
			}
			else if (g_pui8TrfBuffer[0] == 0x00 && (((g_pui8TrfBuffer[1] & 0x07) == 0x06) || ((g_pui8TrfBuffer[1] & 0x07) == 0x05)))
			{
				ui16NumberOfBlocks = ((g_pui8TrfBuffer[12] << 8) | (g_pui8TrfBuffer[11])) ;
			}
			else if (g_pui8TrfBuffer[0] == 0x00 && ((g_pui8TrfBuffer[1] & 0x07) == 0x04))
			{
				ui16NumberOfBlocks = ((g_pui8TrfBuffer[11] << 8) | (g_pui8TrfBuffer[10])) ;
			}
		}
	}

	return ui16NumberOfBlocks;
}

//*****************************************************************************
//
//! ISO15693_sendReadSingleBlock - Issue the Read Single Block command
//! for ISO15693 tags.
//!
//! \param ui8ReqFlag are the request flags for ISO15693 commands.
//! \param ui8BlockNumber is the block number to read data from.
//!
//! This function issues a Read Single Block with the specified request flags
//! and block number to read data from.
//!
//! If UART is enabled, the data read from the ISO15693 tag is output via UART.
//!
//! \return ui8Status returns either STATUS_SUCCESS or STATUS_FAIL
//! to indicate if the Read Single Block was successful or not.
//
//*****************************************************************************

uint8_t ISO15693_sendReadSingleBlock(uint8_t ui8ReqFlag, uint8_t ui8BlockNumber)
{
	uint8_t ui8Offset = 0;
	uint8_t ui8Status = STATUS_FAIL;
#ifdef ENABLE_HOST
	uint8_t ui8LoopCount = 1;
	uint8_t ui8RxLength = 0;
#endif

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;		// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;		// Send with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;		// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;		// Length of packet in bytes - upper and middle nibbles of transmit byte length
	if (ui8ReqFlag & 0x20)
	{
		g_pui8TrfBuffer[ui8Offset++] = 0xB0;		// Length of packet in bytes - lower and broken nibbles of transmit byte length
	}
	else
	{
		g_pui8TrfBuffer[ui8Offset++] = 0x30;		// Length of packet in bytes - lower and broken nibbles of transmit byte length
	}

	g_pui8TrfBuffer[ui8Offset++] = ui8ReqFlag;	// ISO15693 flags
	g_pui8TrfBuffer[ui8Offset++] = 0x20;			// Read Single Block command code

	if (ui8ReqFlag & 0x20)
	{
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[0];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[1];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[2];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[3];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[4];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[5];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[6];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[7];	// Tag UID
	}

	g_pui8TrfBuffer[ui8Offset++] = ui8BlockNumber;		// Block # (variable, for HF-I Plus device can go to 0x3F, Pro and Standard handled with "error" response flags)

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);		// Issue the Get System Information command

	g_sTrfStatus = TRF79xxA_waitRxData(5,15);		// 5 millisecond TX timeout, 15 millisecond RX timeout

	if (g_sTrfStatus == RX_COMPLETE)		// If data has been received
	{
		if (g_pui8TrfBuffer[0] == 0x00)		// Confirm "no error" in response flags byte
		{
#ifdef ENABLE_HOST
			UART_sendCString("Block ");
			UART_putByte(ui8BlockNumber);		// Output block number
			UART_sendCString(" Data: ");
			UART_putChar('[');

			ui8RxLength = TRF79xxA_getRxBytesReceived();

			if (ui8ReqFlag & BIT6) // Handle case for Option Flag causing one extra byte to be transmitted.
			{
				ui8Offset = 2;
			}
			else
			{
				ui8Offset = 1;
			}

			// Output received data to UART
			for (ui8LoopCount = ui8Offset; ui8LoopCount < ui8RxLength; ui8LoopCount++)
			{
				UART_putByte(g_pui8TrfBuffer[ui8LoopCount]);		// Send out data read from tag to host
			}

			UART_putChar(']');
			UART_putNewLine();
#endif
			// Response received
			ui8Status = STATUS_SUCCESS;
		}
		else		// An error has been sent back in the response byte
		{
#ifdef ENABLE_HOST
			// 	Indicates when an error occurs or block addresses are unreachable - useful for debugging
			UART_sendCString("Block ");
			UART_putByte(ui8BlockNumber);			// Output block number
			UART_sendCString(" Error");
			UART_putNewLine();
			UART_sendCString("ISO15693 Error Code: ");
			UART_putByte(g_pui8TrfBuffer[1]);		// Output ISO15693 error code
			UART_putNewLine();
#endif
			// Response with error
			ui8Status = STATUS_FAIL;
		}
	}
	else
	{
		// No response
		ui8Status = STATUS_FAIL;
	}

	return ui8Status;
}

//*****************************************************************************
//
//! ISO15693_sendReadMultipleBlocks - Issue the Read Multiple Block command
//! for ISO15693 tags.
//!
//! \param ui8ReqFlag are the request flags for ISO15693 commands.
//! \param ui8FirstBlock is the starting block number to read data from.
//! \param ui8NumberOfBlocks is the amount of blocks to read data from.
//!
//! This function issues a Read Multiple Block with the specified request
//! flags, the starting block number, and the number blocks to read data from.
//!
//! If UART is enabled, the data read from the ISO15693 tag is output via UART.
//!
//! \return ui8Status returns either STATUS_SUCCESS or STATUS_FAIL
//! to indicate if the Read Multiple Block was successful or not.
//
//*****************************************************************************

uint8_t ISO15693_sendReadMultipleBlocks(uint8_t ui8ReqFlag, uint8_t ui8FirstBlock, uint8_t ui8NumberOfBlocks)
{
	uint8_t ui8Offset = 0;
	uint8_t ui8Status = STATUS_FAIL;
#ifdef ENABLE_HOST
	uint8_t ui8LoopCount1 = 0;
	uint8_t ui8LoopCount2 = 0;
	uint8_t ui8RxLength = 0;
	uint8_t ui8BlockSize = 0;
#endif

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;		// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;		// Send with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;		// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;		// Length of packet in bytes - upper and middle nibbles of transmit byte length
	if (ui8ReqFlag & 0x20)
	{
		g_pui8TrfBuffer[ui8Offset++] = 0xC0;		// Length of packet in bytes - lower and broken nibbles of transmit byte length
	}
	else
	{
		g_pui8TrfBuffer[ui8Offset++] = 0x40;		// Length of packet in bytes - lower and broken nibbles of transmit byte length
	}

	g_pui8TrfBuffer[ui8Offset++] = ui8ReqFlag;	// ISO15693 flags
	g_pui8TrfBuffer[ui8Offset++] = 0x23;			// Read Multiple Block command code

	if (ui8ReqFlag & 0x20)
	{
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[0];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[1];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[2];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[3];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[4];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[5];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[6];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[7];	// Tag UID
	}

	g_pui8TrfBuffer[ui8Offset++] = ui8FirstBlock;			// Number of the first block to read from

	if (ui8NumberOfBlocks > 0)
	{
		g_pui8TrfBuffer[ui8Offset++] = ui8NumberOfBlocks-1;	// Index for number of blocks to be read - this value is one less than
	}
	else
	{
		// Invalid count provided
		return ui8Status = STATUS_FAIL;
	}

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);		// Issue the Get System Information command

	g_sTrfStatus = TRF79xxA_waitRxData(10,30+ui8NumberOfBlocks);	// 10 millisecond TX timeout, 30 millisecond RX timeout - adding number of blocks to extend timeout for larger read requests

	if (g_sTrfStatus == RX_COMPLETE)		// If data has been received
	{
		if (g_pui8TrfBuffer[0] == 0x00)		// Confirm "no error" in response flags byte
		{
#ifdef ENABLE_HOST
			ui8RxLength = TRF79xxA_getRxBytesReceived();

			if (ui8ReqFlag & BIT6) // Handle case for Option Flag causing one extra byte to be transmitted.
			{
				ui8Offset = 2;
			}
			else
			{
				ui8Offset = 1;
			}

			ui8LoopCount1 = ui8RxLength-ui8Offset;

			while (ui8LoopCount1 > 0)
			{
				if (ui8LoopCount1 > ui8NumberOfBlocks)
				{
					ui8LoopCount1 = ui8LoopCount1 - ui8NumberOfBlocks;
				}
				else
				{
					ui8LoopCount1 = 0;
				}
				ui8BlockSize++;
			}

			for (ui8LoopCount2 = 0; ui8LoopCount2 < ui8NumberOfBlocks; ui8LoopCount2++)
			{
				UART_sendCString("Block ");
				UART_putByte(ui8FirstBlock+ui8LoopCount2);		// Output block number
				UART_sendCString(" Data: ");
				UART_putChar('[');

				// Output received data to UART
				for (ui8LoopCount1 = 0; ui8LoopCount1 < ui8BlockSize; ui8LoopCount1++)
				{
					UART_putByte(g_pui8TrfBuffer[ui8Offset++]);		// Send out data read from tag to host
				}

				UART_putChar(']');
				UART_putNewLine();
			}
#endif
			// Response received
			ui8Status = STATUS_SUCCESS;
		}
		else		// An error has been sent back in the response byte
		{
#ifdef ENABLE_HOST
			// 	Indicates when an error occurs or block addresses are unreachable - useful for debugging
			UART_sendCString("Block ");
			UART_putByte(ui8FirstBlock);			// Output block number
			UART_sendCString(" Error");
			UART_putNewLine();
			UART_sendCString("ISO15693 Error Code: ");
			UART_putByte(g_pui8TrfBuffer[1]);		// Output ISO15693 error code
			UART_putNewLine();
#endif
			// Response with error
			ui8Status = STATUS_FAIL;
		}
	}
	else
	{
		// No response
		ui8Status = STATUS_FAIL;
	}

	return ui8Status;
}


//*****************************************************************************
//
//! ISO15693_sendReadSingleBlockExtended - Issue the Read Single Block
//! command for ISO15693 tags with the protocol extention flag set
//!
//! \param ui8ReqFlag are the request flags for ISO15693 commands.
//! \param ui16BlockNumber is the block number to read data from.
//!
//! This function issues a Read Single Block with the block number and the
//! specified request flags, including the Protocol Extension bit, to read
//! data from ISO15693 tags which require the use of extended protocol
//! commands.
//!
//! If UART is enabled, the data read from the ISO15693 tag is output via UART.
//!
//! \return ui8Status returns either STATUS_SUCCESS or STATUS_FAIL
//! to indicate if the Read Single Block was successful or not.
//
//*****************************************************************************

uint8_t ISO15693_sendReadSingleBlockExtended(uint8_t ui8ReqFlag, uint16_t ui16BlockNumber)
{
	uint8_t ui8Offset = 0;
	uint8_t ui8Status = STATUS_FAIL;
#ifdef ENABLE_HOST
	uint8_t ui8LoopCount = 1;
	uint8_t ui8RxLength = 0;
#endif

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;		// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;		// Send with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;		// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;		// Length of packet in bytes - upper and middle nibbles of transmit byte length
	if (ui8ReqFlag & 0x20)
	{
		g_pui8TrfBuffer[ui8Offset++] = 0xC0;		// Length of packet in bytes - lower and broken nibbles of transmit byte length
	}
	else
	{
		g_pui8TrfBuffer[ui8Offset++] = 0x40;		// Length of packet in bytes - lower and broken nibbles of transmit byte length
	}
	g_pui8TrfBuffer[ui8Offset++] = ui8ReqFlag | 0x08;	// ISO15693 flags with protocol extension bit set
	g_pui8TrfBuffer[ui8Offset++] = 0x20;		// Read Single Block command code

	if (ui8ReqFlag & 0x20)
	{
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[0];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[1];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[2];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[3];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[4];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[5];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[6];	// Tag UID
		g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[7];	// Tag UID
	}

	g_pui8TrfBuffer[ui8Offset++] = (uint8_t) (ui16BlockNumber & 0xFF);			// Block # (variable, for this device it can go to 0xFF)
	g_pui8TrfBuffer[ui8Offset++] = (uint8_t) ((ui16BlockNumber >> 8) & 0xFF);		// Block # (variable, for this device it can go to 0x07)

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);		// Issue the Read Single Block command

	g_sTrfStatus = TRF79xxA_waitRxData(5,15);		// 5 millisecond TX timeout, 15 millisecond RX timeout

	if (g_sTrfStatus == RX_COMPLETE)		// If data has been received
	{
		if (g_pui8TrfBuffer[0] == 0x00)		// Confirm "no error" in response flags byte
		{
			// Response received
			ui8Status = STATUS_SUCCESS;

#ifdef ENABLE_HOST
			UART_sendCString("Block ");
			UART_putByte((ui16BlockNumber >> 8) & 0xFF);			// Output block number
			UART_putByte(ui16BlockNumber & 0xFF);
			UART_sendCString(" Data: ");
			UART_putChar('[');

			ui8RxLength = TRF79xxA_getRxBytesReceived();

			if (ui8ReqFlag & BIT6) // Handle case for Option Flag causing one extra byte to be transmitted.
			{
				ui8Offset = 2;
			}
			else
			{
				ui8Offset = 1;
			}

			// Output received data to UART
			for (ui8LoopCount = ui8Offset; ui8LoopCount < ui8RxLength; ui8LoopCount++)
			{
				UART_putByte(g_pui8TrfBuffer[ui8LoopCount]);		// Send out data read from tag to host
			}

			UART_putChar(']');
			UART_putNewLine();
#endif
		}
		else
		{
			// Received an error from the tag
			ui8Status = STATUS_FAIL;
#ifdef ENABLE_HOST
			// 	Indicates when an error occurs or block addresses are unreachable - useful for debugging
			UART_sendCString("Block ");
			UART_putByte(((ui16BlockNumber >> 8) & 0xFF));		// Output block number
			UART_putByte((ui16BlockNumber & 0xFF));
			UART_sendCString(" Error");
			UART_putNewLine();
			UART_sendCString("ISO15693 Error Code: ");
			UART_putByte(g_pui8TrfBuffer[1]);						// Output ISO15693 error code
			UART_putNewLine();
#endif
		}
	}
	else
	{
		// Did not receive a proper response from tag
		ui8Status = STATUS_FAIL;
	}

	return ui8Status;
}

//*****************************************************************************
//
//! ISO15693_sendWriteSingleBlock - Issue the Write Single Block command
//! for ISO15693 tags.
//!
//! \param ui8ReqFlag are the request flags for ISO15693 commands.
//! \param ui8BlockNumber is the block number to write data to.
//! \param ui8BlockSize is the tag block size.
//! \param pui8BlockData is the data to be written.
//!
//! Function issues an addressed Write Single Block with the specified request
//! flags as well as the address flag. The write single block command writes
//! the provided data to the addressed ISO15693 tag.
//!
//! The function will always add the address flag to the command as a good
//! practice to avoid overwriting other ISO15693 tags in the vicinity.
//!
//! This function natively supports writing to tags with more than 4 bytes of
//! data per block through the ui8BlockSize variable.
//!
//! \return ui8Status returns either STATUS_SUCCESS or STATUS_FAIL to
//! indicate if the Write Single Block was successful or not.
//
//*****************************************************************************

uint8_t ISO15693_sendWriteSingleBlock(uint8_t ui8ReqFlag, uint8_t ui8BlockNumber, uint8_t ui8BlockSize, uint8_t * pui8BlockData)
{
	uint8_t ui8Offset = 0;
	uint8_t ui8Status = STATUS_FAIL;
	uint8_t ui8LoopCount = 0;

	ui8ReqFlag = ui8ReqFlag | 0x20; 		// Mandatory use of addressed writes - highly recommended practice

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;		// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;		// Send with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;		// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = (((0x0B+ui8BlockSize) & 0xF0) >> 0x04);	// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = ((0x0B+ui8BlockSize) << 0x04);			// Length of packet in bytes - lower and broken nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = ui8ReqFlag;	// ISO15693 flags
	g_pui8TrfBuffer[ui8Offset++] = 0x21;			// Write Single Block command code
	g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[0];	// Tag UID
	g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[1];	// Tag UID
	g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[2];	// Tag UID
	g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[3];	// Tag UID
	g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[4];	// Tag UID
	g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[5];	// Tag UID
	g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[6];	// Tag UID
	g_pui8TrfBuffer[ui8Offset++] = g_pui8Iso15693UId[7];	// Tag UID
	g_pui8TrfBuffer[ui8Offset++] = ui8BlockNumber;		// Block # (variable, for HF-I Plus device can go to 0x3F, Pro and Standard handled with "error" response flags)
	for (ui8LoopCount = 0; ui8LoopCount < ui8BlockSize; ui8LoopCount++)
	{
		g_pui8TrfBuffer[ui8Offset++] = pui8BlockData[ui8LoopCount];			// Data to write to tag
	}

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);		// Issue the Get System Information command

	g_sTrfStatus = TRF79xxA_getTrfStatus();

	// Special handling to cover option flag use case for TI Tag-It HF-I ISO15693 transponders
	if (g_sTrfStatus == TRF_IDLE || g_sTrfStatus == TX_WAIT)
	{
		// Check if the option flag is set
		if (ui8ReqFlag & 0x40)
		{
			TRF79xxA_waitTxIRQ(10);	// 10 millisecond TX timeout

			g_sTrfStatus = TRF79xxA_getTrfStatus();

			if (g_sTrfStatus == TX_COMPLETE)	// If transmit is complete
			{
				MCU_delayMillisecond(10);
				TRF79xxA_sendDirectCommand(TRF79XXA_TRANSMIT_NEXT_SLOT_CMD);		// Send out End of Frame marker
				TRF79xxA_waitRxIRQ(30);				// 30 millisecond RX timeout
			}
			else								// Otherwise return an error
			{
				return ui8Status = STATUS_FAIL;
			}
		}
		else
		{
			TRF79xxA_waitTxIRQ(10);	// 10 millisecond TX timeout
			TRF79xxA_waitRxIRQ(30);	// 30 millisecond RX timeout
		}
	}
	else if (g_sTrfStatus == TX_COMPLETE)
	{
		// Check if the option flag is set
		if (ui8ReqFlag & 0x40)
		{
			MCU_delayMillisecond(10);
			TRF79xxA_sendDirectCommand(TRF79XXA_TRANSMIT_NEXT_SLOT_CMD);		// Send out End of Frame marker
		}

		TRF79xxA_waitRxIRQ(30);				// 30 millisecond RX timeout
	}
	else
	{
		return ui8Status = STATUS_FAIL;
	}

	g_sTrfStatus = TRF79xxA_getTrfStatus();

	if (g_sTrfStatus == RX_COMPLETE)		// If data has been received
	{
		if (g_pui8TrfBuffer[0] == 0x00)		// Confirm "no error" in response flags byte
		{
			// Response received
			ui8Status = STATUS_SUCCESS;
		}
		else		// An error has been sent back in the response byte
		{
			ui8Status = STATUS_FAIL;
		}
	}
	else
	{
		// No response
		ui8Status = STATUS_FAIL;
	}

	return ui8Status;
}

//*****************************************************************************
//
//! ISO15693_getUid - Fetches the ISO15693 Tag UID.
//!
//! \param ui8Index is the index for the ISO15693 UID array.
//!
//! This function allows for higher layers to fetch the tag UID of one ISO15693
//! tag.
//!
//! \return g_pui8Iso15693UId returns a pointer to the UID buffer.
//
//*****************************************************************************

uint8_t * ISO15693_getUid(void)
{
	return g_pui8Iso15693UId;
}

//*****************************************************************************
//
//! ISO15693_getTagCount - Fetches the number of ISO15693 tags which have been
//! detected.
//!
//! This function allows for higher layers to fetch the tag detected count in
//! order to know how many ISO15693 tags have been detected by the anticollsion
//! process.
//!
//! \return g_ui8TagDetectedCount returns the count of ISO15693 tags detected.
//
//*****************************************************************************

uint8_t ISO15693_getTagCount(void)
{
	return g_ui8TagDetectedCount;
}

//*****************************************************************************
//
//! ISO15693_resetTagCount - Reset number of the ISO15693 tags detected.
//!
//! This function allows for higher layers to reset the tag detected counter.
//!
//! \return None
//
//*****************************************************************************

void ISO15693_resetTagCount(void)
{
	g_ui8TagDetectedCount = 0;
}


//*****************************************************************************
//
//! ISO15693_resetRecursionCount - Clear the recursion counter for the
//! anticollision process.
//!
//! This function allows for higher layers to reset the anticollision process
//! recursion counter.
//!
//! \return None
//
//*****************************************************************************

void ISO15693_resetRecursionCount(void)
{
	g_ui8RecursionCount = 0;
}

//*****************************************************************************
//
//! ISO15693_init - Initialize all Global Variables for the ISO15693 layer.
//!
//! This function will initialize all the global variables for the ISO15693
//! layer with the appropriate starting values or empty values/buffers.
//!
//! \return None
//
//*****************************************************************************

void ISO15693_init(void)
{
	uint8_t ui8LoopCount;

	g_ui8TagDetectedCount = 0;
	g_ui8RecursionCount = 0;

	for (ui8LoopCount = 0; ui8LoopCount < 8; ui8LoopCount++)
	{
		g_pui8Iso15693UId[ui8LoopCount] = 0xFF;  // Initialize with invalid UID
	}
	for (ui8LoopCount = 0; ui8LoopCount < 8; ui8LoopCount++)
	{
		g_pui8AnticollisionMaskBuffer[ui8LoopCount] = 0x00;
	}
}


//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
