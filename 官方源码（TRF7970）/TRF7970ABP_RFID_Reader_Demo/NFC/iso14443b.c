/*
 * File Name: iso14443b.c
 *
 * Description: ISO14443B Specific Functions
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

#include "iso14443b.h"

//*****************************************************************************
//
//! \addtogroup iso14443b_api ISO14443B Read/Write API's
//! @{
//!
//! A ISO14443B Reader/Writer issues commands based on the ISO14443-3 and -4
//! specifications. The ISO14443 specification is for Proximity Integrated
//! Circuit Cards (PICCs) and communication with PICCs that are Type 4B tags
//! can occur at bitrates of 106, 212, 424, or 848 kbps.
//!
//! For more information on ISO14443B Readers and Tags please read the
//! ISO14443-3 and ISO14443-4 Specifications.
//
//*****************************************************************************

//*****************************************************************************
//		Global Variables
//*****************************************************************************

extern uint8_t g_pui8TrfBuffer[NFC_FIFO_SIZE];
static volatile tTrfStatus g_sTrfStatus;

static uint8_t g_pui8Pupi[4];
static bool g_bType4BCompliant;

//*****************************************************************************
//
//! ISO14443B_runAnticollision - Performs ISO14443B anti-collision sequence.
//!
//! \param ui8NumberofSlots is the number of slots
//! \param bRecursion is a flag to indicate if the function should be
//! recursively called.
//!
//! This function handles the process of ISO14443 Anticollision. After issuing
//! a REQB with the provided number of slots, each slot will be checked for a
//! tag response before issuing the Slot-MARKER command. If any tags are found,
//! the function will return true.
//!
//! \return bTagFound returns true if a tag has been found, and false if no tag
//! has been detected after all slots have been checked.
//
//*****************************************************************************

bool ISO14443B_runAnticollision(uint8_t ui8NumberofSlots, bool bRecursion)
{
	uint8_t ui8APn = 0;
	uint8_t ui8SlotCount = 0;
	uint8_t ui8Status = STATUS_FAIL;
	bool bCollision = false;
	bool bTagFound = false;

	ui8NumberofSlots = (ui8NumberofSlots & 0x07);			// Number of Slots value is determined by only 3 bits, discard all others.

	// Set Slot Count based on how many slots were passed in Anticollision function call
	switch (ui8NumberofSlots)
	{
	case 0x00:
		ui8SlotCount = 1;
		break;
	case 0x01:
		ui8SlotCount = 2;
		break;
	case 0x02:
		ui8SlotCount = 4;
		break;
	case 0x03:
		ui8SlotCount = 8;
		break;
	case 0x04:
		ui8SlotCount = 16;
		break;
	}

	if (ISO14443B_sendPollCmd(REQB,ui8NumberofSlots))	// Issue a REQB Command with the inputted number of slots
	{
		bTagFound = true;	// Mark that a tag has been found
#ifdef ENABLE_STANDALONE
		LED_14443B_ON;		// LEDs indicate detected ISO15693 tag
#endif
	}
	else if (g_sTrfStatus == COLLISION_ERROR)				// If a collision occurred
	{
		bCollision = true;									// Then set the collision flag
	}

	ui8APn = 0x01; 			// Start the APn counter at 1 which will signify slot #2 when the Slot-MARKER command is sent

	while (ui8APn < ui8SlotCount)
	{
		ui8Status = ISO14443B_sendSlotMarker(((ui8APn << 4) | 0x05));	// Issue the Slot-MARKER command with current APn

		if (ui8Status == STATUS_SUCCESS)					// Tag responded without collision
		{
			bTagFound = true;								// Mark that a tag has been found
#ifdef ENABLE_STANDALONE
			LED_14443B_ON;		// LEDs indicate detected ISO15693 tag
#endif
		}
		else												// An error occurred when getting the tag response
		{
			g_sTrfStatus = TRF79xxA_getTrfStatus();			// Fetch TRF797x Status

			if (g_sTrfStatus == COLLISION_ERROR)				// If a collision occurred
			{
				bCollision = true;							// Then set the collision flag
			}
		}

		ui8APn++;											// Increase APn
	}

	if(bCollision && bRecursion)							// If a collision occurred and recursion is allowed
	{
		if (ui8NumberofSlots < 0x04)						// If the number of slots is not the maximum allowed
		{
			MCU_delayMillisecond(6);							// Delay prior to recursive function call
			bTagFound = ISO14443B_runAnticollision(ui8NumberofSlots+1,true);	// Recursive call for more time slots to try and resolve collisions
		}
	}

#ifdef ENABLE_STANDALONE
	if (bTagFound)
	{
		LED_14443B_ON;		// LEDs indicate detected ISO15693 tag
	}
	else
	{
		LED_14443B_OFF;
	}
#endif

	return bTagFound;
}

//*****************************************************************************
//
//! ISO14443B_sendPollCmd - Issue ISO14443B Polling command.
//!
//! \param ui8Command is the polling command to be issued.
//! \param ui8NValue indicates the number of slots which ISO14443B
//! tags may reply to.
//!
//! This function issues the ISO14443B polling command based on the command
//! input (either REQB or WUPB) and with the indicated number of slots.
//!
//! \return ui8Status returns either STATUS_SUCCESS or STATUS_FAIL
//! to indicate if a tag replied to the Polling command.
//
//*****************************************************************************

uint8_t ISO14443B_sendPollCmd(uint8_t ui8Command, uint8_t ui8NValue)
{
	uint8_t ui8Offset = 0;
	uint8_t ui8Status = STATUS_FAIL;
	uint8_t ui8LoopCount = 0;

	ui8NValue = (ui8NValue & 0x07);					// Number of Slots value is determined by only 3 bits, discard all others.

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;				// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;				// Send with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;				// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;				// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x30;				// Length of packet in bytes - lower and broken nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x05;				// REQB/SENSB_REQ Flag
	g_pui8TrfBuffer[ui8Offset++] = 0x00; 			// AFI, 0x00 for all families and sub-families
	g_pui8TrfBuffer[ui8Offset++] = ui8Command | ui8NValue;		// PARASM = No Extended ATQB Support + Sending REQB/WUPB + Number of slots

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);	// Issue the ISO14443B Anti-collision command

	g_sTrfStatus = TRF79xxA_waitRxData(10,30);			// 10 millisecond TX timeout, 30 millisecond RX Timeout

	if ((g_sTrfStatus == RX_COMPLETE) && (TRF79xxA_getRxBytesReceived() != 0x00))			// Received PUPI in buffer
	{
		ui8Status = STATUS_SUCCESS;					// Received reply with no collisions

#ifdef ENABLE_STANDALONE
		LED_14443B_ON;		// LEDs indicate detected ISO15693 tag
#endif

		for(ui8LoopCount = 1; ui8LoopCount < 5; ui8LoopCount++)
		{
			g_pui8Pupi[ui8LoopCount-1] = g_pui8TrfBuffer[ui8LoopCount];	// Store PUPI into array
		}

		if (g_pui8TrfBuffer[10] & 0x01)
		{
			g_bType4BCompliant = true; 				// Set Type B compliance info
		}
		else
		{
			g_bType4BCompliant = false; 			// Set Type B compliance info
		}

#ifdef ENABLE_HOST
		UART_putNewLine();
		UART_sendCString("ISO14443B PUPI: ");
		UART_putChar('[');
		for(ui8LoopCount = 0; ui8LoopCount < 4; ui8LoopCount++)
		{
			UART_putByte(g_pui8Pupi[ui8LoopCount]);	// Print PUPI to UART
		}
		UART_putChar(']');
		UART_putNewLine();
#endif
	}
	else if (g_sTrfStatus == COLLISION_ERROR)		// Collision occurred
	{
		ui8Status = STATUS_FAIL;					// Set collision flag
	}
	else
	{
		ui8Status = STATUS_FAIL;
	}

	return ui8Status;
}

//*****************************************************************************
//
//! ISO14443B_sendSlotMarker - Issue the Slot-MARKER command.
//!
//! \param ui8APn is the anticollision prefix.
//!
//! This function issues the ISO14443B Slot-MARKER command with the current slot
//! number which is indicated by the value of the anticollision prefix byte, n.
//!
//! \return ui8Status returns either STATUS_SUCCESS or STATUS_FAIL
//! to indicate if a tag replied to the Slot-MARKER command.
//
//*****************************************************************************

uint8_t ISO14443B_sendSlotMarker(uint8_t ui8APn)
{
	uint8_t ui8Offset = 0;
	uint8_t ui8Status = STATUS_FAIL;
	uint8_t ui8LoopCount = 0;

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;					// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;					// Send with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;					// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;					// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x10;					// Length of packet in bytes - lower and broken nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = ui8APn;				// Anticollision Prefix byte, n

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);		// Issue the Slot Marker Command

	g_sTrfStatus = TRF79xxA_waitRxData(10,30);			// 10 millisecond TX timeout, 30 millisecond RX Timeout

	if ((g_sTrfStatus == RX_COMPLETE) && (TRF79xxA_getRxBytesReceived() != 0x00))			// Received PUPI in buffer
	{
		ui8Status = STATUS_SUCCESS;

		for(ui8LoopCount = 1; ui8LoopCount < 5; ui8LoopCount++)
		{
			g_pui8Pupi[ui8LoopCount-1] = g_pui8TrfBuffer[ui8LoopCount];	// Store PUPI into array
		}

		if (g_pui8TrfBuffer[10] & 0x01)
		{
			g_bType4BCompliant = true; 				// Set Type B compliance info
		}
		else
		{
			g_bType4BCompliant = false; 			// Set Type B compliance info
		}

#ifdef ENABLE_HOST
		UART_putNewLine();
		UART_sendCString("ISO14443B PUPI: ");
		UART_putChar('[');
		for(ui8LoopCount = 0; ui8LoopCount < 4; ui8LoopCount++)
		{
			UART_putByte(g_pui8Pupi[ui8LoopCount]);	// Print PUPI to UART
		}
		UART_putChar(']');
		UART_putNewLine();
#endif
	}
	else if (g_sTrfStatus == COLLISION_ERROR)		// Collision occurred
	{
		ui8Status = STATUS_FAIL;
	}
	else
	{
		ui8Status = STATUS_FAIL;
	}

	return ui8Status;
}

//*****************************************************************************
//
//! ISO14443B_sendAttrib - Issue the ATTRIB command.
//!
//! This function issues the ISO14443B ATTRIB command using the globally stored
//! PUPI in order to select and activate a specific ISO14443B tag.
//!
//! \return ui8Status returns either STATUS_SUCCESS or STATUS_FAIL
//! to indicate if the Attrib command was successful or not.
//
//*****************************************************************************

uint8_t ISO14443B_sendAttrib(void)
{
	uint8_t ui8Offset = 0;
	uint8_t ui8Status = STATUS_FAIL;

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;					// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;					// Send with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;					// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;					// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x90;					// Length of packet in bytes - lower and broken nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = ATTRIB_CMD;			// ATTRIB start byte
	g_pui8TrfBuffer[ui8Offset++] = g_pui8Pupi[0];		// PUPI
	g_pui8TrfBuffer[ui8Offset++] = g_pui8Pupi[1];		// PUPI
	g_pui8TrfBuffer[ui8Offset++] = g_pui8Pupi[2];		// PUPI
	g_pui8TrfBuffer[ui8Offset++] = g_pui8Pupi[3];		// PUPI
	g_pui8TrfBuffer[ui8Offset++] = 0x00;					// Param1 - Timing parameters, EOF support, SOF support (See ISO14443-3, ATTRIB command)
	g_pui8TrfBuffer[ui8Offset++] = 0x05;					// Param2 - 106kbps data rate and 64 maximum bytes send per transmission (See ISO14443-3, ATTRIB command)
	if (g_bType4BCompliant == true)
	{
		g_pui8TrfBuffer[ui8Offset++] = 0x01;				// Param3 - Type 4B Compliance (See ISO14443-3, ATTRIB command)
	}
	else
	{
		g_pui8TrfBuffer[ui8Offset++] = 0x00;				// Param3 - Not Type 4B Compliant (See ISO14443-3, ATTRIB command)
	}
	g_pui8TrfBuffer[ui8Offset++] = 0x00;					// Param4 - CID and RFU bits (See ISO14443-3, ATTRIB command)

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);		// Issue the ATTRIB Command

	g_sTrfStatus = TRF79xxA_waitRxData(10,30);			// 10 millisecond TX timeout, 30 millisecond RX Timeout

	if(g_sTrfStatus == RX_COMPLETE)						// Received Answer to ATTRIB in buffer
	{
		ui8Status = STATUS_SUCCESS;
#ifdef ENABLE_HOST
		UART_sendCString("Layer 4 Activated");
		UART_putNewLine();
#endif
	}
	else
	{
		ui8Status = STATUS_FAIL;
	}

	return ui8Status;
}


//*****************************************************************************
//
//! ISO14443B_sendHalt - Issue the HLTB command.
//!
//! This function issues the ISO14443B HLTB command using the globally stored
//! PUPI in order to place a specific ISO14443B tag into the Halt state.
//! While in a Halt state, a tag will not respond to any ISO14443B command
//! EXCEPT for the WUPB (Wake-Up) command.
//!
//! \return ui8Status returns either STATUS_SUCCESS or STATUS_FAIL
//! to indicate if the HLTB command was successful or not.
//
//*****************************************************************************

uint8_t ISO14443B_sendHalt(void)
{
	uint8_t ui8Offset = 0;

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;					// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;					// Send with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;					// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;					// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x50;					// Length of packet in bytes - lower and broken nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = HLTB_CMD;				// Halt Command
	g_pui8TrfBuffer[ui8Offset++] = g_pui8Pupi[0];		// Send out PUPI to Halt the correct tag
	g_pui8TrfBuffer[ui8Offset++] = g_pui8Pupi[1];
	g_pui8TrfBuffer[ui8Offset++] = g_pui8Pupi[2];
	g_pui8TrfBuffer[ui8Offset++] = g_pui8Pupi[3];

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);		// Issue the Halt Command

	g_sTrfStatus = TRF79xxA_waitRxData(10,30);			// 10 millisecond TX timeout, 30 millisecond RX Timeout

	if (g_sTrfStatus != NO_RESPONSE_RECEIVED)			// If PICC gives a response to the command, this means the Halt command failed or had an error
	{
		g_sTrfStatus = PROTOCOL_ERROR;
		TRF79xxA_setTrfStatus(g_sTrfStatus);
#ifdef ENABLE_HOST
		UART_sendCString("ISO14443B HALT Error");
		UART_putNewLine();
#endif
		return STATUS_FAIL;
	}
	else
	{
		return STATUS_SUCCESS;
	}
}

//*****************************************************************************
//
//! ISO14443B_getType4BCompliance - Fetches g_bType4BCompliant value
//!
//! This function allows for higher layers of the firmware to determine if a
//! tag is NFC Forum Type 4B compliant.
//!
//! \return g_bType4BCompliant returns whether a tag is Type 4B compliant
//! or not.
//
//*****************************************************************************

bool ISO14443B_getType4BCompliance(void)
{
	return g_bType4BCompliant;
}

//*****************************************************************************
//
//! ISO14443B_getPUPI - Fetches the ISO14443B PUPI.
//!
//! This function allows for higher layers to fetch the PUPI of an ISO14443B
//! tag. In the current implementation, the PUPI stored is from the most recent
//! tag to reply to a SENSB/WUPB request.
//!
//! \return g_pui8Pupi returns the currently stored PUPI.
//
//*****************************************************************************

uint8_t * ISO14443B_getPUPI(void)
{
	return g_pui8Pupi;
}

//*****************************************************************************
//
//! Iso14443b_Initialize - Initialize all Global Variables for the ISO14443B
//! layer.
//!
//! This function will initialize all the global variables for the ISO14443B
//! layer with the appropriate starting values and clear the buffers.
//!
//! \return None
//
//*****************************************************************************

void ISO14443B_init(void)
{
	uint8_t ui8LoopCount;

	g_bType4BCompliant = false;

	for (ui8LoopCount = 0; ui8LoopCount < 4; ui8LoopCount++)
	{
		g_pui8Pupi[ui8LoopCount] = 0xFF;
	}
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
