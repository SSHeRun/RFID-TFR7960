/*
 * File Name: mcu.c
 *
 * Description: Contains general microcontroller level functions
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

#include "mcu.h"
#include "trf79xxa.h"

//*****************************************************************************
//
//! \addtogroup mcu_api MCU Specific Peripheral API's
//! @{
//!
//! This section contains descriptions for the general MSP430G2553 specific
//! peripheral functions such as clock configurations and timers.
//
//*****************************************************************************

static uint16_t ui16VLOConst;

//*****************************************************************************
//
//! MCU_setCounter - Set up the counter for Timer A0
//!
//! \param ui16mSecTimeout is the amount of time to set the counter for.
//!
//! Set up the counter for Timer A0 and enable the interrupt.
//!
//! \return None.
//
//*****************************************************************************

void MCU_setCounter(uint16_t ui16mSecTimeout)
{
	// Max delay = 3.2 seconds (to cover maximum VLO speed of 20kHz)
	if (ui16mSecTimeout > 3200)
	{
		ui16mSecTimeout = 3200;
	}

	TA0CTL |= TACLR;						// Clear Counter
	TA0CTL &= ~TASSEL_3;					// Clear Source Clock Settings

    TA0CCTL0 = CCIE;                		// compare interrupt enable
    COUNTER_VALUE = (((ui16VLOConst+1) * ui16mSecTimeout)>>1); // Div by 2 due to ACLK divider
    TA0CTL |= MC_1 + TASSEL_1;      		// Source from ACLK, Up Mode
}

//*****************************************************************************
//
//! MCU_delayMillisecond - Delay for inputted number of millisecond.
//!
//! \param n_ms is the number of milliseconds to delay by.
//!
//! Blocking function to delay is approximately one millisecond, looping
//! until the inputted number of milliseconds have gone by. DELAY_1ms must
//! be calibrated based on the clock speed of the MCU.
//!
//! \return None.
//
//*****************************************************************************

void MCU_delayMillisecond(uint32_t n_ms)
{
    while (n_ms--)
    {
    	__delay_cycles(DELAY_1ms);		// clock speed in Hz divined by 1000
    }
}

//*****************************************************************************
//
//! MCU_initClock - Calibrate the Oscillator.
//!
//! Function to calibrate the DCO of the MSP430G2553.
//!
//! \return None.
//
//*****************************************************************************

void MCU_initClock(void)
{
    // select DCO to 8MHz

	if (CALBC1_8MHZ==0xFF)					// If calibration constant erased
	{
		while(1);                               // do not load, trap CPU!!
	}

	// Follow recommended flow. First, clear all DCOx and MODx bits.
	// Then apply new RSELx values. Finally, apply new DCOx and MODx bit
	// values.
	DCOCTL = 0x00;
	BCSCTL1 = CALBC1_8MHZ;
	DCOCTL = CALDCO_8MHZ;

    // Disable XT1 pins
    P2SEL &= ~(BIT6 + BIT7);

    // Disable XT1 high frequency mode, ACLK divider = /2
    BCSCTL1 &= ~XTS;
    BCSCTL1 |= DIVA_1;

    // Set ACLK source to VLO (~12kHz) - with ACLK divider, 12kHz/2 = ACLK speed = 6kHz
    BCSCTL3 |= LFXT1S_2;

    __delay_cycles(1000);

	return;
}

//*****************************************************************************
//
//! MCU_calculateVLOFreq - Calculate the VLO Frequency to calibrate clocks.
//!
//! Using the MSP430 VLO Library, get a calibration variable which can be used
//! to determine the VLO frequency value. Knowing the VLO frequency will allow
//! for the calibration of any clocks which need to use it.
//!
//! In this case - the ui16VLOConst is a global variable which is used
//! by ACLK to create a 1 ms delay, which is why the VLO Frequency is divided
//! by 1000.
//!
//! \return None.
//
//*****************************************************************************

void MCU_calculateVLOFreq(void)
{
	int16_t i16VLOCalib;
	uint32_t ui32VLOFreq;

	i16VLOCalib = TI_measureVLO();		// VLO_Calib = number of 1MHz cycles in 8 ACLK cycles

	ui32VLOFreq = 8000000 / i16VLOCalib;

	ui16VLOConst = ui32VLOFreq / 1000;
}


//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
