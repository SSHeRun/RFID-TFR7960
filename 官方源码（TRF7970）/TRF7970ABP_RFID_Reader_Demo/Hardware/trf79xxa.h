/*
 * File Name: trf797x.h
 *
 * Description: Headers and Defines for TRF797x Driver Functions
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

#ifndef _TRF79xxa_H_
#define _TRF79xxa_H_

//===============================================================

#include "spi.h"
#include "uart.h"

//===============================================================

#define ENABLE_STANDALONE			// Disables all LED features to allow for easier porting

//===============================================================
//
// This defines the version of the TRF79xxA chip being used (Use Decimal, not Hex for the numbers)
// Current Support:
//  - TRF7970A, TRF7964A* ==> 70
//  - TRF7960A ==> 60
//
// * The TRF7964A is a derivative of the TRF7970A and thus uses all the same settings as the TRF7970A.
//
// FIXME: There is no existing BoosterPack with the TRF7960A that pairs natively with the MSP-EXP430G2 LaunchPad.
// This functionality is provided only as a helping hand for users interested in using the TRF7960A in
// their own system to make code porting easier. Initial evaluation with TI evaluation hardware should take
// place on the TRF7970A BoosterPack.
//

#define	TRF79xxA_VERSION			70

//===============================================================


//
// This defines the largest FIFO size of the TRF79xxA chip being used
// TRF7970A and TRF7964A - 127 bytes
// TRF7960A, TRF7961A, TRF7962A, and TRF7963A - 12 bytes
//
#if (TRF79xxA_VERSION == 70)
#define TRF79xxA_MAX_FIFO_SIZE 			127
#elif (TRF79xxA_VERSION == 60)
#define TRF79xxA_MAX_FIFO_SIZE 			12
#endif

//
// This defines the size of the FIFO buffer generated. This size comes out of RAM memory.
// For example, setting this to 100 takes up 100 bytes of RAM.
// Recommendation is to reduce this if the full buffer would not be used in certain applications.
//
#define NFC_FIFO_SIZE 					100

//
// Power Settings for TRF79xxA.
// Each definition is based of the setting which would required in the Chip Status Control Register
// These settings do not differ across the TRF79xxA family.
//
typedef enum
{
	TRF79xxA_3V_FULL_POWER = 0x00,			// Full Power, 3V
	TRF79xxA_5V_FULL_POWER = 0x01,			// Full Power, 5V
	TRF79xxA_3V_HALF_POWER = 0x10,			// Half Power, 3V
	TRF79xxA_5V_HALF_POWER = 0x11,			// Half Power, 5V
}tTrfPowerOptions;

//
// Structure for various TRF79xxA Settings
//
typedef struct
{
	uint8_t ui8IsoControl;
	bool bRfFieldOn;					// RF Field Status
	tTrfPowerOptions eTrfPowerSetting;
}tTrfSettings;

//
// TRF79xxA Status
//
typedef enum
{
	TRF_IDLE,
	TX_COMPLETE,
	RX_COMPLETE,
	TX_ERROR,
	RX_WAIT,
	RX_WAIT_EXTENSION,
	TX_WAIT,
	PROTOCOL_ERROR,
	COLLISION_ERROR,
	NO_RESPONSE_RECEIVED,
	NO_RESPONSE_RECEIVED_15693
}tTrfStatus;

//===============================================================
//==== Version History ==========================================
//
// REV.   DATE        		WHO    			DETAIL
// 1.00	  July 30, 2015		RJ				Working ISO14443A, ISO14443B, and ISO15693 Reader firmware, with basic NDEF support for Type 4 tags.
// 1.01	  Aug 3, 2015		RJ				Added SpiReceiveByte and SpiSendByte functions, added #define for Standalone mode, revised comments
// 1.02	  Aug 5, 2015		RJ				Added UART outputs to ISO14443A Anticollision and Selection process, verified Type 2 Reader
// 1.03	  Aug 7, 2015		RJ				Added Felica read functionality, fixed issue with TRF Driver which caused firmware to lock up
// 1.04	  Aug 13, 2015		RJ				Updated Felica tag identification table, revised available NDEF commands, updated naming conventions for hardware layer drivers
// 1.05	  Sept 4, 2015		RJ				Fixed issue with ISO15693 Anticollision, updated function header comments for uart.c, mcu.c, and trf797x.c
// 1.06	  Sept 16, 2015		RJ				Fixes to ISO15693 Anticollision recursion, reduced global buffer size, improved checks for RF field/ISO Control register settings prior to issuing RF commands
// 1.07	  Sept 22, 2015		RJ				Revamp of ISO14443A Anticollision. Testing in progress - can handle first round of collisions for Double UID tags.
// 1.08	  Sept 28, 2015		RJ				Finished correct implementation of ISO14443A Anticollision, made changes to ISO15693 Anticollision to save significant stack space.
// 1.09	  Sept 30, 2015		RJ				Revised ISO14443B Anticollision procedure and SlotMarker function. Updated ISO14443A StoreUID function to handle if the last UID bytes received start with 0x88
// 1.10	  Oct 6, 2015		RJ				Finished Doxygen comments for ISO14443B. Edited file names for NDEF and the TRF Driver. Updated the application layer to make it more intuitive.
// 1.11	  Oct 8, 2015		RJ				Updated LED Behaviors. Made additional modifications to the application layer.
// 1.12	  Oct 19, 2015		RJ				Modified FeliCa polling functionality.
// 1.13	  Oct 21, 2015		RJ				Minor application layer improvements.
// 1.14	  Nov 5, 2015		RJ				Fixes to ISO15693 Write Single Block function for transmitting with the option flag. Modified TRF Driver to handle FIFO Watermark during RX. Added #define for FIFO buffer size.
// 1.15   Nov 12, 2015		RJ				Fix to modulation depths during TRF initialization. Adjustments to Type 4 NDEF file to handle max FIFO Size. Adjustments to Type 2/4A Application handling.
// 1.16   Nov 19, 2015		RJ				Fix to ISO15693 No Response Timeout handling in TRF Driver.
// 1.17   Dec 16, 2015		RJ				Fixed issue with ISO15693 recursive call overflowing stack with updated anticollision process.
// 1.18   Jan 7, 2016		RJ				Fixed issue with ISO14443A Anticollision buffers being overflown. Adjusted Application Layer behavior for ISO15693. Added check for PPS support when receiving ATS for ISO14443A.
// 2.00   Jan 8, 2016		RJ				Preparation for release - cleaned up code files, finished comments, removed debugging sections
// 2.01   Jan 12, 2016		RJ				Minor modifications to application to improve Out of the Box experience, fixes to NDEF process for Type 4 Tags.
// 2.02   Mar 10, 2016		RJ				Updated naming of functions in some files to follow naming conventions better.
// 2.03   Mar 23, 2016		RJ				Re-worked initialization of global variables, added Type 2 Write Single Block function, adjusted Terminal data outputs to lessen Flash memory impact
// 2.04   Apr 18, 2016		RJ				Added 4 bit ACK/NAK support for ISO14443A Type 2 Tags.
// 2.05   May 17, 2016		RJ				Added soft reset on protocol errors, fixes to 4 bit ACK/NAK support, clarified some Timer names
// 2.06   June 29, 2016		RJ				Resolved issues with Soft Reset functionality in specific subcases, added external RF field avoidance checks, updated Regular Control settings
// 2.07   Aug 29, 2016		RJ				Added VLO Calibration to handle VLO drifting over time, adjusted TRF7970A software reset sequence
// 2.08   Aug 31, 2016		RJ				Significant TRF driver updates - begun to add hooks to support both 60A and 70A, function naming updates, removal of unneeded functions, updates to check external RF field
// 2.09   Sept 14, 2016     RJ				Finished primary TRF driver updates, renamed functions across NFC stack, added recursion count to ISO15693 anticollision to prevent any possible stack overflows
// 2.10   Nov 9, 2016       RJ				Renamed a few remaining TRF driver functions, added API to allow higher levels to fetch TRF buffer, added Doxygen comments to TRF driver and MCU layers
// 2.11   Nov 21, 2016      RJ				Added Doxygen comments to NFC Layers, edited multiple NDEF functions, changed an if else tree to a switch statement in the ISO14443A_sendPPS function
// 3.00   Feb 22, 2017		RJ				Added support for TRF7960A which required re-work of Write FIFO and TX/RX Wait functions.
// 3.01   Mar 20, 2017		RJ				Bug fixes to updated TRF79xx FIFO handling and ISO15693 layer.

//===============================================================
//==== Version Control ==========================================

#define RFID_READER_FW_VERSION		"3.01"
#define RFID_READER_FW_DATE			"March 20th, 2017"

//===============================================================

//==== TRF797x definitions ======================================

//---- Direct commands ------------------------------------------

#define TRF79XXA_IDLE_CMD                    			0x00
#define TRF79XXA_SOFT_INIT_CMD               			0x03
#if (TRF79xxA_VERSION == 70)
#define TRF79XXA_INITIAL_RF_COLLISION_AVOID_CMD			0x04
#define TRF79XXA_PERFORM_RES_RF_COLLISION_AVOID_CMD		0x05
#define TRF79XXA_PERFORM_RES_RF_COLLISION_AVOID_N0_CMD	0x06
#endif
#define TRF79XXA_RESET_FIFO_CMD              			0x0F
#define TRF79XXA_TRANSMIT_NO_CRC_CMD         			0x10
#define TRF79XXA_TRANSMIT_CRC_CMD            			0x11
#define TRF79XXA_DELAY_TRANSMIT_NO_CRC_CMD   			0x12
#define TRF79XXA_DELAY_TRANSMIT_CRC_CMD      			0x13
#define TRF79XXA_TRANSMIT_NEXT_SLOT_CMD      			0x14
#if (TRF79xxA_VERSION == 70)
#define TRF79XXA_CLOSE_SLOT_SEQUENCE_CMD     			0x15
#endif
#define TRF79XXA_STOP_DECODERS_CMD           			0x16
#define TRF79XXA_RUN_DECODERS_CMD            			0x17
#define TRF79XXA_TEST_INTERNAL_RF_CMD        			0x18
#define TRF79XXA_TEST_EXTERNAL_RF_CMD        			0x19

//---- Registers ------------------------------------------------

#define TRF79XXA_CHIP_STATUS_CONTROL			0x00
#define TRF79XXA_ISO_CONTROL					0x01
#define TRF79XXA_ISO14443B_TX_OPTIONS			0x02
#define TRF79XXA_ISO14443A_BITRATE_OPTIONS		0x03
#define TRF79XXA_TX_TIMER_EPC_HIGH				0x04
#define TRF79XXA_TX_TIMER_EPC_LOW				0x05
#define TRF79XXA_TX_PULSE_LENGTH_CONTROL		0x06
#define TRF79XXA_RX_NO_RESPONSE_WAIT_TIME		0x07
#define TRF79XXA_RX_WAIT_TIME					0x08
#define TRF79XXA_MODULATOR_CONTROL				0x09
#define TRF79XXA_RX_SPECIAL_SETTINGS			0x0A
#define TRF79XXA_REGULATOR_CONTROL				0x0B
#define TRF79XXA_IRQ_STATUS						0x0C
#define TRF79XXA_IRQ_MASK						0x0D
#define	TRF79XXA_COLLISION_POSITION				0x0E
#define TRF79XXA_RSSI_LEVELS					0x0F
#define TRF79XXA_SPECIAL_FUNCTION_1				0x10	// Register not documented on TRF7960A Datasheet, See sloa155 Section 7.8
#if (TRF79xxA_VERSION == 70)
#define TRF79XXA_SPECIAL_FUNCTION_2				0x11
#define TRF79XXA_FIFO_IRQ_LEVELS				0x14
#define TRF79XXA_NFC_LOW_DETECTION_LEVEL		0x16
#define TRF79XXA_NFC_ID_REG                 	0x17
#define TRF79XXA_NFC_TARGET_LEVEL				0x18
#define TRF79XXA_NFC_TARGET_PROTOCOL			0x19
#endif
#define TRF79XXA_TEST_SETTINGS_1				0x1A
#define TRF79XXA_TEST_SETTINGS_2				0x1B
#define TRF79XXA_FIFO_STATUS					0x1C
#define TRF79XXA_TX_LENGTH_BYTE_1				0x1D
#define TRF79XXA_TX_LENGTH_BYTE_2				0x1E
#define TRF79XXA_FIFO							0x1F

//---- IRQ STATUS -----------------------------------------------

#define TRF79XXA_IRQ_STATUS_IDLE					0x00
#define TRF79XXA_IRQ_STATUS_NO_RESPONSE				0x01
#define TRF79XXA_IRQ_STATUS_COLLISION_ERROR			0x02
#define TRF79XXA_IRQ_STATUS_FRAMING_ERROR 			0x04
#define TRF79XXA_IRQ_STATUS_PARITY_ERROR 			0x08
#define TRF79XXA_IRQ_STATUS_CRC_ERROR	 			0x10
#define TRF79XXA_IRQ_STATUS_FIFO_HIGH_OR_LOW 		0x20
#define TRF79XXA_IRQ_STATUS_RX_COMPLETE 			0x40
#define TRF79XXA_IRQ_STATUS_TX_COMPLETE 			0x80

//==== End of TRF797x definitions ===============================

//===============================================================

extern void TRF79xxA_disableSlotCounter(void);
extern void TRF79xxA_enableSlotCounter(void);
extern void TRF79xxA_initialSettings(void);
extern void TRF79xxA_writeRaw(uint8_t * pui8Payload, uint8_t ui8Length);
extern void TRF79xxA_readContinuous(uint8_t * pui8Payload, uint8_t ui8Length);
extern uint8_t TRF79xxA_readRegister(uint8_t ui8TrfRegister);
extern void TRF79xxA_reset(void);
extern void TRF79xxA_resetFIFO(void);
extern void TRF79xxA_resetIrqStatus(void);
extern void TRF79xxA_sendDirectCommand(uint8_t ui8Value);
extern void TRF79xxA_setupInitiator(uint8_t ui8IsoControl);
extern void TRF79xxA_turnRfOff(void);
extern void TRF79xxA_turnRfOn(void);
extern void TRF79xxA_writeContinuous(uint8_t * pui8Payload, uint8_t ui8Length);
extern void TRF79xxA_writeRegister(uint8_t ui8TrfRegister, uint8_t ui8Value);

extern void TRF79xxA_waitTxIRQ(uint8_t ui8TxTimeout);
extern void TRF79xxA_waitRxIRQ(uint8_t ui8RxTimeout);
extern tTrfStatus TRF79xxA_waitRxData(uint8_t ui8TxTimeout, uint8_t ui8RxTimeout);

extern tTrfStatus TRF79xxA_getTrfStatus(void);
extern uint8_t TRF79xxA_getCollisionPosition(void);
extern void TRF79xxA_setCollisionPosition(uint8_t ui8ColPos);

extern void TRF79xxA_setTrfStatus(tTrfStatus sTrfStatus);
extern void TRF79xxA_setTrfPowerSetting(tTrfPowerOptions sTrfPowerSetting);
extern uint8_t TRF79xxA_getRxBytesReceived(void);
extern uint8_t * TRF79xxA_getTrfBuffer(void);
extern uint8_t TRF79xxA_getIsoControlValue(void);
extern bool TRF79xxA_checkExternalRfField(void);

//===============================================================

#endif
