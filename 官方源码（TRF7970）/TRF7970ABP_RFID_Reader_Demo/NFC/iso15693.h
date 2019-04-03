/*
 * File Name: iso15693.h
 *
 * File Name: Headers and Defines for ISO15693 Specific Functions
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

#ifndef _ISO15693_H_
#define _ISO15693_H_

//================================================================

#include "trf79xxa.h"

//===============================================================

// Number of recursive calls allowed by Anticollision sequence.
// Set to 8 for TI Example code based on 250 byte stack
// Each recursive call will take up 18 bytes of Stack space
#define ISO15693_MAX_RECURSION_COUNT	8

// Premade Request Flags
#define T5T_SINGLE_SLOT_INVENTORY       0x26	// Tables 3 & 5 in ISO15693-3; single-subcarrier, high tag data rate, one slot
#define T5T_SIXTEEN_SLOT_INVENTORY		0x06	// Tables 3 & 5 in ISO15693-3; single-subcarrier, high tag data rate, sixteen slots

// Individual Flags
#define T5T_REQ_FLAG_SUB_CARRIER		0x01
#define T5T_REQ_FLAG_HIGH_DATA			0x02
#define T5T_REQ_FLAG_INVENTORY			0x04
#define T5T_REQ_FLAG_EXTENDED			0x08
#define T5T_REQ_FLAG_SELECT				0x10
#define T5T_REQ_FLAG_ADDRESSED			0x20
#define T5T_REQ_FLAG_OPTION				0x40

#define T5T_INV_FLAG_AFI				0x10
#define T5T_INV_FLAG_SINGLE_SLOT		0x20
#define T5T_INV_FLAG_OPTION				0x40

#define T5T_RESP_FLAG_NO_ERROR			0x00
#define T5T_RESP_FLAG_EXTENDED			0x08

//===============================================================

uint8_t ISO15693_sendSingleSlotInventory(void);
uint8_t ISO15693_runAnticollision(uint8_t ui8ReqFlags, uint8_t ui8MaskLength, uint8_t ui8Afi);

uint16_t ISO15693_sendGetSystemInfo(uint8_t ui8ReqFlag);
uint16_t ISO15693_sendGetSystemInfoExtended(uint8_t ui8ReqFlag);

uint8_t ISO15693_sendReadSingleBlock(uint8_t ui8ReqFlag, uint8_t ui8BlockNumber);
uint8_t ISO15693_sendReadMultipleBlocks(uint8_t ui8ReqFlag, uint8_t ui8FirstBlock, uint8_t ui8NumberOfBlocks);
uint8_t ISO15693_sendReadSingleBlockExtended(uint8_t ui8ReqFlag, uint16_t ui16StartBlock);
uint8_t ISO15693_sendWriteSingleBlock(uint8_t ui8ReqFlag, uint8_t ui8BlockNumber, uint8_t ui8BlockSize, uint8_t * pui8BlockData);

uint8_t * ISO15693_getUid(void);
uint8_t ISO15693_getTagCount(void);
void ISO15693_resetTagCount(void);
void ISO15693_resetRecursionCount(void);

void ISO15693_init(void);

//===============================================================

#endif
