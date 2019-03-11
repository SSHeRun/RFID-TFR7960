#include "hardware.h"
#include "parallel.h"
#include "SPI.h"
#include "anticollision.h"
#include "globals.h"
#include "tiris.h"
#include "14443.h"
#include "host.h"
#include "epc.h"
#include "automatic.h"

unsigned char	RXdone;
unsigned char	ENABLE;
unsigned char	FirstSPIdata = 1;

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void UARTset(void) //Uses USCI_A0
{
  P3SEL |= BIT4 + BIT5;	/* P3.4, P3.5 UART mode */
  P3DIR |= BIT4;			/* P3.4 - output */

  UCA0CTL1 |= UCSWRST;			/* disable UART */

  UCA0CTL0 = 0x00;
//  UCA0CTL0 |= UCMSB ;
  UCA0CTL1 |= UCSSEL_2;                     // SMCLK
  UCA0BR0 = BAUD0;                              // 115200
  UCA0BR1 = BAUD1;
  UCA0MCTL = 0;               // Modulation UCBRSx = 5
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt

}

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void USARTset(void)  //Uses USCI_B0
{

  UCB0CTL1 |= UCSWRST;                     // Disable USCI first
  UCB0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC;  // 3-pin, 8-bit SPI master
  UCB0CTL1 |= UCSSEL_2;                     // SMCLK

  UCB0BR0 = 0;
  UCB0BR1 = 0;
  P3SEL |= BIT1 + BIT2 + BIT3;                            // P3.1,3.2,3.3 UCB0SIMO,UCB0SOMI,UCBOCLK option select

  SlaveSelectPortSet  // P3.0 - Slave Select
  SlaveSelectHIGH     // Slave Select - inactive ( high)

  UCB0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**

 // IE2 |= UCB0RXIE + UCB0TXIE;	/* transmitter and reciever enable */

  //IE2 |= UCB0RXIE ;	/*  reciever enable */

  //Harsha - code added
 // IFG2 &= ~UCB0RXIFG; //Clear Interrupt flag first
 // UCB0TXBUF=0x00; //dummy write to clear IFG

}

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void USARTEXTCLKset(void)  //Uses USCI_B0
{

    UCB0CTL1 |= UCSWRST;                     // Disable USCI first
    UCB0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC;  // 3-pin, 8-bit SPI master


  UCB0CTL1 |= UCSSEL_2;                     // SMCLK
  //UCB0BR0 = 0x02;
  UCB0BR0 = 0x00;
  UCB0BR1 = 0;


  P3SEL |= BIT1 + BIT2 + BIT3;                            // P3.1,3.2,3.3 UCB0SIMO,UCB0SOMI,UCBOCLK option select

  SlaveSelectPortSet  // P3.0 - Slave Select
  SlaveSelectHIGH     // Slave Select - inactive ( high)

  UCB0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**

 // IE2 |= UCB0RXIE + UCB0TXIE;	/* transmitter and reciever enable */

//  IE2 |= UCB0RXIE ;	/*  reciever enable */

  //Harsha - code added
 // IFG2 &= ~UCB0RXIFG; //Clear Interrupt flag first
 // UCB0TXBUF=0x00; //dummy write to clear IFG

}


/*
 =======================================================================================================================
 =======================================================================================================================
 */


void BaudSet(unsigned char mode)
{
	if(mode == 0x00)
	{
		UCA0BR0 = BAUD0;	/* baud rate register */
		UCA0BR1 = BAUD1;
	}
	else
	{
		UCA0BR0 = BAUD0EN;	/* baud rate register */
		UCA0BR1 = BAUD1EN;
	}
}	/* BaudSet */

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void kputchar(char TXchar)
{
	while(!(IFG2 & UCA0TXIFG));

	/* wait for TX register to be empty */
	UCA0TXBUF = TXchar;	/* send the character */
}						/* putchar */

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void put_bksp(void)
{
	kputchar('\b');
	kputchar(' ');
	kputchar('\b');
}	/* put_bksp */

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void put_space(void)
{
	kputchar(' ');
}	/* put_space */

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void put_crlf(void)
{
	kputchar('\r');
	kputchar('\n');
}	/* put_crlf */

/*
 =======================================================================================================================
    Send a character string to USART ;
 =======================================================================================================================
 */
void send_cstring(char *pstr)
{
	while(*pstr != '\0')
	{
		kputchar(*pstr++);
	}
}

/*
 =======================================================================================================================
    convert a nibble to ASCII hex byte ;
 =======================================================================================================================
 */
unsigned char Nibble2Ascii(unsigned char anibble)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	unsigned char	AsciiOut = anibble;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	if(anibble > 9) /* If req ASCII A-F then add 7(hex) */
		AsciiOut = AsciiOut + 0x07;

	/* Add offset to convert to Ascii */
	AsciiOut = AsciiOut + 0x30;

	return(AsciiOut);
}

/*
 =======================================================================================================================
    end of Nibble2Ascii output a binary coded byte as two hex coded ascii bytes ;
 =======================================================================================================================
 */
void Put_byte(unsigned char abyte)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~*/
	unsigned char	temp1, temp2;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~*/

	temp1 = (abyte >> 4) & 0x0F;	/* get high nibble */
	temp2 = Nibble2Ascii(temp1);	/* convert to ASCII */
	kputchar(temp2);				/* output */

	temp1 = abyte & 0x0F;			/* get low nibble */
	temp2 = Nibble2Ascii(temp1);	/* convert to ASCII */
	kputchar(temp2);				/* output */
}

/*
 =======================================================================================================================
    end of Put_byte get a hex coded nibble
 =======================================================================================================================
 */
unsigned char Get_nibble(void)
{
	/*~~~~~~~~~~~~~~~~~~~~*/
	unsigned char	reading;
	unsigned char	rxdata;
	/*~~~~~~~~~~~~~~~~~~~~*/

	reading = 1;				/* flag: reading not yet finished */
	while(reading)
	{							/* loop and read characters */
		LPM0;					/* sync, wakeup by irq */
		if(rxdata >= 'a')
		{
			rxdata -= 32;
		}						/* change to uppercase */

		/* echo if hex */
		if(((rxdata >= '0') && (rxdata <= '9')) || ((rxdata >= 'A') && (rxdata <= 'F')))
		{
			reading = 0;
			kputchar(rxdata);	/* echo */
			if(rxdata > '9')
			{					/* If ASCII A-F then add 9 */
				rxdata = (rxdata & 0x0F) + 9;
			}
		}

		/* else discard */
	}

	return(rxdata);
}

/*
 =======================================================================================================================
    end of Get_nibble get a line of characters ;
    returns 0 if no error, <> 0 if error input ;
    allows only "0,..,9","A,..,F" or "a,..,f", <CR>,<LF>,<backspace> ;
    ignores other characters ;
 =======================================================================================================================
 */
unsigned char Get_line(unsigned char *pline)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	unsigned char	reading, err_flg;
	unsigned char	pos;			/* counts number of received nibbles */
	unsigned char	Length_Byte;	/* max 256 bytes allowed (512 nibbles) */
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	err_flg = 0;					/* assume no error */
	Length_Byte = 0xff;				/* assume max number of bytes */

	/* wait for SOF: '0' followed by '1' */
	if(!FirstSPIdata)
	{
		LPM0;						/* sync, wakeup by irq */
		while(rxdata != '0')
		{
		}
	}
	else
	{
		FirstSPIdata = 0;
	}

	kputchar('0');
	LPM0;										/* sync, wakeup by irq */
	while(rxdata != '1')
	{
	}

	kputchar('1');

	RXdone = 0;

	pos = 0;									/* character position counter - 8bit */
	reading = 1;								/* flag: reading not yet finished */
	while(reading)
	{											/* loop and read characters */
		while(RXdone == 0);

		/* sync, wakeup by irq */
		RXdone = 0;
		switch(rxdata)
		{
		/* process RETURN key */
		case '\r':
			break;								/* ignore CR */

		case '\n':
			reading = 0;						/* exit read loop */
			break;

		/* backspace */
		case '\b':
			if(pos > 0)
			{									/* is there a char to delete? */
				pos--;							/* remove it from buffer */
				put_bksp();						/* go back and erase on screen */
				if(pos & 0x01 > 0)
				{								/* (high) even byte */
					*pline--;
					*pline &= 0xF0;				/* clear lo nibble */
				}
			}
			break;

		/* other characters */
		default:
			if(rxdata >= 'a')
			{
				rxdata -= 32;
			}									/* change to uppercase */

			/* discard if not hex */
			if((rxdata < '0') || ((rxdata > '9') && (rxdata < 'A')) || (rxdata > 'F'))
			{
				break;
			}

			/* only store characters if buffer has space */
			if(pos++ < 2 * BUF_LENGTH)
			{
				kputchar(rxdata);				/* echo */
				if(rxdata > '9')
				{								/* If ASCII A-F then add 9 */
					rxdata = (rxdata & 0x0F) + 9;
				}

				if((pos & 0x01) == 0)
				{								/* (low) odd nibble */
					*pline += (rxdata & 0x0F);	/* store */
					if(pos == 2)
					{
						/*
						 * just finished receiving 2 nibbles containing number of expected data bytes ;
						 * change Length_Bytes (total number of expected data bytes)
						 */
						Length_Byte = *pline;
					}

					pline++;
					if(((Length_Byte - 1) * 2) == pos)
					{
						reading = 0;			/* flag loop exit - done */
					}
				}
				else
				{	/* (high) even nibble */
					rxdata = rxdata << 4;	/* Move to high nibble */
					*pline = rxdata;		/* store */
				}
			}
			else
				err_flg = 1;
		}
	}

	return(err_flg);						/* normal exit */
}

/* end of Get_line */


/////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//Common Interrupt RX Vector for both USCIA - UART & USCIB - SPI

#pragma vector = USCIAB0RX_VECTOR
__interrupt void RXhandler (void)
{
   if (IFG2 & UCA0RXIFG)  //UART
   {	rxdata = UCA0RXBUF;
	RXdone = 1;
	if(ENABLE == 0)
	{
		TRFEnable;
		BaudSet(0x01);
		OSCsel(0x01);
		InitialSettings();
		send_cstring("Reader enabled.");
		ENABLE = 1;
	}
	__low_power_mode_off_on_exit();

	if(FirstSPIdata)
	{
		irqOFF;
		stopCounter;
		asm("mov.w #HostCommands,10(SP)");
                // This manipulation of SP is needed so that the control transfers to the
                //HostCommand function after the interrupt return
	}

   }


//   else if (IFG2 & UCB0RXIFG)  //SPI
 //  {
 //    unsigned char dontcare;
 //    dontcare=UCB0RXBUF;
 //    Do Nothing
 //  }

}					/* RXhandler */

/////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//Common Interrupt TX Vector for both USCIA - UART & USCIB - SPI

//#pragma vector = USCIAB0TX_VECTOR
//__interrupt void TXhandler (void)
//{
  /* if (IFG2 & UCB0TXIFG)
   {
     //Do Nothing - Just RETI
   }*/


//}					/* TXhandler */

/////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void HostCommands ()
{
	char *phello;

	unsigned char *pbuf, count;

	POLLING = 0;

	/* main loop, never ends */
	while(1)
	{
		pbuf = &buf[0];
		Get_line(pbuf);
		put_crlf(); /* finish line */

		pbuf = &buf[4];
		RXErrorFlag = 0;

		if(*pbuf == 0xFF)
		{			/* check COM port number */
			phello = "TRF7960 EVM \r\n";
			send_cstring(phello);
		}
		else if(*pbuf == 0x10)
		{			/* register write (adress:data, adress:data, ...) */
			send_cstring("Register write request.\r\n");
			count = buf[0] - 8;
			WriteSingle(&buf[5], count);
		}
		else if(*pbuf == 0x11)
		{			/* continous write (adress:data, data, ...) */
			phello = "Continous write request.\r\n";
			send_cstring(phello);
			count = buf[0] - 8;
			WriteCont(&buf[5], count);
		}
		else if(*pbuf == 0x12)
		{			/* register read (adress:data, adress:data, ...) */
			phello = "Register read request.\r\n";
			send_cstring(phello);
			count = buf[0] - 8;
			ReadSingle(&buf[5], count);
			Response(&buf[5], count);
		}
		else if(*pbuf == 0x13)
		{			/* continous read (adress:data, data, ...) */
			send_cstring("Continous read request\r\n");
			pbuf++;
			count = *pbuf;					/* the amount of registers to be read */

			/* is speciffied after the command */
			pbuf++;
			buf[5] = *pbuf;					/* the start register is speciffied */

			/* after the amount of registers */
			ReadCont(&buf[5], count);
			Response(&buf[5], count);
		}
		else if(*pbuf == 0x14)
		{									/* inventory request */
			phello = "ISO 15693 Inventory request.\r\n";
			send_cstring(phello);
			flags = buf[5];
			for(count = 0; count < 8; count++) buf[count + 20] = 0x00;
			InventoryRequest(&buf[20], 0x00);
		}
		else if(*pbuf == 0x15)
		{									/* direct command */
			phello = "Direct command.\r\n";
			send_cstring(phello);
			DirectCommand(&buf[5]);
		}
		else if(*pbuf == 0x16)
		{									/* raw */
			phello = "RAW mode.\r\n";
			send_cstring(phello);
			count = buf[0] - 8;
			RAWwrite(&buf[5], count);
		}
		else if(*pbuf == 0x18)
		{									/* request code */
			phello = "Request mode.\r\n";
			send_cstring(phello);
			count = buf[0] - 8;
			RequestCommand(&buf[0], count, 0x00, 0);
		}
		else if(*pbuf == 0x19)
		{									/* testing 14443A - sending and recieving */
			/*
			 * in different bitrates with changing ;
			 * the ISOmode register after TX
			 */
			phello = "14443A Request - change bit rate.\r\n";
			send_cstring(phello);
			count = buf[0] - 9;
			Request14443A(&buf[1], count, buf[5]);
		}
		
		else if(*pbuf == 0x34)
		{									/* SID poll */
			phello = "Ti SID Poll.\r\n";
			send_cstring(phello);
			flags = buf[5];
			for(count = 0; count < 4; count++) buf[count + 20] = 0x00;
			TIInventoryRequest(&buf[20], 0x00);
		}
		
		else if(*pbuf == 0x0F)
		{									/* Direct mode */
			phello = "Direct mode.\r\n";
			send_cstring(phello);
			DirectMode();
		}
		else if((*pbuf == 0xB0) || (*pbuf == 0xB1))
		{									/* 0xB0 - REQB */
			phello = "14443B REQB.\r\n";	/* 0xB1 - WUPB */
			send_cstring(phello);
			AnticollisionSequenceB(*pbuf, *(pbuf + 1));
                       // AnticollisionSequenceB(0xB0, 0x00); ///* single slot
		}
		else if((*pbuf == 0xA0) || (*pbuf == 0xA1))
		{					/* 0xA0 - REQA */
			phello = "14443A REQA.\r\n";
			send_cstring(phello);
			AnticollisionSequenceA(*(pbuf + 1));
		}
		else if(*pbuf == 0xA2)
		{					/* 0xA0 - REQA */
			phello = "14443A Select.\r\n";
			send_cstring(phello);
			switch(buf[0])
			{
			case 0x0D:
				for(count = 1; count < 6; count++) buf[99 + count] = *(pbuf + count);
				break;

			case 0x11:
				for(count = 1; count < 11; count++) buf[100 + count] = *(pbuf + count);
				buf[100] = 0x88;
				break;

			case 0x15:
				for(count = 1; count < 5; count++) buf[100 + count] = *(pbuf + count);
				buf[100] = 0x88;
				buf[105] = 0x88;
				for(count = 1; count < 10; count++) buf[105 + count] = *(pbuf + count + 4);
			}				/* switch */
			buf[0] = ISOControl;
			buf[1] = 0x88;	/* recieve with no CRC */
			WriteSingle(buf, 2);

			buf[5] = 0x26;	/* send REQA command */
			if(RequestCommand(&buf[0], 0x00, 0x0f, 1) == 0)
			{
				if(SelectCommand(0x93, &buf[100]))
				{
					if(SelectCommand(0x95, &buf[105])) SelectCommand(0x97, &buf[110]);
				}
			}
		}
		else if(*pbuf == 0x03)
		{					/* enable or disable the reader chip */
			if(*(pbuf + 1) == 0x00)
			{
				/*
				 * enable;
				 * BaudSet(*(pbuf + 1));
				 * OSCsel(*(pbuf + 1));
				 * InitialSettings();
				 * send_cstring("Reader enabled.");
				 * ENABLE = 1;
				 */
			}
			else if(*(pbuf + 1) == 0xFF)
			{
				BaudSet(*(pbuf + 1));
				OSCsel(*(pbuf + 1));
				TRFDisable;
				send_cstring("Reader disabled.");
				ENABLE = 0;
			}
			else if(*(pbuf + 1) == 0x0A)
			{
				BaudSet(0x00);
				OSCsel(0x00);
				send_cstring("External clock.");
			}
			else if(*(pbuf + 1) == 0x0B)
			{
				BaudSet(0x01);
				OSCsel(0x01);
				send_cstring("Internal clock.");
			}
			else
			{
			}
		}
		else if(*pbuf == 0xF0)
		{					/* AGC toggle */
			buf[0] = ChipStateControl;
			buf[1] = ChipStateControl;
			ReadSingle(&buf[1], 1);
			if(*(pbuf + 1) == 0xFF)
				buf[1] |= BIT2;
			else
				buf[1] &= ~BIT2;
			WriteSingle(buf, 2);
		}
		else if(*pbuf == 0xF1)
		{					/* AM PM toggle */
			buf[0] = ChipStateControl;
			buf[1] = ChipStateControl;
			ReadSingle(&buf[1], 1);
			if(*(pbuf + 1) == 0xFF)
				buf[1] &= ~BIT3;
			else
				buf[1] |= BIT3;
			WriteSingle(buf, 2);
		}
		else if(*pbuf == 0xF2)
		{					/* Full - half power selection (FF - full power) */
			buf[0] = ChipStateControl;
			buf[1] = ChipStateControl;
			ReadSingle(&buf[1], 1);
			if(*(pbuf + 1) == 0xFF)
				buf[1] &= ~BIT4;
			else
				buf[1] |= BIT4;
			WriteSingle(buf, 2);
		}
		else if(*pbuf == 0xFE)
		{					/* Firmware Version Number */
			phello = "Firmware Version 3.2.EXP.NOBB \r\n";
			send_cstring(phello);
		}
		else
		{
			phello = "Unknown command.\r\n";
			send_cstring(phello);
		}					/* end if */

		while(!(IFG2 & UCA0TXIFG));
	}						/* end while(1) */
}	/* HostCommands */


