#include "hardware.h"
#include "parallel.h"
#include "SPI.h"

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void delay_ms(unsigned int n_ms) {
    unsigned int ii1, ii0;
    for(ii0=n_ms; ii0>0; ii0--) {
        ii1 = 0x07FF;                    // Delay
        do (ii1--);
        while (ii1 != 0);
    }
}
// end of delay_ms

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void CounterSet(void)
{
	TACTL |= TACLR;
	TACTL &= ~TACLR;			//reset the timerA
	TACTL |= TASSEL0 + ID1 + ID0;		//ACLK, div 8, interrupt enable, timer stoped
	
	TAR = 0x0000;
	TACCTL0 |= CCIE;			//compare interrupt enable
}//CounterSet()

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void OSCsel(unsigned char mode)
{

  	unsigned int ii1;

  	if(mode == 0x00){               //select crystal oscilator
    		BCSCTL1 |= XTS + XT2OFF;                       // ACLK = LFXT1 HF XTAL
                BCSCTL3 |= LFXT1S1;                       // 3 – 16MHz crystal or resonator
		// turn external oscillator on
    		do
    		{
      			IFG1 &= ~OFIFG;                   // Clear OSCFault flag
      			for (ii1 = 0xFF; ii1 > 0; ii1--); // Time delay for flag to set
    		}
    		while ((IFG1 & OFIFG) == OFIFG);    // OSCFault flag still set?
    		BCSCTL2 |= SELM1 + SELM0 + SELS;           //  MCLK = SMCLK = HF LFXT1 (safe)

    		return;
    	}

  	else{                           //select DCO for main clock
    		DCOCTL |= DCO0 + DCO1 + DCO2;
    		BCSCTL1 |= XT2OFF + XTS + RSEL0 + RSEL1 + RSEL2;
   		
		// turn external oscillator on
//    		do
//    		{
//      		IFG1 &= ~OFIFG;                   // Clear OSCFault flag
//      		for (ii1 = 0xFF; ii1 > 0; ii1--); // Time delay for flag to set
//    		}
//    		while ((IFG1 & OFIFG) == OFIFG);    // OSCFault flag still set?
    		BCSCTL2 &= ~(SELM1 + SELM0 + SELS + DCOR);

    		return;
    	}
    	//_BIC_SR(OSCOFF);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */

#pragma vector=TIMERA0_VECTOR
__interrupt void TimerAhandler(void)
{	
	//unsigned char Register;
          unsigned char Register[2];

         //kputchar('V');
	stopCounter;
	
	//Register = IRQStatus;		//IRQ status register address
	//irqCLR;				//PORT2 interrupt flag clear
	//ReadSingle(&Register, 1);	//function call for single address read
					//IRQ status register has to be read
        Register[0] = IRQStatus;	/* IRQ status register address */
        Register[1] = IRQMask;		//Dummy read	
	//ReadSingle(Register, 2);	/* function call for single address read */
        //ReadCont(Register, 1);
        ReadCont(Register, 2);
       // ReadSingle(Register, 1);



        *Register = *Register & 0xF7;	//set the parity flag to 0

	
	//if(Register == 0x00)
        if(*Register == 0x00 || *Register == 0x80) //added code
          	//if(RXTXstate > 1)
                //  	i_reg = 0xFF;
        	//else
			i_reg = 0x00;
	else
		i_reg = 0x01;
	
	__low_power_mode_off_on_exit();
	
}//TimerAhandler
