/*
 * File Name: iso14443a.c
 *
 * Description: ISO14443A Specific Functions
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

#include "iso14443a.h"

//*****************************************************************************
//
//! \addtogroup iso14443a_api ISO14443A Read/Write API's
//! @{
//!
//! A ISO14443A Reader/Writer issues commands based on the ISO14443-3 and -4
//! specifications. The ISO14443 specification is for Proximity Integrated
//! Circuit Cards (PICCs) and communication with PICCs that are NFC Forum
//! Type 4A Tag Platform compliant can occur at bitrates of 106, 212, 424, or
//! 848 kbps.
//!
//! For more information on ISO14443A Readers and Tags please read the
//! ISO14443-3 and ISO14443-4 Specifications.
//
//*****************************************************************************

//*****************************************************************************
//		Global Variables
//*****************************************************************************

extern uint8_t g_pui8TrfBuffer[NFC_FIFO_SIZE];
static volatile tTrfStatus g_sTrfStatus;

static tISO14443A_UidSize g_sUidSize;
static uint8_t g_pui8CompleteUid[10];
static uint8_t g_pui8PartialUid[5];
static uint8_t g_ui8UidPos;
static uint8_t g_ui8ValidUidByteCount;
static uint8_t g_ui8ValidUidBitCount;
static uint8_t g_ui8ValidBits;

static uint8_t g_ui8RecursionCount;
static uint8_t g_ui8MaxRecurviseCalls;

static uint8_t g_ui8Iso14443aSAK;
static bool g_bType4ACompliant;
static uint8_t g_ui8AtsSupportedBitrates; // This is used to store the ATS reply for TA(1) which contains the Tags supported bitrates - needed to determine PPS request parameters.

//*****************************************************************************
//
//! ISO14443A_selectTag - Process to detect and select ISO14443A/NFC
//! Type 2/4A Tag Platform compliant tags.
//!
//! \param ui8Command is the Polling command to be issued by the TRF79xxA for
//! ISO14443A tag detection.
//!
//! This function issues polling command, processes the ATQA, and then handles
//! sending the correct anticollision and selection commands supporting up to
//! the maximum of three Cascade levels.
//!
//! When a collision occurs, this function will call the anticollision
//! function in order to attempt to resolve the collision.
//!
//! \return ui8Status returns whether or not an ISO14443A
//! compliant tag has been successfully selected.
//
//*****************************************************************************

uint8_t ISO14443A_selectTag(uint8_t ui8Command)
{
	uint8_t ui8Index = 0;
	uint8_t ui8LoopCount = 0;
	uint8_t ui8Status = STATUS_FAIL;
	bool bSendCT = false;
	tISO14443A_UidStatus sUidProgress = CASCADE1;
	tCollisionStatus sCollisionStatus = NO_COLLISION;

	// Clear Bit 1 in Special Functions Register to enable anticollision framing
	TRF79xxA_writeRegister(TRF79XXA_SPECIAL_FUNCTION_1,0x00);

	// Clear UID to store new one
	for(ui8Index = 0; ui8Index < 10; ui8Index++)
	{
		g_pui8CompleteUid[ui8Index] = 0x00;
	}
	// Clear partial UID buffer
	for (ui8LoopCount = 0; ui8LoopCount < 5; ui8LoopCount++)
	{
		g_pui8PartialUid[ui8LoopCount] = 0x00;
	}

	g_ui8UidPos = 0;				// Reset UID Position Marker
	g_ui8ValidUidByteCount = 0;		// Reset Valid Bytes Received Counter
	g_ui8ValidUidBitCount = 0; 		// Reset Valid Bits Received Counter
	g_ui8Iso14443aSAK = 0;			// Reset the SAK
	g_bType4ACompliant = false; 	// Reset Type 4A Compliance
	g_ui8AtsSupportedBitrates = 0;	// Reset the ATS Reply for TA(1)
	g_ui8ValidBits = 0; 			// Clear Valid Bit global

	// Poll for a ISO14443A tag
	if (ISO14443A_sendPollCmd(ui8Command))
	{
		if (g_sTrfStatus == RX_COMPLETE)
		{
			// Check ATQA Response for UID size
			if ((g_pui8TrfBuffer[0] & 0xC0) == 0x00)
			{
				g_sUidSize = ISO14443A_UID_SINGLE;
			}
			else if ((g_pui8TrfBuffer[0] & 0xC0) == 0x40)
			{
				g_sUidSize = ISO14443A_UID_DOUBLE;
			}
			else if ((g_pui8TrfBuffer[0] & 0xC0) == 0x80)
			{
				g_sUidSize = ISO14443A_UID_TRIPLE;
			}
			else
			{
				g_sUidSize = ISO14443A_UID_UNKNOWN;
			}
		}
		else
		{
			// Collision occurred, UID size not known
			g_sUidSize = ISO14443A_UID_UNKNOWN;
		}
	}
	else
	{
		// No response to polling command, exit function
		ui8Status = STATUS_FAIL;
		g_ui8RecursionCount = 0; // Reset the recursion count for the anticollision loops
		return ui8Status;
	}

	while (sUidProgress != UID_COMPLETE)
	{
		sCollisionStatus = ISO14443A_sendAnticollisionCmd(sUidProgress, NVB_INIT, &g_pui8CompleteUid[0]);	// Call anticollision loop function

		// Process the response
		if (sCollisionStatus == NO_COLLISION)
		{
			// Store the UID and keep track if the CT byte needs to be sent
			bSendCT = ISO14443A_storeUID(sUidProgress,&g_pui8TrfBuffer[0]);

			// Issue Select command
			if (ISO14443A_sendSelectCmd(sUidProgress,&g_pui8CompleteUid[g_ui8UidPos],bSendCT)) 	// Issue the Select Command
			{
				// If successful, use SAK information to determine if the UID is complete
				if ((g_ui8Iso14443aSAK & BIT2) == 0x00)
				{
					// UID complete, set status to success
					ui8Status = STATUS_SUCCESS;
					sUidProgress = UID_COMPLETE;

					if (g_sUidSize == ISO14443A_UID_UNKNOWN)
					{
						if (sUidProgress == CASCADE1)
						{
							g_sUidSize = ISO14443A_UID_SINGLE;
						}
						else if (sUidProgress == CASCADE2)
						{
							g_sUidSize = ISO14443A_UID_DOUBLE;
						}
						else if (sUidProgress == CASCADE3)
						{
							g_sUidSize = ISO14443A_UID_TRIPLE;
						}
					}
				}
				else
				{
					// UID is not Complete, increase cascade level, update UidSize as well
					if (sUidProgress == CASCADE1)
					{
						sUidProgress = CASCADE2;
						if (g_sUidSize == ISO14443A_UID_UNKNOWN)
						{
							g_sUidSize = ISO14443A_UID_DOUBLE;
						}
					}
					else if (sUidProgress == CASCADE2)
					{
						sUidProgress = CASCADE3;
						if (g_sUidSize == ISO14443A_UID_UNKNOWN)
						{
							g_sUidSize = ISO14443A_UID_TRIPLE;
						}
					}
					else
					{
						// Either Cascade was already CASCADE3 or an error occured, so break to hit re-try loop
						sUidProgress = UID_INCOMPLETE;
						break;
					}

				}
			}
			else
			{
				// Break to hit the re-try loop
				sUidProgress = UID_INCOMPLETE;
				break;
			}
		}
		else if (sCollisionStatus == COLLISION)
		{
			// If a collision occurs, call the Anticollision loop to handle tag collisions
			sCollisionStatus = ISO14443A_runAnticollision(sUidProgress);

			// Check if the anticollision loop is successful
			if (sCollisionStatus == NO_COLLISION)
			{
				// If successful, use SAK information to determine if the UID is complete
				if ((g_ui8Iso14443aSAK & BIT2) == 0x00)
				{
					// UID complete, set status to success
					ui8Status = STATUS_SUCCESS;
					sUidProgress = UID_COMPLETE;
				}
				else
				{
					// UID is not Complete, increase cascade level, update UidSize as well
					if (sUidProgress == CASCADE1)
					{
						sUidProgress = CASCADE2;
					}
					else if (sUidProgress == CASCADE2)
					{
						sUidProgress = CASCADE3;
					}
					else
					{
						// Either Cascade was already CASCADE3 or an error occured, so break to hit re-try loop
						sUidProgress = UID_INCOMPLETE;
						break;
					}
				}
			}
			else
			{
				break;
			}
		}
		else
		{
			// Other error occurred, do not proceed
			sUidProgress = UID_INCOMPLETE;
			g_ui8RecursionCount = 0; // Reset the recursion count for the anticollision loops
			return ui8Status;
		}
	}

	if (sUidProgress == UID_INCOMPLETE)
	{	// Some error occurred, attempt to find the tag again

		if (g_ui8RecursionCount < g_ui8MaxRecurviseCalls)
		{
			g_ui8RecursionCount++;
			ui8Status = ISO14443A_selectTag(ui8Command);
		}
		else
		{
			g_ui8RecursionCount = 0; // Reset the recursion count for the anticollision loops
			return ui8Status;
		}
	}

	// This won't repetively trigger after the recursive call of ISO14443A_selectTag since the sUidProgress will not change
	if (sUidProgress == UID_COMPLETE)
	{
		// Set Bit 1 in Special Functions Register to 1
		TRF79xxA_writeRegister(TRF79XXA_SPECIAL_FUNCTION_1,0x02);

		LED_14443A_ON;
#ifdef ENABLE_HOST
		// UID Completed
		UART_putNewLine();
		UART_sendCString("Anticollison Completed");
		UART_putNewLine();

		// Output UID to UART Terminal
		UART_sendCString("ISO14443A UID:  ");
		UART_putChar('[');
		if (g_sUidSize == ISO14443A_UID_UNKNOWN)	// Assume ID is a single if it has not been defined to this point.
		{
			g_sUidSize = ISO14443A_UID_SINGLE;
		}
		for (ui8LoopCount=0; ui8LoopCount<g_sUidSize; ui8LoopCount++)
		{
			UART_putByte(g_pui8CompleteUid[ui8LoopCount]);
		}
		UART_putChar(']');
		UART_putNewLine();
#endif
		// Output compliance to ISO14443-4
		if (g_ui8Iso14443aSAK & BIT5)
		{
			g_bType4ACompliant = true;
#ifdef ENABLE_HOST
			UART_sendCString("Tag is ISO14443-4 Compliant");
			UART_putNewLine();
#endif
		}
		else
		{
			g_bType4ACompliant = false;
#ifdef ENABLE_HOST
			UART_sendCString("Tag is not ISO14443-4 Compliant");
			UART_putNewLine();
#endif
		}
#ifdef ENABLE_HOST
		UART_putNewLine();
#endif
	}

	g_ui8RecursionCount = 0; // Reset the recursion count for the anticollision loops after anticollision is finished
	return ui8Status;
}

//*****************************************************************************
//
//! ISO14443A_runAnticollision - Process the anticollision routine for
//! ISO14443A to attempt to resolve collisions so a single tag can be detected.
//!
//! \param sCascade is the current anticollision cascade.
//!
//! This function handles the ISO14443A anticollision procedures including
//! dealing with receiving broken bytes and issuing anticollision commands
//! based on those broken bytes. It will run until either a complete UID is
//! received, the maximum number of iterations is reached, or an error occurs.
//!
//! Since this function uses a recursive call for the anticollision process,
//! a limit of maximum iterations (g_ui8MaxRecurviseCalls) has to be
//! established in order to prevent stack overflows due to recursion.
//!
//! \return sStatus returns a status based on what tag response is
//! received following the most recently issued Anticollision
//! command.
//
//*****************************************************************************

tCollisionStatus ISO14443A_runAnticollision(tISO14443A_UidStatus sCascade)
{
	uint8_t ui8NVB = NVB_INIT;
	uint8_t	ui8NVBytes = 0;
	uint8_t	ui8NVBits = 0;
	uint8_t	ui8NVBitCount = 0;
	uint8_t	ui8CollisionPosition = 0;
	uint8_t	ui8LoopCount = 0;
	tCollisionStatus sStatus = COLLISION;

	// Note: The g_ui8UidPos will be used differently in this function in that it will track where to place received
	// valid UID bytes rather than mark the location of the first byte as it does in all other functions.
	// When a full UID is received, the ISO14443A_storeUID function will be called which will restore the g_ui8UidPos
	// value to what is expected by the rest of the firmware. If a microcontroller with larger RAM reserves is used,
	// a seperate global variable could be used instead.

	ui8CollisionPosition = TRF79xxA_getCollisionPosition();		// Get the collision position information from the TRF driver

	ui8NVBytes = (ui8CollisionPosition >> 4) - 2;	// This represents the number of known valid bytes of the UID
	ui8NVBitCount = ui8CollisionPosition & 0x07;	// This represents the number of known valid bits of the UID (can't be more than 8 or else it would be a valid byte)

	g_ui8ValidUidBitCount = ui8NVBitCount;			// Set the valid bit count to be equal to the received value from the TRF

	// Use the number of bits received to generate the value of the bits received so far
	for(ui8LoopCount = 0; ui8LoopCount < ui8NVBitCount; ui8LoopCount++)
	{
		ui8NVBits = (ui8NVBits << 1) + 1;			// Store the info for the valid bits
	}

	if (g_ui8ValidUidByteCount < ui8NVBytes)
	{
		if ((ui8NVBytes-g_ui8ValidUidByteCount) > 5)
		{
			sStatus = COLLISION_FAIL;
			return sStatus;
		}

		// Store the received bytes of the UID in a storage buffer
		for (ui8LoopCount = 0; ui8LoopCount < (ui8NVBytes-g_ui8ValidUidByteCount); ui8LoopCount++)
		{
			g_pui8PartialUid[ui8LoopCount+g_ui8UidPos] |= g_pui8TrfBuffer[ui8LoopCount];
		}

		g_ui8UidPos = ui8LoopCount+g_ui8UidPos;		// Set the UID Position indicator to the next array index

		if (g_ui8UidPos > 4)
		{
			sStatus = COLLISION_FAIL;
			return sStatus;
		}

		g_ui8ValidUidByteCount = ui8NVBytes;		// Set the Valid byte count equal to what was received by the TRF

		// Store the received bits of the UID in a storage buffer
				// "The valid bits shall be part of the UID CLn that was received before a collision occurred
				// followed by a (0)b or (1)b, decided by the PCD. A typical implementation adds a (1)b."
		if (g_ui8ValidUidBitCount < 7)
		{
			// Since the valid bits are not at the maximum amount of bits allowed, add the extra bit at the end of the UID
			g_pui8PartialUid[g_ui8UidPos] = ((g_pui8TrfBuffer[g_ui8ValidUidByteCount] & ui8NVBits));
			g_ui8ValidBits = g_pui8PartialUid[g_ui8UidPos];		// Save the current valid bits in a variable

			// NVB is equivalent to the value received in the TRF79xxA Collision Position Register plus one for the extra bit added per the standard.
			ui8NVB = ui8CollisionPosition+1; 		// "The PCD shall assign NVB with a value that specifies the number of valid bits of UID CLn."
			g_ui8ValidUidBitCount++;				// Increment the valid bit count by one to mark the extra bit which was added per specifications.
		}
		else
		{
			g_pui8PartialUid[g_ui8UidPos] = (g_pui8TrfBuffer[g_ui8ValidUidByteCount] & ui8NVBits);
			g_ui8ValidBits = g_pui8PartialUid[g_ui8UidPos];		// Save the current valid bits in a variable

			// NVB is equivalent to the value received in the TRF79xxA Collision Position Register plus one for the extra bit added per the standard.
			ui8NVB = ui8CollisionPosition; 			// "The PCD shall assign NVB with a value that specifies the number of valid bits of UID CLn."
		}
	}
	else
	{
		// Update the valid bits based on the current valid bits as well as the newly received valid bits from the tag response
				// "The valid bits shall be part of the UID CLn that was received before a collision occurred
				// followed by a (0)b or (1)b, decided by the PCD. A typical implementation adds a (1)b."
		if (g_ui8ValidUidBitCount < 7)
		{
			// Since the valid bits are not at the maximum amount of bits allowed, add the extra bit at the end of the UID
			g_pui8PartialUid[g_ui8UidPos] = (g_ui8ValidBits | ((g_pui8TrfBuffer[0] & ui8NVBits) << (g_ui8ValidUidBitCount-ui8NVBitCount)));
			g_ui8ValidBits = g_pui8PartialUid[g_ui8UidPos];		// Save the current valid bits in a variable

			ui8NVB = ui8CollisionPosition+1; 		// "The PCD shall assign NVB with a value that specifies the number of valid bits of UID CLn."
			g_ui8ValidUidBitCount++;				// Increment the valid bit count by one to mark the extra bit which was added per specifications.
		}
		else
		{
			g_pui8PartialUid[g_ui8UidPos] = (g_ui8ValidBits | ((g_pui8TrfBuffer[0] & ui8NVBits) << (g_ui8ValidUidBitCount-ui8NVBitCount)));
			g_ui8ValidBits = g_pui8PartialUid[g_ui8UidPos];		// Save the current valid bits in a variable

			ui8NVB = ui8CollisionPosition; 			// "The PCD shall assign NVB with a value that specifies the number of valid bits of UID CLn."
		}
	}

	MCU_delayMillisecond(1);							// Small delay prior to sending out packet.

	sStatus = ISO14443A_sendAnticollisionCmd(sCascade,ui8NVB,&g_pui8PartialUid[0]);				// Issue anti-collision command with the partial UID

	if (sStatus == NO_COLLISION)
	{
		// No Collision means the anticollision command was successful and the remaining bytes were received

		g_pui8PartialUid[g_ui8UidPos] = g_pui8PartialUid[g_ui8UidPos] | g_pui8TrfBuffer[0]; 		// Combine broken byte with 1st received byte to finalize the 1st byte of the UID
		g_ui8UidPos++;								// Increment the UID Position Indicator
		g_ui8ValidUidBitCount = 0;					// Reset the valid bit counter
		g_ui8ValidBits = 0;							// Reset the valid bit variable

		// Store the other bytes
		for (ui8LoopCount = 0; ui8LoopCount < (5-g_ui8UidPos); ui8LoopCount++)
		{
			g_pui8PartialUid[ui8LoopCount+g_ui8UidPos] = g_pui8TrfBuffer[(ui8LoopCount+1)];	// Store remaining received UID bytes into the partial UID buffer
		}

		// Issue the Select Command with the fully received UID
		if (ISO14443A_sendSelectCmd(sCascade,&g_pui8PartialUid[0],false))
		{
			// Received the SAK from the Select Command (This is stored in a global within the Select function)

			// If the UID Size is not known yet
			if (g_sUidSize == ISO14443A_UID_UNKNOWN)
			{
				// Use SAK information to determine if the UID is complete
				if ((g_ui8Iso14443aSAK & BIT2) == 0x00)
				{
					if (sCascade == CASCADE1)
					{
						g_sUidSize = ISO14443A_UID_SINGLE;
					}
					else if (sCascade == CASCADE2)
					{
						g_sUidSize = ISO14443A_UID_DOUBLE;
					}
					else if (sCascade == CASCADE3)
					{
						g_sUidSize = ISO14443A_UID_TRIPLE;
					}
				}
				else
				{
					// UID is not complete, update UidSize as well
					if (sCascade == CASCADE1)
					{
						g_sUidSize = ISO14443A_UID_DOUBLE;
					}
					else if (sCascade == CASCADE2)
					{
						g_sUidSize = ISO14443A_UID_TRIPLE;
					}
				}
			}

			// Store the partial UID into the global UID buffer
			ISO14443A_storeUID(sCascade,&g_pui8PartialUid[0]);

			// Clear partial UID buffer in order to handle future collisions
			for (ui8LoopCount = 0; ui8LoopCount < 5; ui8LoopCount++)
			{
				g_pui8PartialUid[ui8LoopCount] = 0x00;
			}

			// Set status to NO_COLLISION and exit function
			sStatus = NO_COLLISION;
			return sStatus;
		}
		else
		{
			sStatus = COLLISION_FAIL;
		}
	}
	else if (sStatus == COLLISION)
	{
		// If a collision occurred, check the Recursion Counter and then call Anticollision Loop function again if the condition is met
		if (g_ui8RecursionCount < g_ui8MaxRecurviseCalls)
		{
			g_ui8RecursionCount++;	// Increment Recursion Counter
			sStatus = ISO14443A_runAnticollision(sCascade);	// Recursive call of Anticollision Loop
		}
		else
		{
			sStatus = COLLISION_FAIL;
		}
	}
	else
	{
		// For all other statuses, return the status that was received
	}

	return sStatus;
}


//*****************************************************************************
//
//! ISO14443A_sendPollCmd - Issue a polling command for ISO14443A compliant
//! tags.
//!
//! \param ui8Command is the polling command to be issued.
//!
//! This function sends the Polling command based on the inputted
//! command (either REQA or WUPA).
//!
//! \return ui8Status returns whether or not an ISO14443A compliant tag has
//! responded to the Polling command.
//
//*****************************************************************************

uint8_t ISO14443A_sendPollCmd(uint8_t ui8Command)
{
	uint8_t ui8Offset = 0;
	uint8_t ui8Status = STATUS_FAIL;

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;				// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x90;				// Send without CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;				// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;				// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x0F;				// Length of packet in bytes - lower and broken nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = ui8Command;		// Send the polling command from function input - either REQA (0x26) or WUPA (0x52)

    TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);	// Issue the ISO14443A Polling Command

    g_sTrfStatus = TRF79xxA_waitRxData(3,10);				// 3 millisecond TX timeout, 10 millisecond RX timeout

	if (g_sTrfStatus == RX_COMPLETE)	// Tag detected - could be either a single or collided tag
	{
		ui8Status = STATUS_SUCCESS;
	}
	else if (g_sTrfStatus == COLLISION_ERROR)
	{
		ui8Status = STATUS_SUCCESS;		// "A PCD detecting a collision in any bit of (b16 to b1) shall commence with the first step of the anticollision loop."
	}

	return ui8Status;
}


//*****************************************************************************
//
//! ISO14443A_sendAnticollisionCmd - Issue Anticollsion command for ISO14443A
//! compliant tags.
//!
//! \param sCascade is the current anticollision cascade
//! \param ui8NVB is the NVB value for the UID bytes and bits
//! \param pui8UID is the location of the UID bytes/bits to send.
//!
//! This function sends the Anticollision command based on the provided
//! cascade level.
//!
//! \return sStatus returns a status based on what tag response is received
//! following the Anticollision command.
//
//*****************************************************************************

tCollisionStatus ISO14443A_sendAnticollisionCmd(tISO14443A_UidStatus sCascade, uint8_t ui8NVB, uint8_t * pui8UID)
{
	uint8_t ui8Offset = 0;
	uint8_t ui8LoopCount = 0;
	uint8_t ui8UidLength = 0;
	uint8_t ui8RxLength = 0;
	uint8_t ui8Select = SEL_CASCADE1;
	tCollisionStatus sStatus = NO_COLLISION;

	if (sCascade == CASCADE1)
	{
		ui8Select = SEL_CASCADE1;
	}
	else if (sCascade == CASCADE2)
	{
		ui8Select = SEL_CASCADE2;
	}
	else if (sCascade == CASCADE3)
	{
		ui8Select = SEL_CASCADE3;
	}
	else
	{
		return sStatus = COLLISION_FAIL;
	}

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;		// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x90;		// Transmit without CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;		// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;		// Length of packet in bytes - upper and middle nibbles of transmit byte length
	if((ui8NVB & 0x07) != 0x00)				// Length of packet in bytes - lower nibble and broken bits of transmit byte length
	{
		g_pui8TrfBuffer[ui8Offset++] = (ui8NVB & 0xF0) | (((ui8NVB & 0x07) << 1) + 1); 	// Set the number of broken bits, last bit is 1 means broken byte
	}
	else
	{
		g_pui8TrfBuffer[ui8Offset++] = ui8NVB & 0xF0;	// No broken bits
	}
	g_pui8TrfBuffer[ui8Offset++] = ui8Select;			// Select Command; can be 0x93, 0x95 or 0x97
	g_pui8TrfBuffer[ui8Offset++] = ui8NVB;				// Number of valid bits

	ui8UidLength = (ui8NVB >> 4) - 2;
	if ((ui8NVB & 0x0F) != 0x00)
	{
		ui8UidLength++;
	}

	for (ui8LoopCount = 0; ui8LoopCount < ui8UidLength; ui8LoopCount++)
	{
		g_pui8TrfBuffer[ui8Offset++] = pui8UID[ui8LoopCount];	// UID Bytes
	}

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);		// Issue the Select Command

	g_sTrfStatus = TRF79xxA_waitRxData(5,3);		// 5 millisecond TX timeout, 3 millisecond RX timeout

    if (g_sTrfStatus == RX_COMPLETE)
    {
    	ui8RxLength = TRF79xxA_getRxBytesReceived();

    	if (ui8RxLength > 1)
    	{
			sStatus = NO_COLLISION;
    	}
    	else
    	{
    		sStatus = COLLISION_FAIL;
    	}
    }
    else if (g_sTrfStatus == COLLISION_ERROR)
    {
    	sStatus = COLLISION;
    }
    else if (g_sTrfStatus == NO_RESPONSE_RECEIVED)
    {
    	sStatus = NO_RESPONSE;
    }
    else
    {
    	// Do nothing
    	sStatus = COLLISION_FAIL;
    }

    return sStatus;
}


//*****************************************************************************
//
//! ISO14443A_sendSelectCmd - Issue the Select command for ISO14443A
//! compliant tags.
//!
//! \param sCascade is the current anticollision cascade.
//! \param pui8UID is the location of the UID bytes to send.
//! \param bSendCT determines if the CT byte must be sent.
//!
//! This function issues the Select command based on the current cascade level.
//!
//! \return ui8Status returns whether or not an ISO14443A compliant tag has
//! responded to the Select command.
//
//*****************************************************************************

uint8_t ISO14443A_sendSelectCmd(tISO14443A_UidStatus sCascade, uint8_t * pui8UID, bool bSendCT)
{
	uint8_t ui8Offset = 0;
	uint8_t ui8Status = STATUS_FAIL;
	uint8_t ui8Select = SEL_CASCADE1;

	if (sCascade == CASCADE1)
	{
		ui8Select = SEL_CASCADE1;
	}
	else if (sCascade == CASCADE2)
	{
		ui8Select = SEL_CASCADE2;
	}
	else if (sCascade == CASCADE3)
	{
		ui8Select = SEL_CASCADE3;
	}
	else
	{
		return ui8Status = STATUS_FAIL;
	}

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;				// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;				// Transmit with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;				// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;				// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x70;				// Length of packet in bytes - lower and broken nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = ui8Select;		// Select Command; can be 0x93, 0x95 or 0x97
	g_pui8TrfBuffer[ui8Offset++] = NVB_FULL;			// Number of valid bits
	if (bSendCT == true)
	{
		g_pui8TrfBuffer[ui8Offset++] = 0x88;			// CT
		g_pui8TrfBuffer[ui8Offset++] = *pui8UID;		// UID Bytes
		g_pui8TrfBuffer[ui8Offset++] = *(pui8UID + 1);
		g_pui8TrfBuffer[ui8Offset++] = *(pui8UID + 2);
		g_pui8TrfBuffer[ui8Offset++] = ( 0x88 ^ *pui8UID ^ *(pui8UID + 1) ^ *(pui8UID + 2) );	// Calculate BCC Byte
	}
	else
	{
		g_pui8TrfBuffer[ui8Offset++] = *pui8UID;		// UID Bytes
		g_pui8TrfBuffer[ui8Offset++] = *(pui8UID + 1);
		g_pui8TrfBuffer[ui8Offset++] = *(pui8UID + 2);
		g_pui8TrfBuffer[ui8Offset++] = *(pui8UID + 3);
		g_pui8TrfBuffer[ui8Offset++] = ( *pui8UID ^ *(pui8UID + 1) ^ *(pui8UID + 2) ^ *(pui8UID + 3) );	// Calculate BCC Byte
	}

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);	// Issue the Select Command

	g_sTrfStatus = TRF79xxA_waitRxData(5,15);					// 5 millisecond TX timeout, 15 millisecond RX timeout

    if (g_sTrfStatus == RX_COMPLETE)
    {
    	ui8Status = STATUS_SUCCESS;

   		g_ui8Iso14443aSAK = g_pui8TrfBuffer[0];
    }
    else
    {
    	// Do nothing
    }

    return ui8Status;
}


//*****************************************************************************
//
//! ISO14443A_sendHalt - Issue the Halt command to the currently selected
//! ISO14443A compliant tag.
//!
//! This function sends the HALT command to the currently selected ISO14443A
//! compliant tag. When a tag has been placed in a Halt State, then it will
//! no longer reply to any issued commands and it can only be woken by either
//! using the WUPA command or by removing it from the RF field of the
//! Reader/Writer device. A tag should never reply to the HALT command.
//!
//! \return ui8Status returns STATUS_FAIL if the tag erroneously responded
//! to the Halt command.
//
//*****************************************************************************

uint8_t ISO14443A_sendHalt(void)
{
	uint8_t ui8Offset = 0;

	// Ensure we can receive any possible response to treat it as an error case
	if (TRF79xxA_getIsoControlValue() != 0x88)
	{
		// Trf797x has not been properly configured for ISO14443A
		TRF79xxA_writeRegister(TRF79XXA_ISO_CONTROL,0x88);			// Configure the TRF79xxA for ISO14443A @ 106kbps and Receive without CRC
	}

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;		// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x90;		// Send without CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;		// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;		// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x20;		// Length of packet in bytes - lower and broken nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x50;		// Halt Command
	g_pui8TrfBuffer[ui8Offset++] = 0x00;

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);		// Issue the Halt Command

	g_sTrfStatus = TRF79xxA_waitRxData(3,5);		// 3 millisecond TX timeout, 5 millisecond RX timeout

	if (g_sTrfStatus != NO_RESPONSE_RECEIVED)	// If PICC gives a response to the command, this means the Halt command failed or had an error
	{
		g_sTrfStatus = PROTOCOL_ERROR;
		TRF79xxA_setTrfStatus(g_sTrfStatus);
#ifdef ENABLE_HOST
		UART_sendCString("ISO14443A HALT Error");
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
//! ISO14443A_sendRATS - Issue the RATS command to the currently selected
//! ISO14443-4 compliant tag.
//!
//! This function sends the RATS command to activate an ISO14443-4 compliant
//! tag for data exchange.
//!
//! \return ui8Status returns whether or not the selected ISO14443-4 compliant
//! tag responded to the RATS command.
//
//*****************************************************************************

uint8_t ISO14443A_sendRATS(void)
{
	uint8_t ui8Offset = 0;
	uint8_t ui8RxLength = 0;
	uint8_t ui8Status = STATUS_FAIL;
#ifdef ENABLE_HOST
	uint8_t ui8LoopCount = 0;
#endif

	// Buffer setup for FIFO writing
	g_pui8TrfBuffer[ui8Offset++] = 0x8F;		// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;		// Send with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;		// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;		// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x20;		// Length of packet in bytes - lower and broken nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = RATS_CMD;		//RATS Command
	g_pui8TrfBuffer[ui8Offset++] = RATS_PARAM;	//RATS Parameters: 128 byte max receive and CID = 0

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);		// Issue the RATS command

	g_sTrfStatus = TRF79xxA_waitRxData(3,10);	// 3 millisecond TX timeout, 10 millisecond RX timeout

	// If data received
	if(g_sTrfStatus == RX_COMPLETE)
	{
		ui8RxLength = TRF79xxA_getRxBytesReceived();

		if (g_pui8TrfBuffer[0] == ui8RxLength)
		{
			ui8Status = STATUS_SUCCESS;

			if ((ui8RxLength > 1) && (g_pui8TrfBuffer[1] & 0x10))
			{
				g_ui8AtsSupportedBitrates = g_pui8TrfBuffer[ui8Offset++];
			}

#ifdef ENABLE_HOST
			UART_sendCString("ISO14443A ATS: ");
			for (ui8LoopCount = 0; ui8LoopCount < ui8RxLength; ui8LoopCount++)
			{
				UART_putByte(g_pui8TrfBuffer[ui8LoopCount]);
			}
			UART_putNewLine();
#endif
		}
		else
		{
			ui8Status = STATUS_FAIL;
		}
	}
	else
	{
		ui8Status = STATUS_FAIL;
	}
	return ui8Status;
}



//*****************************************************************************
//
//! ISO14443A_sendPPS - Issue the PPS command to the currently selected
//! ISO14443-4 compliant tag.
//!
//! This function sends the PPS command to modify the over the air data rate
//! of the selected ISO14443-4 compliant tag.
//!
//! \return ui8Status returns whether or not the selected ISO14443-4
//! compliant tag responded to the PPS command.
//
//*****************************************************************************

uint8_t ISO14443A_sendPPS(void)
{
	uint8_t ui8Offset = 0;
	uint8_t ui8Status = STATUS_FAIL;
	uint8_t ui8PPSBitrate;

	// Check if PPS is supported based on last received ATS reply
	if ((g_ui8AtsSupportedBitrates == 0x00) || (g_ui8AtsSupportedBitrates == 0x80))
	{
#ifdef ENABLE_HOST
		UART_sendCString("No PPS Support, Bitrate = 106kbps");
#endif

		return ui8Status = STATUS_SUCCESS;
	}

	ui8PPSBitrate = PPS1_106; 				// Set the PPS bit rate to 106kbps for best range performance
											// It is recommended to keep the data rate low to get better transmission ranges

	// Buffer setup for FIFO writing
	g_pui8TrfBuffer[ui8Offset++] = 0x8F;		// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;		// Send with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;		// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;		// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x30;		// Length of packet in bytes - lower and broken nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = PPSS;		// PPS Command
	g_pui8TrfBuffer[ui8Offset++] = PPS0;
	g_pui8TrfBuffer[ui8Offset++] = ui8PPSBitrate;	// Send PPS Bit Rate

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);		// Issue the PPS Command

	g_sTrfStatus = TRF79xxA_waitRxData(3,10);		// 3 millisecond TX timeout, 10 millisecond RX timeout

	// If data received
	if(g_sTrfStatus == RX_COMPLETE)
	{
		// Check PPS response
		if (g_pui8TrfBuffer[0] == PPSS)
		{
			ui8Status = STATUS_SUCCESS;

			// Execute Bitrate Change
			MCU_delayMillisecond(1);

#ifdef ENABLE_HOST
			UART_sendCString("PPS Success, Bitrate = ");
#endif

			switch(ui8PPSBitrate)
			{
				case PPS1_106:
					TRF79xxA_writeRegister(TRF79XXA_ISO_CONTROL,0x08);	// Configure the TRF79xxA for ISO14443A @ 106kbps and Receive with CRC
#ifdef ENABLE_HOST
					UART_sendCString("106kpbs");
#endif
					break;
				case PPS1_212:
					TRF79xxA_writeRegister(TRF79XXA_ISO_CONTROL,0x09);	// Configure the TRF79xxA for ISO14443A @ 212kbps and Receive with CRC
#ifdef ENABLE_HOST
					UART_sendCString("212kbps");
#endif
					break;
				case PPS1_424:
				TRF79xxA_writeRegister(TRF79XXA_ISO_CONTROL,0x0A);	// Configure the TRF79xxA for ISO14443A @ 424kbps and Receive with CRC
#ifdef ENABLE_HOST
				UART_sendCString("424kbps");
#endif
				break;
			case PPS1_848:
				TRF79xxA_writeRegister(TRF79XXA_ISO_CONTROL,0x0B);	// Configure the TRF79xxA for ISO14443A @ 848kbps and Receive with CRC
#ifdef ENABLE_HOST
				UART_sendCString("848kbps");
#endif
				break;
			default:
				// Do Nothing
				break;
			}
			MCU_delayMillisecond(6);				// Guard time after bitrate change
		}
		else
		{
			ui8Status = STATUS_FAIL;
#ifdef ENABLE_HOST
			UART_sendCString("PPS Failed");
#endif
		}
	}
	else
	{
		ui8Status = STATUS_FAIL;
#ifdef ENABLE_HOST
		UART_sendCString("PPS Failed");
#endif
	}

#ifdef ENABLE_HOST
	UART_putNewLine();
	UART_putNewLine();
#endif

	return ui8Status;
}

//*****************************************************************************
//
//! ISO14443A_storeUID - Store the received UID bytes into the global
//! g_pui8CompleteUid buffer.
//!
//! \param sCascade is the current anticollision cascade.
//! \param pui8UID is the location of the UID bytes to store.
//!
//! This function stores the received UID bytes into the global UID buffer.
//! Additionally, it will parse out the CT byte and set a flag to indicate if
//! the CT byte was present.
//!
//! \return bSendCT returns whether or not the next transmission will require
//! the CT byte to be sent in addition to the UID bytes.
//
//*****************************************************************************

bool ISO14443A_storeUID(tISO14443A_UidStatus sCascade, uint8_t * pui8UID)
{
	bool bSendCT = false;
	uint8_t ui8Offset = 0;

	if (((g_sUidSize == ISO14443A_UID_SINGLE) && (sCascade == CASCADE1))
			|| ((g_sUidSize == ISO14443A_UID_DOUBLE) && (sCascade == CASCADE2))
			|| ((g_sUidSize == ISO14443A_UID_TRIPLE) && (sCascade == CASCADE3)))
	{
		// UID has no CT, so store all bytes normally.
		ui8Offset = 0;
		bSendCT = false;
	}
	else
	{
		if (pui8UID[0] == 0x88)
		{
			// UID has a CT, set bool to return that a CT must be sent
			ui8Offset = 1;		// Set offset to account for the location of the CT byte
			bSendCT = true;		// Set variable to tell Select Command to include a CT in addition to UID bytes
		}
		else
		{
			// UID has no CT, so store all bytes normally.
			ui8Offset = 0;
			bSendCT = false;
		}
	}

	// Store UID based on the current Cascade level
	if (sCascade == CASCADE1)
	{
		g_pui8CompleteUid[0] = pui8UID[0+ui8Offset];
		g_pui8CompleteUid[1] = pui8UID[1+ui8Offset];
		g_pui8CompleteUid[2] = pui8UID[2+ui8Offset];
		g_pui8CompleteUid[3] = pui8UID[3+ui8Offset];	// BCC Byte or last byte of UID
		g_ui8UidPos = 0;	// Update the UID Position indicator to the first byte of the newly stored UID for when the Select command is issued
	}
	else if (sCascade == CASCADE2)
	{
		g_pui8CompleteUid[3] = pui8UID[0+ui8Offset];	// Override the BCC from prior Cascade as it is no longer needed
		g_pui8CompleteUid[4] = pui8UID[1+ui8Offset];
		g_pui8CompleteUid[5] = pui8UID[2+ui8Offset];
		g_pui8CompleteUid[6] = pui8UID[3+ui8Offset];	// BCC Byte or last byte of UID
		g_ui8UidPos = 3;	// Update the UID Position indicator to the first byte of the newly stored UID for when the Select command is issued
	}
	else if (sCascade == CASCADE3)
	{
		g_pui8CompleteUid[6] = pui8UID[0];	// Override the BCC from prior Cascade as it is no longer needed
		g_pui8CompleteUid[7] = pui8UID[1];
		g_pui8CompleteUid[8] = pui8UID[2];
		g_pui8CompleteUid[9] = pui8UID[3];
		bSendCT = false; 	// Ensure no accidental sending of the CT occurs incase uid6 for a Triple Size UID = 0x88 (which is permitted per ISO14443-3 specifications)
		g_ui8UidPos = 6;	// Update the UID Position indicator to the first byte of the newly stored UID for when the Select command is issued
	}

	return bSendCT;
}

//*****************************************************************************
//
//! ISO14443A_sendT2TReadFourBlocks - Reads out four blocks of data from NFC
//! Type 2 Tag.
//!
//! \param ui8StartBlock is the starting block number to read the tag data
//!
//! This function will issue a Read Block command with the provided starting
//! block number. The Read Block command will prompt the Type 2 Tag to reply
//! with four blocks of data starting from the block number given.
//!
//! Type 2 Tags use a 4 bit NAK response when the command is recognized, but
//! the TRF79xxA cannot both process the 4 bit response and also still receive
//! block data, so it is left in the mode to receive block data. Therefore if
//! a failure in communication occurs, no NAK responses shall be explicitly
//! received by the device.
//!
//! \return ui8Status returns whether or not the tag data was successfully
//! read.
//
//*****************************************************************************

uint8_t ISO14443A_sendT2TReadFourBlocks(uint8_t ui8StartBlock)
{
	uint8_t ui8Offset = 0;
	uint8_t ui8Status = STATUS_FAIL;
#ifdef ENABLE_HOST
	uint8_t	ui8LoopCount1 = 1;
	uint8_t	ui8LoopCount2 = 0;
#endif

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;		// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;		// Send with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;		// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;		// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x20;		// Length of packet in bytes - lower and broken nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x30;				// Read Command
	g_pui8TrfBuffer[ui8Offset++] = ui8StartBlock;	// Starting from Block # (called Bno)

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);		// Issue the Type 2 Read Command

	g_sTrfStatus = TRF79xxA_waitRxData(10,30);		// 10 millisecond TX timeout, 30 millisecond RX timeout

	if(g_sTrfStatus == RX_COMPLETE)		// If block data has been received
	{
		if (TRF79xxA_getRxBytesReceived() > 0)
		{
			ui8Status = STATUS_SUCCESS;		// Mark tag has been successfully read

#ifdef ENABLE_HOST
			for(ui8LoopCount2 = 0; ui8LoopCount2 < 4; ui8LoopCount2++)
			{
				UART_sendCString("Block ");
				UART_putByte(ui8StartBlock++);
				UART_sendCString(":  [");
				for(ui8LoopCount1 = (ui8LoopCount2*4); ui8LoopCount1 < 4+(ui8LoopCount2*4); ui8LoopCount1++)
				{
					UART_putByte(g_pui8TrfBuffer[ui8LoopCount1]);		// Print out the received data
				}
				UART_putChar(']');
				UART_putNewLine();
			}
#endif
		}
		else
		{
#ifdef ENABLE_HOST
			UART_sendCString("Read Fail");
			UART_putNewLine();
#endif
		}
	}
	else
	{
		// Otherwise return a fail
		ui8Status = STATUS_FAIL;
	}

	return ui8Status;
}

//*****************************************************************************
//
//! ISO14443A_sendT2TWriteSingleBlock - Write a single block of data to an NFC
//! Type 2 Tag.
//!
//! \param ui8StartBlock is the block number to write data to
//! \param pui8TagData is the buffer that has the data to be written
//!
//! This function will issue a Write Block command with the provided block
//! number and tag data. The function will prevent users by default from
//! overwriting the One-Time Programmable (OTP) Blocks on the tag. It also
//! will configure the TRF79xxA to properly receive the 4 bit ACK/NAK reply.
//!
//! \return ui8Status returns whether or not the tag data was
//! successfully written.
//
//*****************************************************************************

uint8_t ISO14443A_sendT2TWriteSingleBlock(uint8_t ui8StartBlock, uint8_t * pui8TagData)
{
	uint8_t ui8Offset = 0;
	uint8_t ui8Status = STATUS_FAIL;
	uint8_t pui8TrfConfig;
	uint8_t ui8TagResp;

	if (ui8StartBlock < 4)
	{
		// Attempt to write OTP blocks, do not allow
		// * This can be removed by experienced users who understand
		//   that they can permanently break a tag by using this incorrectly
		return ui8Status;
	}

	// Read Register 0x10 - Special Functions Register
	pui8TrfConfig = TRF79xxA_readRegister(TRF79XXA_SPECIAL_FUNCTION_1);

	// Set Bit 2 in Special Functions Register to 1
	pui8TrfConfig |= 0x04;
	TRF79xxA_writeRegister(TRF79XXA_SPECIAL_FUNCTION_1,pui8TrfConfig); 	// Turn on 4-bit receive for ACK/NAK replies

	g_pui8TrfBuffer[ui8Offset++] = 0x8F;				// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;				// Send with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;				// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;				// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x60;				// Length of packet in bytes - lower and broken nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0xA2;				// Write Command
	g_pui8TrfBuffer[ui8Offset++] = ui8StartBlock;	// Starting from Block # (called Bno)
	g_pui8TrfBuffer[ui8Offset++] = pui8TagData[0];	// Four tag data bytes
	g_pui8TrfBuffer[ui8Offset++] = pui8TagData[1];
	g_pui8TrfBuffer[ui8Offset++] = pui8TagData[2];
	g_pui8TrfBuffer[ui8Offset++] = pui8TagData[3];

	TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);		// Issue the Type 2 Read Command

	g_sTrfStatus = TRF79xxA_waitRxData(10,30);		// 10 millisecond TX timeout, 30 millisecond RX timeout

	if(g_sTrfStatus == RX_COMPLETE)		// If an acknowledgment is received
	{
		ui8TagResp = g_pui8TrfBuffer[0] >> 4;	// Only use upper four bits received for ACK/NAK check

		if (ui8TagResp == 0xA)
		{
			ui8Status = STATUS_SUCCESS;		// Mark tag has been successfully read
	#ifdef ENABLE_HOST
			UART_sendCString("Write Success");
			UART_putNewLine();
	#endif
		}
		else
		{
#ifdef ENABLE_HOST
		UART_sendCString("Write Fail: ");
		if (ui8TagResp == 0x0)
		{
			UART_sendCString("Invalid Page Address");
		}
		else
		{
			UART_sendCString("NAK = ");
			UART_putChar(ui8TagResp);
		}
		UART_putNewLine();
#endif
		}

	}
	else
	{
#ifdef ENABLE_HOST
		UART_sendCString("Write Fail");
		UART_putNewLine();
#endif
		// Otherwise return a fail
		ui8Status = STATUS_FAIL;
	}

	// Read Register 0x10 - Special Functions Register
	pui8TrfConfig = TRF79xxA_readRegister(TRF79XXA_SPECIAL_FUNCTION_1);

	// Set Bit 2 in Special Functions Register to 0
	pui8TrfConfig &= ~0x04;
	TRF79xxA_writeRegister(TRF79XXA_SPECIAL_FUNCTION_1,pui8TrfConfig); 	// Clear 4-bit receive for ACK/NAK replies


	return ui8Status;
}

//*****************************************************************************
//
//! ISO14443A_getType4ACompliance - Fetches g_bType4ACompliant value
//!
//! This function allows for higher layers to fetch the current Type 4A NDEF
//! compliance information.
//!
//! \return g_bType4ACompliant returns the current Type 4A NDEF compliance.
//
//*****************************************************************************

bool ISO14443A_getType4ACompliance(void)
{
	return g_bType4ACompliant;
}

//*****************************************************************************
//
//! ISO14443A_setRecursionCount - Sets the g_ui8RecursionCount variable
//!
//! \param ui8RecursionCount is the new recursion count variable.
//!
//! This function allows for higher layers to adjust the global recursion
//! count. Useful for resetting it prior to running anticollision routines.
//!
//! \return None.
//!
//*****************************************************************************

void ISO14443A_setRecursionCount(uint8_t ui8RecursionCount)
{
	g_ui8RecursionCount = ui8RecursionCount;
}

//*****************************************************************************
//
//! ISO14443A_getUid - Fetches the ISO14443A tag UID.
//!
//! This function allows for higher layers to fetch the tag UID of an ISO14443A
//! tag. In the current implementation, the UID stored is from the most recent
//! tag which finished the anticollision procedure.
//!
//! \return g_pui8CompleteUid returns a pointer to the currently stored UID.
//!
//*****************************************************************************

uint8_t * ISO14443A_getUid(void)
{
	return g_pui8CompleteUid;
}

//*****************************************************************************
//
//! ISO14443A_getUidSize - Fetches the UID size of the ISO14443A tag.
//!
//! This function allows for higher layers to fetch the size of the currently
//! stored UID for an ISO14443A tag. In the current implementation, the only
//! UID stored is that of the most recent tag read by the firmware.
//!
//! \return g_sUidSize returns the current UID size.
//!
//*****************************************************************************

tISO14443A_UidSize ISO14443A_getUidSize(void)
{
	return g_sUidSize;
}

//*****************************************************************************
//
//! ISO14443A_init - Initialize all Global Variables for the ISO14443A layer.
//!
//! This function will initialize all the global variables for the ISO14443A
//! layer with the appropriate starting values and initialize buffer values.
//!
//! \return None
//
//*****************************************************************************

void ISO14443A_init(void)
{
	uint8_t ui8LoopCount;

	g_sUidSize = ISO14443A_UID_UNKNOWN;
	g_ui8UidPos = 0;
	g_ui8ValidUidByteCount = 0;
	g_ui8ValidUidBitCount = 0;
	g_ui8ValidBits = 0;

	g_ui8RecursionCount = 0;
	g_ui8MaxRecurviseCalls = 5;

	g_ui8Iso14443aSAK = 0xFF;
	g_bType4ACompliant = false;
	g_ui8AtsSupportedBitrates = 0x00;

	for (ui8LoopCount = 0; ui8LoopCount < 10; ui8LoopCount++)
	{
		g_pui8CompleteUid[ui8LoopCount] = 0xFF;
	}
	for (ui8LoopCount = 0; ui8LoopCount < 5; ui8LoopCount++)
	{
		g_pui8PartialUid[ui8LoopCount] = 0x00;
	}
}


//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
