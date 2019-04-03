/*
 * File Name: iso14443.h
 *
 * Description: Headers and Defines for ISO14443A Specific Functions
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

#ifndef _ISO14443A_H_
#define _ISO14443A_H_

//================================================================

#include "trf79xxa.h"
#include "ndef.h"

//===============================================================

//Polling Defines
#define REQA 			0x26
#define WUPA 			0x52

//Anticollision Defines
#define SEL_CASCADE1	0x93
#define SEL_CASCADE2	0x95
#define SEL_CASCADE3	0x97
#define NVB_INIT 		0x20
#define NVB_FULL		0x70
#define CT				0x88

//RATS Defines
#define RATS_CMD		0xE0
#define RATS_PARAM		0x70

//PPS Defines
#define PPSS			0xD0
#define PPS0			0x11
#define PPS1_106		0x00
#define PPS1_212 		0x05
#define PPS1_424		0x0A
#define PPS1_848		0x0F

typedef enum
{
	ISO14443A_UID_UNKNOWN = 0x00,
	ISO14443A_UID_SINGLE = 0x04,	// Four Bytes
	ISO14443A_UID_DOUBLE = 0x07,	// Seven Bytes
	ISO14443A_UID_TRIPLE = 0x0A		// Ten Bytes
}tISO14443A_UidSize;

typedef enum
{
	CASCADE1,
	CASCADE2,
	CASCADE3,
	UID_COMPLETE,
	UID_INCOMPLETE
}tISO14443A_UidStatus;

typedef enum
{
	NO_COLLISION,
	COLLISION,
	NO_RESPONSE,
	COLLISION_FAIL
}tCollisionStatus;

//===============================================================

uint8_t ISO14443A_selectTag(uint8_t ui8Command);
tCollisionStatus ISO14443A_runAnticollision(tISO14443A_UidStatus sCascade);

uint8_t ISO14443A_sendPollCmd(uint8_t ui8Command);
tCollisionStatus ISO14443A_sendAnticollisionCmd(tISO14443A_UidStatus sCascade, uint8_t ui8NVB, uint8_t * pui8UID);
uint8_t ISO14443A_sendSelectCmd(tISO14443A_UidStatus sCascade, uint8_t * pui8UID, bool bSendCT);
uint8_t ISO14443A_sendHalt(void);
uint8_t ISO14443A_sendRATS(void);
uint8_t ISO14443A_sendPPS(void);

bool ISO14443A_storeUID(tISO14443A_UidStatus sCascade, uint8_t * pui8UID);

uint8_t ISO14443A_sendT2TReadFourBlocks(uint8_t ui8StartBlock);
uint8_t ISO14443A_sendT2TWriteSingleBlock(uint8_t ui8StartBlock, uint8_t * pui8TagData);

bool ISO14443A_getType4ACompliance(void);
void ISO14443A_setRecursionCount(uint8_t ui8RecursionCount);
uint8_t * ISO14443A_getUid(void);
tISO14443A_UidSize ISO14443A_getUidSize(void);

void ISO14443A_init(void);

//===============================================================

#endif
