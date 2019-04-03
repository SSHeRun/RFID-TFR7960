/*
 * File Name: iso14443b.h
 *
 * File Name: Headers and Defines for ISO14443B Specific Functions
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

#ifndef _ISO14443B_H_
#define _ISO14443B_H_

//================================================================

#include "trf79xxa.h"
#include "ndef.h"

//===============================================================

// ISO14443B Commands
#define REQB			0x00
#define WUPB			0x08
#define HLTB_CMD		0x50
#define ATTRIB_CMD		0x1D

//===============================================================

bool ISO14443B_runAnticollision(uint8_t ui8NumberofSlots, bool bRecursion);

uint8_t ISO14443B_sendPollCmd(uint8_t ui8Command, uint8_t ui8NValue);
uint8_t ISO14443B_sendSlotMarker(uint8_t ui8APn);
uint8_t ISO14443B_sendAttrib(void);
uint8_t ISO14443B_sendHalt(void);

bool ISO14443B_getType4BCompliance(void);
uint8_t * ISO14443B_getPUPI(void);
void ISO14443B_init(void);

//===============================================================

#endif
