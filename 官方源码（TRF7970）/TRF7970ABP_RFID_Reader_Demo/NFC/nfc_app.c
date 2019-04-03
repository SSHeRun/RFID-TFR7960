/*
 * File Name: nfc_app.c
 *
 * Description: Functions to handle the Application Layer processing of
 * the NFC/RFID stack.
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

#include "nfc_app.h"

//*****************************************************************************
//
//! \addtogroup app_nfc_api Application Specific NFC API's
//! @{
//!
//! Application specific API's for NFC/RFID Tag Detection, Selection, and
//! Reading.
//
//*****************************************************************************

//*****************************************************************************
//
//! NFC_findTag - Simple process to cycle through finding NFC/RFID tags of
//! various NFC/RFID technologies.
//!
//! This function cycles through applications designed to detect, active, and
//! read data from each supported NFC/RFID technology - ISO14443A, ISO14443B,
//! ISO15693, and FeliCa.
//!
//! \return None.
//
//*****************************************************************************

void NFC_findTag(void)
{
	// Clear IRQ Flags
	IRQ_CLR;

	TRF79xxA_reset();			// Soft Reset the TRF79xxA to ensure it is in a good state at the start of each process
								// This is a recommended best practice

#ifdef ENABLE_14443A
	NFC_appIso14443a();			// Scan for NFC Type 4A / ISO14443A tags
								// For NFC Type 4A tags - Attempt to read NDEF contents
								// For NFC Type 2 tags - Read block data from the tag
#endif

#ifdef ENABLE_14443B
	NFC_appIso14443b();			// Scans for NFC Type 4B / ISO14443B tags and attempts to read NDEF contents
#endif

#ifdef ENABLE_FELICA
	NFC_appFeliCa();			// Scans for NFC Type 3 (Sony FeliCa) tags, reports FeliCa card type, gets mfg code, CIN, Pmm, and System code or Data Rate Capability
								// then using IDm, reads single block and then reads four blocks, simple NDEF parsing afterwards implemented
#endif

#ifdef ENABLE_15693
	NFC_appIso15693();			// Scans for NFC Type 5 / ISO15693 tags, reads all data blocks indicated.
								// Can supports tags from 256bit tags (TI HF-I Std/Pro) to 64kbit (STM M24LR64) tags
#endif

#ifdef ENABLE_STANDALONE		// No card detected
	LED_14443A_OFF;
	LED_14443B_OFF;
	LED_15693_OFF;
#endif
}

//*****************************************************************************
//
//! NFC_appIso14443a - Customizeable application to search for ISO14443A/NFC
//! Type 2/4A Tag Platform compliant tags.
//!
//! Function configures TRF79xxA for ISO14443A and waits for a guard time which
//! allows the PICC to become powered by the RF field before sending a command.
//!
//! Tag detection is handled in this example by issuing a REQA
//! command, and if a reply is received, the ISO14443A anti-
//! collision process is run until a single tag has provided it's
//! entire UID. If the tag is Type 4 compliant, an NDEF read
//! function is called. If the tag is not Type 4 Compliant, then
//! a Type 2 Application is called.
//!
//! \return None.
//
//*****************************************************************************

uint8_t NFC_appIso14443a(void)
{
#ifdef ENABLE_14443A

#if (TRF79xxA_VERSION == 70)
	if (TRF79xxA_checkExternalRfField() == true)
	{
		return STATUS_FAIL;
	}
#endif

	TRF79xxA_setupInitiator(0x88);		// Configure the TRF79xxA for ISO14443A @ 106kbps and Receive no CRC

	// When a PICC is exposed to an unmodulated operating field
	// it shall be able to accept a quest within 5 ms.
	// PCDs should periodically present an unmodulated field of at least
	// 5.1 ms duration. (ISO14443-3)
	MCU_delayMillisecond(6);

	ISO14443A_setRecursionCount(0); 		// Clear the recursion count for anticollision loops

	if (ISO14443A_selectTag(REQA) == STATUS_SUCCESS)	//  Do a complete anticollision sequence as described in ISO14443-3 standard for type A
	{
		if (ISO14443A_getType4ACompliance() == true)
		{
			NFC_appIso14443aType4NDEF();	// For a Type 4A compliant tag, the tag is put into Layer 4, and in order to attempt to read/write NDEF contents
		}
		else
		{
			NFC_appIso14443aType2(0x10);	// If a tag that is not Type 4A compliant then assume it is NFC Type 2. Proceed to read data block(s)
		}
	}
	else
	{
#ifdef ENABLE_STANDALONE		// No card detected
		LED_14443A_OFF;
#endif
	}

	TRF79xxA_turnRfOff();
#endif

	return STATUS_SUCCESS;
}

//*****************************************************************************
//
//! NFC_appIso14443aType4NDEF - Customizeable application to read NDEF data
//! from a Type 4A NDEF Formatted tag.
//!
//! This function sends the needed commands to select and read NDEF data from
//! a Type 4A Compliant NFC tag. Tags which are Type 4A compliant are selected
//! via RATS. The PPS command can then be used to change the data rate from
//! 106kbps up to 848kbps.
//!
//! If the tag contains an NDEF message, then the NDEF API's will attempt to
//! select the NDEF file and read the data contained within it.
//!
//! \return None.
//
//*****************************************************************************

void NFC_appIso14443aType4NDEF(void)
{
#ifdef ENABLE_14443A
	TRF79xxA_writeRegister(TRF79XXA_ISO_CONTROL,0x08);	// Enable RX with CRC

	if(ISO14443A_sendRATS() == STATUS_SUCCESS)
	{
		if(ISO14443A_sendPPS() == STATUS_SUCCESS)
		{
			NDEF_setBlockNumberBit(0);

			if(NDEF_selectApplication() == STATUS_SUCCESS) // Selects NDEF Application
			{
				MCU_delayMillisecond(1);						// Short delay before sending next command
				NDEF_readCapabilityContainer();
				MCU_delayMillisecond(1);						// Short delay before sending next command
				NDEF_readApplication();
			}
			else
			{
#ifdef ENABLE_STANDALONE
				LED_14443A_OFF;
#endif
			}
		}
	}
	else
	{
#ifdef ENABLE_STANDALONE		// Tag was not activated properly
		LED_14443A_OFF;
#endif
	}
#endif
}

//*****************************************************************************
//
//! NFC_appIso14443aType2 - Customizeable application to read data from
//! ISO14443A compliant tags that do not support NFC Forum Type 4A commands.
//!
//! \param ui8ReadBlocks is the number of blocks to read from the tag.
//!
//! This function reads block data from Type 2 Tags using the Read Four Blocks
//! command. The number of blocks to be read needs to be customzied by the
//! user either in an application specific manner, or by implementing a
//! look-up table feature that uses the SAK response to determine what the tag
//! is and how large it's memory contents are.
//!
//! \return None.
//
//*****************************************************************************

void NFC_appIso14443aType2(uint8_t ui8ReadBlocks)
{
#ifdef ENABLE_14443A
	uint8_t ui8BlockNumber = 0;

	// If tag is not ISO14443-4 Compliant, then attempt to read data blocks for Type 2 Tag
	for (ui8BlockNumber = 0x00; ui8BlockNumber < ui8ReadBlocks; ui8BlockNumber = ui8BlockNumber+4)
	{
		if (ISO14443A_sendT2TReadFourBlocks(ui8BlockNumber) == STATUS_FAIL)
		{
#ifdef ENABLE_HOST
			UART_sendCString("Read Error, Blocks: ");
			UART_putByte(ui8BlockNumber);
			UART_sendCString(" to ");
			UART_putByte(ui8BlockNumber+3);
			UART_putNewLine();
#endif
#ifdef ENABLE_STANDALONE
			LED_14443A_OFF;
#endif
			break;
		}
	}
#endif
}

//*****************************************************************************
//
//! NFC_appIso14443b - Customizeable application to search for ISO14443B/NFC
//! Type 4B Tag Platform compliant tags.
//!
//! Function configures TRF79xxA for ISO14443B and waits for a guard time which
//! allows the PICC to become powered by the RF field before sending a command.
//!
//! This function sends the needed commands to detection, select, and read NDEF
//! data from a Type 4B Compliant NFC tag. Tag detection/selection is handled
//! in this example by issuing a REQB command, and if a reply is received then
//! issuing an ATTRIB command to activate and select the tag prior to reading
//! any data.
//!
//! If the tag contains an NDEF message, then the NDEF API's will attempt to
//! select the NDEF file and read the data contained within it.
//!
//! \return None.
//
//*****************************************************************************

uint8_t NFC_appIso14443b(void)
{
#ifdef ENABLE_14443B
	bool bTagFound = false;

#if (TRF79xxA_VERSION == 70)
	if (TRF79xxA_checkExternalRfField() == true)
	{
		return STATUS_FAIL;
	}
#endif

	TRF79xxA_setupInitiator(0x0C);		// Configure the TRF79xxA for ISO14443B @ 106kbps

	// When a PICC is exposed to an un-modulated operating field
	// it shall be able to accept a quest within 5 ms.
	// PCDs should periodically present an un-modulated field of at least
	// 5.1 mSec duration. (ISO14443-3)

	// *NOTE* Guard time extended to 10 milliseconds in order to support passively
	// powered RF430CL330H designs such as seen in TIDA-00217.
	MCU_delayMillisecond(10);

	if (ISO14443B_sendPollCmd(REQB,0))	// Issue REQB Command with 1 slot to start
	{
		bTagFound = true;
	}

	// If a tag is found proceed to try and read/write an NDEF message to it
	if(bTagFound)
	{
		if (ISO14443B_sendAttrib() == STATUS_SUCCESS)	// Send ATTRIB Request
		{
			if (ISO14443B_getType4BCompliance() == true)
			{
				NDEF_setBlockNumberBit(0);
				// Attempt to selects NDEF Application - if successful move to NDEF Process
				if(NDEF_selectApplication() == STATUS_SUCCESS)
				{
					// Read NDEF Content
					NDEF_readCapabilityContainer();
					NDEF_readApplication();
				}
			}
			else
			{
#ifdef ENABLE_STANDALONE
				LED_14443B_OFF;
#endif
			}
		}
	}
	else
	{
#ifdef ENABLE_STANDALONE		// No card detected
		LED_14443B_OFF;
#endif
	}

	TRF79xxA_turnRfOff();		// Turn off RF field once done reading the tag(s)
#endif

	return STATUS_SUCCESS;
}

//*****************************************************************************
//
//! NFC_appIso15693 - Customizeable application to search for ISO15693
//! compliant tags.
//!
//! Function configures TRF79xxA for ISO1593 and waits for a guard time which
//! allows the VICC to become powered by the RF field before sending a command.
//!
//! Tag detection/selection is handled in this example by issuing a Single Slot
//! Inventory request. If a single tag does not reply, then a 16 Slot Inventory
//! request is issued. If onle a single tag has been presented, then an API to
//! read out it's block data will be called.
//!
//! \return None.
//
//*****************************************************************************

uint8_t NFC_appIso15693(void)
{
#ifdef ENABLE_15693
	uint8_t ui8TagFound = STATUS_FAIL;
	uint8_t ui8AddressedFlag = 0x00;

#if (TRF79xxA_VERSION == 70)
	if (TRF79xxA_checkExternalRfField() == true)
	{
		return STATUS_FAIL;
	}
#endif

	TRF79xxA_setupInitiator(0x02);		// Configure the TRF79xxA for ISO15693 @ High Bit Rate, One Subcarrier, 1 out of 4

	// The VCD should wait at least 1 ms after it activated the
	// powering field before sending the first request, to
	// ensure that the VICCs are ready to receive it. (ISO15693-3)
	MCU_delayMillisecond(20);

	ISO15693_resetTagCount();

	ui8TagFound = ISO15693_sendSingleSlotInventory();							// Send a single slot inventory request to try and detect a single ISO15693 Tag

	// Inventory failed - search with full anticollision routine
	if (ui8TagFound == STATUS_FAIL)
	{
		ISO15693_resetRecursionCount();			// Clear the recursion counter
		MCU_delayMillisecond(5);				// Delay before issuing the anticollision commmand
		ui8TagFound = ISO15693_runAnticollision(0x06, 0x00, 0x00);		// Send 16 Slot Inventory request with no mask length and no AFI
		ui8AddressedFlag = 0x20; 			// Collision occurred, send addressed commands
	}

	if (ui8TagFound == STATUS_SUCCESS)
	{
		if (ISO15693_getTagCount() > 1)
		{
#ifdef ENABLE_HOST
			UART_putNewLine();
			UART_sendCString("Multiple ISO15693 Tags Found");
			UART_putNewLine();
			UART_sendCString("# of Tags Detected: ");
			UART_putByteDecimalValue(ISO15693_getTagCount());
			UART_putNewLine();
			UART_sendCString("Place only 1 tag in RF Field to read data");
			UART_putNewLine();
#endif
		}
		else
		{
			NFC_appIso15693ReadTag(0x02 | ui8AddressedFlag);					// Read an ISO15693 tag
//			NFC_appIso15693ReadExtendedTag(0x0A | ui8AddressedFlag);			// Read an ISO15693 tag which has extended protocol implemented
//			ISO15693_sendReadMultipleBlocks(0x22,0x00,25);						// Example to read 25 blocks starting @ Block 0 from a tag which supports Read Multiple Block command
		}
	}
	else
	{
#ifdef ENABLE_STANDALONE		// No card detected
		LED_15693_OFF;
#endif
	}

	TRF79xxA_turnRfOff();						// Turn off RF field once done reading the tag(s)
#endif

	return STATUS_SUCCESS;
}

//*****************************************************************************
//
//! NFC_appIso15693ReadTag - Read all blocks of a ISO15693 tag.
//!
//! \param ui8ReqFlag are the request flags for ISO15693 commands.
//!
//! This function issues Get System Information command to determine how many
//! blocks of data are stored within the ISO15693 tag.
//!
//! Afterwards, all blocks are read out using a Read Single block, unless an
//! error occurs during the read process at which point the function will stop
//! reading data and exit.
//!
//! \return None.
//
//*****************************************************************************

void NFC_appIso15693ReadTag(uint8_t ui8ReqFlag)
{
#ifdef ENABLE_15693
	uint16_t ui16ReadBlocks = 0x00;
	uint16_t ui16LoopCount = 0x00;

	ui16ReadBlocks = ISO15693_sendGetSystemInfo(ui8ReqFlag); 	// Get Tag Information with Request Flag = 0x02

	if (ui16ReadBlocks != 0x00)
	{
		// Read all available blocks on the ISO15693 Tag
		for (ui16LoopCount = 0; ui16LoopCount < ui16ReadBlocks+1; ui16LoopCount++)
		{
			if (ISO15693_sendReadSingleBlock(ui8ReqFlag, ui16LoopCount) == STATUS_FAIL)	// Keep reading blocks unless a No Response is received
			{
				LED_15693_OFF;
				// No Response - stop reading
				break;
			}
		}
	}
#endif
}

//*****************************************************************************
//
//! NFC_appIso15693ReadExtendedTag - Read all blocks of an ISO15693 tag that
//! requires the Protocol Extension flag to be used.
//!
//! \param ui8ReqFlag are the request flags for ISO15693 commands.
//!
//! This function issues Get System Information command with the Protocol
//! Extension bit set in the request flags to determine how many blocks of
//! data is stored within the ISO15693 tag.
//!
//! Then all blocks are read out using a Read Single block for Extended
//! ISO15693 tags, unless an error occurs during the read process at which
//! point the function will stop reading data and exit.
//!
//! \return None.
//
//*****************************************************************************

void NFC_appIso15693ReadExtendedTag(uint8_t ui8ReqFlag)
{
#ifdef ENABLE_15693
	uint16_t ui16ReadBlocks = 0x00;
	uint16_t ui16LoopCount = 0x00;

	ui8ReqFlag |= 0x08; 	// Add in Protocol Extension Flag if it was omitted from the inputted request flags

	ui16ReadBlocks = ISO15693_sendGetSystemInfoExtended(ui8ReqFlag);	// Issue a Get System Info with Protocol Extension

	if (ui16ReadBlocks != 0x00)
	{
		// Read all available blocks on the ISO15693 Tag
		for (ui16LoopCount = 0; ui16LoopCount < ui16ReadBlocks+1; ui16LoopCount++)
		{
			if (ISO15693_sendReadSingleBlockExtended(ui8ReqFlag, ui16LoopCount) == STATUS_FAIL)	// Keep reading blocks until a No Response is received
			{
				LED_15693_OFF;
				// No Response - stop reading
				break;
			}
		}
	}
#endif
}

//*****************************************************************************
//
//! NFC_appFeliCa - Customizeable application to search for FeliCa tags.
//!
//! Function configures TRF79xxA for FeliCa and waits for a guard time which
//! allows the tag to become powered by the RF field before sending a command.
//!
//! Tag detection/selection is handled in this example by issuing a polling
//! command followed by reading the Attribute Information Block (Block 0x00)
//! of the tag.
//!
//! \return None.
//
//*****************************************************************************

uint8_t NFC_appFeliCa(void)
{
#ifdef ENABLE_FELICA

#if (TRF79xxA_VERSION == 70)
	if (TRF79xxA_checkExternalRfField() == true)
	{
		return STATUS_FAIL;
	}
#endif

	TRF79xxA_setupInitiator(0x1A);			// Use to configure the TRF79xxA for FeliCa @ 212kbps
//	TRF79xxA_setupInitiator(0x1B);			// Use to configure the TRF79xxA for FeliCa @ 424kbps

	MCU_delayMillisecond(20);					// Guard time of 20 mS.

	if (FeliCa_pollSingleSlot() == STATUS_SUCCESS)	// Send a polling command with 4 time slots
	{
		FeliCa_putTagInformation();			// Print out tag information to the UART

		FeliCa_readSingleBlock(0x00);			// Read single block without encryption

		if(FeliCa_getNDEFLength() != 0x00)
		{
			FeliCa_readFourBlocks(0x01);		// Read four blocks, without encryption
		}
		else
		{
			LED_15693_OFF;
			LED_14443B_OFF;
		}
	}
	else
	{
#ifdef ENABLE_STANDALONE		// No card detected
		LED_15693_OFF;
		LED_14443B_OFF;
#endif
	}

	TRF79xxA_turnRfOff();						// Turn off RF field once done reading the tag(s)
#endif

	return STATUS_SUCCESS;
}

//*****************************************************************************
//
//! NFC_init - Initialize the layers for each enabled tag type.
//!
//! This function calls the needed initialization functions for
//! each enabled tag type technology and any other related files.
//!
//! \return None
//
//*****************************************************************************

void NFC_init(void)
{
#ifdef ENABLE_14443A
	ISO14443A_init();
	NDEF_setBlockNumberBit(0);
#endif
#ifdef ENABLE_14443B
	ISO14443B_init();
	NDEF_setBlockNumberBit(0);
#endif
#ifdef ENABLE_15693
	ISO15693_init();
#endif
#ifdef ENABLE_FELICA
	FeliCa_init();
#endif
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
