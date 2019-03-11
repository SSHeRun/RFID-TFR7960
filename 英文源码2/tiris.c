#include "tiris.h"

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void TIInventoryRequest(unsigned char *mask, unsigned char length)	/* host command 0x34 */
																/* 010800030414(req.packet)[00ff] */
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	unsigned char	i = 1, j = 3, command, found = 0,command3[2];
	unsigned char	*PslotNo, slotNo[17];
	unsigned char	NewMask[8], Newlength, masksize;
	int				size;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/



        /*buf[0] = RXNoResponseWaitTime;
       buf[1] = 0x14;
	WriteSingle(buf, 2);*/

       //added code
        buf[0] = RXNoResponseWaitTime;
	buf[1] = 0x14;
	buf[2] = ModulatorControl;
	buf[3] = 0x21;
	WriteSingle(buf, 4);
       //end added code



	slotNo[0] = 0x00;
	EnableSlotCounter();
	PslotNo = &slotNo[0];		/* we will use the allready defined buffer for storing */

	/*
	 * collision position ;
	 * x = 17 16 slots ;
	 * x = 2 1 slot
	 */
	masksize = (((length >> 2) + 1) >> 1);
	size = masksize + 3;		/* mask value + mask length + command code + flags */

	buf[0] = 0x8f;
	buf[1] = 0x91;				/* send with CRC */
	buf[2] = 0x3d;				/* write continous from 1D */
	buf[3] = (char) (size >> 4);
	buf[4] = (char) (size << 4);
	buf[5] = 0x00;
	buf[6] = 0x50;				/* SID_Pol Command Code */

	/*
	 * !!! buf[7] = (length | buf[19]);
	 * //Add Info_Flag=buf[19]=buf[*3]
	 */
	buf[7] = (length | 0x80);	/* Add Info_Flag=1 */
	if(length > 0)
	{
		for(i = 0; i < masksize; i++) buf[i + 8] = *(mask + i);
	}	/* if */

	if(length & 0x04)
	{	/* broken byte correction */
		buf[4] = (buf[4] | 0x09);						/* 4 bits */
		buf[masksize + 7] = (buf[masksize + 7] << 4);	/* move 4 bits to MSB part */
	}



	CounterSet();						/* TimerA set */
	countValue = count1ms * 20;			/* 20ms */
	i_reg = 0x01;
	irqCLR;								/* PORT2 interrupt flag clear */
	RAWwrite(&buf[0], masksize + 8);	/* writing to FIFO */
	irqON;
	startCounter;						/* start timer up mode */
	LPM0;					/* wait for end of TX interrupt */

	for(i = 1; i < 17; i++) //16 slot
       	{
		RXTXstate = 1;		/* prepare the global counter */

		/* the first SID will be stored from buf[1] upwards */
		i_reg = 0x01;
		j = 0;
		while((i_reg == 0x01) && (j < 2))
		{					/* wait for RX complete */
			j++;
			CounterSet();	/* TimerA set */
			countValue = count1ms * 20; /* 20ms */
			startCounter;				/* start timer up mode */
			LPM0;
		}



		if(i_reg == 0xFF)
		{					/* recieved SID in buffer */
			if(POLLING)
			{
				found = 1;
			}
			else
			{
				kputchar('[');
				for(j = 1; j < 11; j++) Put_byte(buf[j]);
				kputchar(']');
			}
		}
		else if(i_reg == 0x02)
		{					/* collision occured */
			if(!POLLING)
			{
				kputchar('[');
				kputchar('z');
				kputchar(']');
			}

			PslotNo++;
			*PslotNo = i;
		}
		else if(i_reg == 0x00)
		{					/* slot timeout */
			if(!POLLING)
			{
				kputchar('[');
				kputchar(']');
			}
		}
		else
			;

		command = Reset;
		DirectCommand(&command);

		if(i < 16)
		{					/* if less than 16 slots used send EOF(next slot) */
			command = TransmitNextSlot;
			DirectCommand(&command);
		}

		if(!POLLING)
		{
			put_crlf();
		}
	}/* for */

	DisableSlotCounter();

	if(found)
	{
		LEDtagitON;
	}
	else
	{
		LEDtagitOFF;
	}

	Newlength = length + 4; /* the mask length is a multiple of 4 bits */

	masksize = (((Newlength >> 2) + 1) >> 1) - 1;

	while(*PslotNo != 0x00)
	{
		*PslotNo = *PslotNo - 1;	/* the slot counter counts from 1 to 16, */

		/* but the slot numbers go from 0 to 15 */
		for(i = 0; i < 4; i++) NewMask[i] = *(mask + i);	/* first the whole mask is copied */

		if((Newlength & BIT2) == 0x00) *PslotNo = *PslotNo << 4;

		/*
		 * Put_byte(*PslotNo);
		 * *put_crlf();
		 */
		NewMask[masksize] |= *PslotNo;						/* the mask is changed */

		TIInventoryRequest(&NewMask[0], Newlength);			/* recursive call */

		PslotNo--;
	}	/* while */

	irqOFF;
}		/* TIInventoryRequest */
