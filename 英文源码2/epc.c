#include "epc.h"

/*
 =======================================================================================================================
    This is the inventory function for the EPC protocol. // ;
    It searches the tags in the field starting with Mask // ;
    lenght 0 - sending out no mask. A mask can be added if// ;
    a collision is detected within a response. The mask is// ;
    bit oriented, the mask lenght is the number of bits in// ;
    the mask that is used. // ;
 =======================================================================================================================
 */
void BeginRound(unsigned char MaskLenght, unsigned char *Mask, unsigned char slotNo)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	unsigned char	ByteNumber, command, i, j;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	ByteNumber = 3 + (MaskLenght >> 3); /* nuber of complete bytes */

	buf[0] = 0x8f;
	buf[1] = 0x91;						/* send with CRC */
	buf[2] = 0x3d;						/* write continous from 1D */
	buf[3] = (ByteNumber >> 4);
	buf[4] = (ByteNumber << 4);			/* there are no broken bytes */
	buf[5] = 0x30;						/* command code */
	buf[6] = MaskLenght;
	if(MaskLenght > 0)
	{
		for(i = 0; i < (MaskLenght >> 3); i++) buf[i + 7] = *(Mask + i);
		if((MaskLenght & 0x07) != 0x00) /* add one byte for broken byte */
			buf[i + 8] = *(Mask + i + 1);
	}						/* if */

	buf[i + 9] = slotNo;

	RAWwrite(&buf[0], 11);	/* writing to FIFO */
	irqCLR;					/* PORT2 interrupt flag clear */
	irqON;

	LPM0;					/* wait for end of TX interrupt */

	for(i = 0; i <= slotNo; i++)
	{
		RXTXstate = 1;		/* prepare the global counter */
		i_reg = 0x01;

		while((i_reg == 0x01) && (j < 2))
		{					/* wait for response or timer interrupt */
			j++;

			CounterSet();	/* TimerA set */
			countValue = 0x0401;	/* 1.21ms */
			startCounter;			/* start timer up mode */
			LPM0;
		}

		if(i_reg == 0xFF)
		{					/* recieved UID in buffer */
			kputchar('[');
			for(j = 1; j < RXTXstate; j++)
			{
				Put_byte(buf[j]);
			}				/* for */

			kputchar(']');
		}
		else if(i_reg == 0x02)
		{					/* collision occured */
			kputchar('[');
			kputchar('z');
			kputchar(']');
		}
		else if(i_reg == 0x00)
		{					/* timer interrupt */
			kputchar('[');
			kputchar(']');
		}
		else
			;

		command = Reset;	/* FIFO has to be reset before recieving the next response */
		DirectCommand(&command);
	}						/* for */

	irqOFF;
}	/* BeginRound */

/*
 =======================================================================================================================
    Send a request, recieve a response and sent another requrest with ;
    the delayed send command.
 =======================================================================================================================
 */
unsigned char RequestEPC(unsigned char *pbuf, unsigned char lenght)
{
	/*~~~~~~~~~~~~~~~~~~~~~*/
	unsigned char	index, j;
	/*~~~~~~~~~~~~~~~~~~~~~*/

	RXTXstate = lenght; /* RXTXstate global wariable is the main transmit counter */

	*pbuf = 0x8f;
	*(pbuf + 1) = 0x91; /* buffer setup for FIFO writing */
	*(pbuf + 2) = 0x3d;
	*(pbuf + 3) = RXTXstate >> 4;
	*(pbuf + 4) = RXTXstate << 4;

	if(lenght > 12) lenght = 12;

	RAWwrite(pbuf, lenght + 5);		/* send the request using RAW writing */

	/* Write 12 bytes the first time you write to FIFO */
	irqCLR;							/* PORT2 interrupt flag clear */
	irqON;

	RXTXstate = RXTXstate - 12;
	index = 18;

	i_reg = 0x01;

	while(RXTXstate > 0)
	{
		LPM0;						/* enter low power mode and exit on interrupt */
		if(RXTXstate > 9)
		{							/* the number of unsent bytes is in the RXTXstate global */
			lenght = 10;			/* count variable has to be 10 : 9 bytes for FIFO and 1 address */
		}
		else if(RXTXstate < 1)
		{
			break;					/* return from interrupt if all bytes have been sent to FIFO */
		}
		else
		{
			lenght = RXTXstate + 1; /* all data has been sent out */
		}						/* if */

		buf[index - 1] = FIFO;	/* writes 9 or less bytes to FIFO for transmitting */
		WriteCont(&buf[index - 1], lenght);
		RXTXstate = RXTXstate - 9;	/* write 9 bytes to FIFO */
		index = index + 9;
	}						/* while */

	RXTXstate = 1;			/* the response will be stored in buf[1] upwards */
	while(i_reg == 0x01)
	{
	}

	i_reg = 0x01;

	CounterSet();
	countValue = 0xF000;	/* 60ms for TIMEOUT */
	startCounter;			/* start timer up mode */

	while(i_reg == 0x01)
	{
	}						/* wait for RX complete */

	buf[50] = 0x8f;
	buf[51] = 0x92;
	buf[52] = 0x3d;
	buf[53] = 0x00;
	buf[54] = 0x20;
	buf[55] = 0x00;
	buf[56] = 0xFF;

	RAWwrite(&buf[50], 7);

	if(i_reg == 0xFF)
	{						/* recieved response */
		kputchar('[');
		for(j = 1; j < RXTXstate; j++)
		{
			Put_byte(buf[j]);
		}					/* for */

		kputchar(']');
		return(0);
	}
	else if(i_reg == 0x02)
	{						/* collision occured */
		kputchar('[');
		kputchar('z');
		kputchar(']');
		return(0);
	}
	else if(i_reg == 0x00)
	{						/* timer interrupt */
		kputchar('[');
		kputchar(']');
		return(1);
	}
	else
		;

	irqOFF;
	return(1);
}	/* Request14443A */

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void CSScommand(void)
{
	/*~~~~~~~~~~~~~~*/
	unsigned char	i;
	/*~~~~~~~~~~~~~~*/

	STARTcondition();
	P2OUT = 0x95;	/* send CSS command */
	clkON;
	clkOFF;
	P2OUT = 0x90;	/* send without CRC */
	clkON;
	clkOFF;
	for(i = 0; i < 155; i++);

	STOPcondition();
}					/* CSScommand */
