#include "anticollision.h"
unsigned char	POLLING;

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void EnableSlotCounter(void)
{
	buf[41] = IRQMask;	/* next slot counter */
	buf[40] = IRQMask;
	ReadSingle(&buf[41], 1);
	buf[41] |= BIT0;	/* set BIT0 in register 0x01 */
	WriteSingle(&buf[40], 2);

}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void DisableSlotCounter(void)
{
	buf[41] = IRQMask;	/* next slot counter */
	buf[40] = IRQMask;
	ReadSingle(&buf[41], 1);
	buf[41] &= 0xfe;	/* clear BIT0 in register 0x01 */
	WriteSingle(&buf[40], 2);
}

/*
 =======================================================================================================================
    Function InventoryRequest() performs an invertory cycle of 16 // ;
    timeslots or 1 timeslot for the ISO15693 standard. // ;
    0x14 - 16 timeslots // ;
    0x17 - 1 timeslot // ;
    The recieved UIDs or error messages are sent to the host // ;
    using the function PrintUIDs() at the end of this function. // ;
 =======================================================================================================================
 */
void InventoryRequest(unsigned char *mask, unsigned char lenght)	/* host command 0x14 */
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* 010800030414(req.packet)[00ff] */
	unsigned char	i = 1, j = 3, command[2], NoSlots, found = 0;
	unsigned char	*PslotNo, slotNo[17];
	unsigned char	NewMask[8], NewLenght, masksize;
	int				size;
	unsigned int	k = 0;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/



         //added code

	buf[0] = ModulatorControl;
	buf[1] = 0x21;
	WriteSingle(buf, 2);
       //end added code

        //If Low data rate and SPI mode is used the RXNOResponseWaitTime needs to be updated
        if (SPIMODE)
        {
          if ( (flags & BIT1) == 0x00)//low data rate
          {

	        buf[0] = RXNoResponseWaitTime;
        	buf[1] = 0x2F;
	        WriteSingle(buf, 2);
          }
           else //high data rate
          {
                buf[0] = RXNoResponseWaitTime;
        	buf[1] = 0x13;
	        WriteSingle(buf, 2);

          }
        }

	slotNo[0] = 0x00;

	if((flags & BIT5) == 0x00)
	{						/* flag bit5 is the number of slots indicator */
		NoSlots = 17;		/* 16 slots if bit is cleared */
		EnableSlotCounter();
	}
	else
		NoSlots = 2;		/* 1 slot if bit is set */

	PslotNo = &slotNo[0];	/* slot number pointer */

	masksize = (((lenght >> 2) + 1) >> 1);	/* masksize is 1 for lenght = 4 or 8 */

	/*
	 * masksize is 2 for lenght = 12 or 16 ;
	 * and so on
	 */
	size = masksize + 3;					/* mask value + mask lenght + command code + flags */


	buf[0] = 0x8f;
	buf[1] = 0x91;						/* send with CRC */
	buf[2] = 0x3d;						/* write continous from 1D */
	buf[3] = (char) (size >> 8);
	buf[4] = (char) (size << 4);
	buf[5] = flags;						/* ISO15693 flags */
	buf[6] = 0x01;						/* anticollision command code */

        //optional AFI should be here


	buf[7] = lenght;					/* masklenght */
	if(lenght > 0)
	{
		for(i = 0; i < masksize; i++) buf[i + 8] = *(mask + i);
	}									/* if */

	command[0] = IRQStatus;
        command[1] = IRQMask;		//Dummy read
        //ReadCont(command, 2);
        ReadCont(command, 1);
	//ReadSingle(command, 1);

        CounterSet();						/* TimerA set */
	countValue = count1ms * 20;			/* 20ms */
        irqCLR;							/* PORT2 interrupt flag clear */
	irqON;

      	 RAWwrite(&buf[0], masksize + 8);	/* writing to FIFO */

	
	i_reg = 0x01;
	startCounter;					/* start timer up mode */
	LPM0;							/* wait for end of TX interrupt */

	for(i = 1; i < NoSlots; i++)
	{								/* 1 or 16 available timeslots */
		RXTXstate = 1;				/* prepare the global counter */

		/* the first UID will be stored from buf[1] upwards */
		CounterSet();				/* TimerA set */
		//countValue = count1ms * 20; /* 20ms */
                countValue = 0x4E20;	
                startCounter;				/* start timer up mode */

		k = 0;
		LPM0;

		while(i_reg == 0x01)
		{						/* wait for RX complete */
			k++;

			if(k == 0xFFF0)
			{
				i_reg = 0x00;
				RXErrorFlag = 0x00;
				break;
			}
		}

		command[0] = RSSILevels;	/* read RSSI levels */
		ReadSingle(command, 1);

		if(i_reg == 0xFF)
		{						/* recieved UID in buffer */
			if(POLLING)
			{
				found = 1;
			}
			else
			{
				kputchar('[');
				for(j = 3; j < 11; j++)
				{
					Put_byte(buf[j]);
				}				/* for */

				kputchar(',');
				Put_byte(command[0]);	/* RSSI levels */
				kputchar(']');


			}
		}
		else if(i_reg == 0x02)
		{	/* collision occured */
			if(!POLLING)
			{
				kputchar('[');
				kputchar('z');
				kputchar(',');
				Put_byte(command[0]);	/* RSSI levels */
				kputchar(']');
			}

			PslotNo++;
			*PslotNo = i;
		}
		else if(i_reg == 0x00)
		{	/* timer interrupt */
			if(!POLLING)
			{
				kputchar('[');
				kputchar(',');
				Put_byte(command[0]);	/* RSSI levels */
				kputchar(']');
			}
		}
		else
			;

		command[0] = Reset;			/* FIFO has to be reset before recieving the next response */
		DirectCommand(command);

		if((NoSlots == 17) && (i < 16))
		{					/* if 16 slots used send EOF(next slot) */
			command[0] = StopDecoders;
			DirectCommand(command);
			command[0] = RunDecoders;
			DirectCommand(command);
                        command[0] = TransmitNextSlot;
			DirectCommand(command);

		}
		else if((NoSlots == 17) && (i == 16))
		{					/* at the end of slot 16 stop the slot counter */
			DisableSlotCounter();
		}
		else if(NoSlots == 2)
			break;

		if(!POLLING)
		{
			put_crlf();
		}
	}						/* for */

	if(found)
	{						/* turn on LED */
		LED15693ON;
	}
	else
	{
		LED15693OFF;
	}

	NewLenght = lenght + 4; /* the mask lenght is a multiple of 4 bits */

	masksize = (((NewLenght >> 2) + 1) >> 1) - 1;

	while((*PslotNo != 0x00) && (NoSlots == 17))
	{
		*PslotNo = *PslotNo - 1;

		for(i = 0; i < 8; i++) NewMask[i] = *(mask + i);	/* first the whole mask is copied */

		if((NewLenght & BIT2) == 0x00) *PslotNo = *PslotNo << 4;

		/*
		 * Put_byte(*PslotNo);
		 * *put_crlf();
		 */
		NewMask[masksize] |= *PslotNo;						/* the mask is changed */

		InventoryRequest(&NewMask[0], NewLenght);			/* recursive call */

		PslotNo--;
	}	/* while */

	irqOFF;
}		/* InventoryRequest */

/*
 =======================================================================================================================
    The function RequestCommand() is used for request and // ;
    response handling and timing for VCD to VICC // ;
    communication. // ;
    Host command = 0x18 // ;
 =======================================================================================================================
 */
unsigned char RequestCommand(unsigned char *pbuf, unsigned char lenght, unsigned char brokenBits, char noCRC)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	unsigned char	index, j, command;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	RXTXstate = lenght; /* RXTXstate global wariable is the main transmit counter */

	*pbuf = 0x8f;
	if(noCRC) *(pbuf + 1) = 0x90;	/* buffer setup for FIFO writing */
	else
		*(pbuf + 1) = 0x91;			/* buffer setup for FIFO writing */
	*(pbuf + 2) = 0x3d;
	*(pbuf + 3) = RXTXstate >> 4;
	*(pbuf + 4) = (RXTXstate << 4) | brokenBits;

	if(lenght > 12) lenght = 12;

	if(lenght == 0x00 && brokenBits != 0x00)
	{
		lenght = 1;
		RXTXstate = 1;
	}

	RAWwrite(pbuf, lenght + 5);		/* send the request using RAW writing */

	/* Write 12 bytes the first time you write to FIFO */
	irqCLR;					/* PORT2 interrupt flag clear */
	irqON;

	RXTXstate = RXTXstate - 12;
	index = 17;

	i_reg = 0x01;

	while(RXTXstate > 0)
	{
		LPM0;				/* enter low power mode and exit on interrupt */
		if(RXTXstate > 9)
		{					/* the number of unsent bytes is in the RXTXstate global */
			lenght = 10;	/* count variable has to be 10 : 9 bytes for FIFO and 1 address */
		}
		else if(RXTXstate < 1)
		{
			break;			/* return from interrupt if all bytes have been sent to FIFO */
		}
		else
		{
			lenght = RXTXstate + 1; /* all data has been sent out */
		}						/* if */

		buf[index - 1] = FIFO;	/* writes 9 or less bytes to FIFO for transmitting */
		WriteCont(&buf[index - 1], lenght);
		RXTXstate = RXTXstate - 9;	/* write 9 bytes to FIFO */
		index = index + 9;
	}				/* while */

	RXTXstate = 1;	/* the response will be stored in buf[1] upwards */

	/* wait for end of transmit */
	while(i_reg == 0x01)
	{
		CounterSet();
		countValue = 0xF000;	/* 60ms for TIMEOUT */
		startCounter;			/* start timer up mode */
		LPM0;
	}

	i_reg = 0x01;

	CounterSet();
	countValue = count1ms * 60; /* 60ms for TIMEOUT */
	startCounter;				/* start timer up mode */

	if
	(
		(((buf[5] & BIT6) == BIT6) && ((buf[6] == 0x21) || (buf[6] == 0x24) || (buf[6] == 0x27) || (buf[6] == 0x29)))
	||	(buf[5] == 0x00 && ((buf[6] & 0xF0) == 0x20 || (buf[6] & 0xF0) == 0x30 || (buf[6] & 0xF0) == 0x40))
	)
	{
		delay_ms(20);
		command = Reset;
		DirectCommand(&command);
		command = TransmitNextSlot;
		DirectCommand(&command);
	}				/* if */

	while(i_reg == 0x01)
	{
	}				/* wait for RX complete */

	if(!POLLING)
	{
		switch(noCRC)
		{
		case 0:
			if(i_reg == 0xFF)
			{		/* recieved response */
				kputchar('[');
				for(j = 1; j < RXTXstate; j++)
				{
					Put_byte(buf[j]);
				}	/* for */

				kputchar(']');
				return(0);
			}
			else if(i_reg == 0x02)
			{		/* collision occured */
				kputchar('[');
				kputchar('z');
				kputchar(']');
				return(0);
			}
			else if(i_reg == 0x00)
			{		/* timer interrupt */
				kputchar('[');
				kputchar(']');
				return(1);
			}
			else
				;
			break;

		case 1:
			if(i_reg == 0xFF)
			{		/* recieved response */
				kputchar('(');
				for(j = 1; j < RXTXstate; j++)
				{
					Put_byte(buf[j]);
				}	/* for */

				kputchar(')');
				return(0);
			}
			else if(i_reg == 0x02)
			{		/* collision occured */
				kputchar('(');
				kputchar('z');
				kputchar(')');
				return(0);
			}
			else if(i_reg == 0x00)
			{		/* timer interrupt */
				kputchar('(');
				kputchar(')');
				return(1);
			}
			else
				;
			break;
		}			/* switch */
	}				/* if */

	irqOFF;
	return(1);
}					/* RequestCommand */



