/*
 * File Name: uart.h
 *
 * Description: Header file for all functions for uart.c
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


#ifndef _UART_H_
#define _UART_H_

//================================================================

#include "mcu.h"

//===============================================================

// Toggle UART Host Communication functionality

// TODO: Hardware modification to the default shipped board is required for UART:
// Take the two jumpers for the "TXD" and "RXD" pins and connect them horizontally
// instead of vertically. When done correctly, this should connect the pins which
// are labelled "TXD" and "RXD" pins which are on the MSP-EXP430G2 side of the
// board to each other. The two pins above them on the "Emulation" side of the
// board should also be tied together horizontally.

#define ENABLE_HOST

//===============================================================

#ifdef ENABLE_HOST

uint8_t UART_nibble2Ascii(uint8_t ui8AsciiNibble);
void UART_putByte(uint8_t ui8TxByte);
void UART_putChar(uint8_t ui8TxChar);
void UART_putNewLine(void);
void UART_putSpace(void);
void UART_response(uint8_t * pui8Buffer, uint8_t ui8Length);
void UART_putBuffer(const uint8_t * pui8Buffer, uint8_t ui8Length);
void UART_putBufferAscii(const uint8_t * pui8Buffer, uint8_t ui8Length);
void UART_sendCString(uint8_t * pui8Buffer);
void UART_setup (void);
void UART_putIntroReaderMsg(uint8_t * pui8VersionNumber, uint8_t * pui8VersionDate);
void UART_putByteDecimalValue(uint8_t ui8HexByte);
#endif

//===============================================================

#endif
