/*
 * File Name: felica.h
 *
 * File Name: Headers and Defines for FeliCa Specific Functions
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

#ifndef _FELICA_H_
#define _FELICA_H_

//===============================================================

#include "trf79xxa.h"

//===============================================================

#define FELICA_POLLING				0x00
#define FELICA_READ_NO_AUTH			0x06
#define FELICA_READ_NO_AUTH_RESP	0x07
#define FELICA_WRITE_NO_AUTH		0x08
#define FELICA_WRITE_NO_AUTH_RESP	0x09

#define FELICA_BITRATE_424kbps		0x01
#define FELICA_BITRATE_212kbps		0x00

//===============================================================

uint8_t FeliCa_pollSingleSlot(void);
void FeliCa_readSingleBlock(uint8_t ui8BlockNumber);
void FeliCa_readFourBlocks(uint8_t ui8StartBlock);
void FeliCa_putTagInformation(void);

uint32_t FeliCa_getNDEFLength(void);
uint8_t * FeliCa_getIDm(void);
void FeliCa_init(void);

//===============================================================

#endif
