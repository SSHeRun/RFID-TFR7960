/*
 * File Name: nfc_app.h
 *
 * Description: Headers and Defines for the Application Layer processing
 * of the NFC/RFID stack.
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

#ifndef NFC_H_
#define NFC_H_

//================================================================

#include "iso14443a.h"
#include "iso14443b.h"
#include "iso15693.h"
#include "felica.h"

//================================================================

#define ENABLE_14443A
#define ENABLE_14443B
#define ENABLE_15693
#define ENABLE_FELICA

//================================================================

void NFC_findTag(void);

uint8_t NFC_appIso14443a(void);
uint8_t NFC_appIso14443b(void);
uint8_t NFC_appFeliCa(void);
uint8_t NFC_appIso15693(void);

void NFC_appIso14443aType4NDEF(void);
void NFC_appIso14443aType2(uint8_t ui8ReadBlocks);

void NFC_appIso15693ReadTag(uint8_t ui8ReqFlag);
void NFC_appIso15693ReadExtendedTag(uint8_t ui8ReqFlag);

void NFC_init(void);

#endif /* NFC_H_ */
