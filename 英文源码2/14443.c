#include "14443.h"

/*
 =======================================================================================================================
 =======================================================================================================================
 */

char SelectCommand(unsigned char select, unsigned char *UID)
{
	/*~~~~~~~~~~~~~~~~~~~~*/
	unsigned char	j;
	char			ret = 0;
	/*~~~~~~~~~~~~~~~~~~~~*/

	buf[50] = ISOControl;	/* enable RX CRC calculation */
	buf[51] = 0x08;
	WriteSingle(&buf[50], 2);

	for(j = 0; j < 5; j++) buf[j + 7] = *(UID + j);
	buf[0] = 0x8f;
	buf[1] = 0x91;			/* buffer setup for FIFO writing */
	buf[2] = 0x3d;
	buf[3] = 0x00;
	buf[4] = 0x70;
	buf[5] = select;
	buf[6] = 0x70;

	RAWwrite(buf, 12);		/* send the request using RAW writing */

	/* Write 12 bytes the first time you write to FIFO */
	i_reg = 0x01;
	RXTXstate = 1;			/* the response will be stored in buf[1] upwards */

	/*
	 * LPM0;
	 * //wait for end of transmit
	 */
	while(i_reg == 0x01)
	{
	}

	i_reg = 0x01;

	CounterSet();
	countValue = 0x2000;	/* 10ms for TIMEOUT */
	startCounter;			/* start timer up mode */

	while(i_reg == 0x01)
	{
	}						/* wait for RX complete */

	if(!POLLING)
	{
		if(i_reg == 0xFF)
		{					/* recieved response */
			if((buf[1] & BIT2) == BIT2)
			{				/* UID not complete */
				kputchar('(');
				for(j = 1; j < RXTXstate; j++)
				{
					Put_byte(buf[j]);
				}			/* for */

				kputchar(')');
				ret = 1;
				goto FINISH;
			}
			else
			{				/* UID complete */
				kputchar('[');
				for(j = 1; j < RXTXstate; j++)
				{
					Put_byte(buf[j]);
				}			/* for */

				kputchar(']');
				ret = 0;
				goto FINISH;
			}
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
	}						/* end if(!POLLING) */

FINISH:
	return(ret);
}	/* SelectCommand */

unsigned char	completeUID[14];

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void AnticollisionLoopA(unsigned char select, unsigned char NVB, unsigned char *UID)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	unsigned char	i, lenght, newUID[4], more = 0;
	unsigned char	NvBytes = 0, NvBits = 0, Xbits, found = 0;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	buf[50] = ISOControl;			/* disable RX CRC calculation */
	buf[51] = 0x88;
	WriteSingle(&buf[50], 2);

	RXErrorFlag = 0;
	CollPoss = 0;

	lenght = 5 + (NVB >> 4);
	if((NVB & 0x0f) != 0x00)
	{
		lenght++;
		NvBytes = (NVB >> 4) - 2;	/* the number of known valid bytes */
		Xbits = NVB & 0x07;			/* the number of known valid bits */

		/* Both are used in the UID calculation */
		for(i = 0; i < Xbits; i++)
		{
			NvBits = NvBits << 1;
			NvBits = NvBits + 1;
		}
	}						/* if */

	buf[0] = 0x8f;			/* prepare the SELECT command */
	if(NVB == 0x70)			/* select command, otherwise anticollision command */
		buf[1] = 0x91;		/* transmit with CRC */
	else
		buf[1] = 0x90;
	buf[2] = 0x3d;
	buf[3] = 0x00;
	buf[4] = NVB & 0xf0;	/* number of complete bytes */
	if((NVB & 0x07) != 0x00) buf[4] |= ((NVB & 0x07) << 1) + 1; /* number of broken bits */
	buf[5] = select;			/* can be 0x93, 0x95 or 0x97 */
	buf[6] = NVB;				/* number of valid bits */
	buf[7] = *UID;
	buf[8] = *(UID + 1);
	buf[9] = *(UID + 2);
	buf[10] = *(UID + 3);

	RAWwrite(&buf[0], lenght);

	RXTXstate = 1;				/* the response will be stored in buf[1] upwards */

	i_reg = 0x01;
	while(i_reg != 0x00)
	{
		CounterSet();
		countValue = 0x2710;	/* 10ms for TIMEOUT */
		startCounter;			/* start timer up mode */
		LPM0;
	}	/* wait for end of TX */

	i_reg = 0x01;
	i = 0;
	while((i_reg == 0x01) && (i < 2))
	{	/* wait for end of RX or timeout */
		i++;
		CounterSet();
		countValue = 0x2710;						/* 10ms for TIMEOUT */
		startCounter;								/* start timer up mode */
		LPM0;
	}

	if(RXErrorFlag == 0x02) i_reg = 0x02;

	if(i_reg == 0xff)
	{
		if(!POLLING)
		{
			kputchar('(');
			for(i = 1; i < 6; i++) Put_byte(buf[i]);
			kputchar(')');
		}

		switch(select)
		{
		case 0x93:									/* cascade level 1 */
			if((buf[1] == 0x88) || (*UID == 0x88))
			{										/* UID not complete */
				if(NvBytes > 0)
				{
					for(i = 0; i < 4; i++)
					{
						/* Combine the known bytes and the recieved bytes to a whole UID. */
						if(i < (NvBytes - 1)) completeUID[i] = *(UID + i + 1);

						/* Combine the broken bits to a whole byte. */
						else if(i == (NvBytes - 1))
							completeUID[i] = (buf[i + 2 - NvBytes] &~NvBits) | (*(UID + i + 1) & NvBits);

						/* Add the recieved whole bytes. */
						else
							completeUID[i] = buf[i + 2 - NvBytes];
					}								/* for */
				}
				else
				{
					completeUID[0] = (buf[2] &~NvBits) | (*UID & NvBits);
					for(i = 0; i < 4; i++)
					{
						completeUID[i + 1] = buf[i + 3];
					}								/* for */
				}									/* if-else */

				buf[1] = 0x88;
				for(i = 0; i < 4; i++) buf[i + 2] = completeUID[i];

				SelectCommand(select, &buf[1]);
				NVB = 0x20;
				more = 1;
			}
			else
			{										/* UID
													 * complete;
													 * send UID to host */
				if(POLLING)
				{
					found = 1;
				}
				else
				{
					kputchar('[');
					if(NvBytes > 0)
					{
						for(i = 0; i < 4; i++)
						{
							if(i < (NvBytes - 1))	/* Combine the known bytes and the */
								Put_byte(*(UID + i + 1));	/* recieved bytes to a whole UID. */
							else if(i = (NvBytes - 1))
								Put_byte((buf[i + 2 - NvBytes] &~NvBits) | (*(UID + i + 1) & NvBits));
							else
								Put_byte(buf[i + 2 - NvBytes]);
						}						/* for */
					}
					else
					{
						Put_byte((buf[1] &~NvBits) | (*UID & NvBits));
						for(i = 0; i < 4; i++)
						{
							Put_byte(buf[i + 2]);
						}						/* for */
					}							/* if-else */

					kputchar(']');
				}
			}

			select = 0x95;
			break;

		case 0x95:								/* cascade level 2 */
			if(buf[1] == 0x88)
			{
				for(i = 0; i < 4; i++)
				{
					completeUID[i + 4] = buf[i + 2];
				}

				SelectCommand(select, &buf[1]);
				more = 1;
			}
			else
			{									/* UID
												 * complete;
												 * send UID to host */
				for(i = 0; i < 5; i++)
				{
					completeUID[i + 4] = buf[i + 1];
				}

				if(POLLING)
				{
					found = 1;
				}
				else
				{
					kputchar('[');
					for(i = 0; i < 3; i++)		/* send UID level 1 */
						Put_byte(completeUID[i]);
					Put_byte(completeUID[3]);	/* send BCC for UID 1 */

					for(i = 4; i < 8; i++)		/* send UID level 2 */
						Put_byte(completeUID[i]);
					Put_byte(completeUID[8]);	/* send BCC for UID 1 */
					kputchar(']');
				}
			}

			select = 0x97;
			break;

		case 0x97:						/* cascade level 3 */
			for(i = 0; i < 5; i++)
			{							/* UID is
										 * complete;
										 * send UID to host */
				completeUID[i + 8] = buf[i + 1];
			}

			if(POLLING)
			{
				found = 1;
			}
			else
			{
				kputchar('[');
				for(i = 0; i < 3; i++)	/* send UID level 1 */
					Put_byte(completeUID[i]);
				Put_byte(completeUID[3]);	/* send BCC for UID 1 */

				for(i = 4; i < 7; i++)		/* send UID level 2 */
					Put_byte(completeUID[i]);
				Put_byte(completeUID[7]);	/* send BCC for UID 2 */

				for(i = 8; i < 12; i++)		/* send UID level 3 */
					Put_byte(completeUID[i]);
				Put_byte(completeUID[12]);	/* send BCC for UID 3 */
				kputchar(']');
			}
			break;
		}					/* sswitch */
	}
	else if(i_reg == 0x02)
	{						/* collision occured */
		if(!POLLING)
		{
			kputchar('(');
			kputchar('z');
			kputchar(')');
		}
	}
	else if(i_reg == 0x00)
	{						/* timer interrupt */
		if(!POLLING)
		{
			kputchar('(');
			kputchar(')');
		}
	}
	else
		;

	if(i_reg == 0x02)
	{						/* go into anticollision */
		CollPoss++;			/* reader returns CollPoss - 1 */
		for(i = 1; i < 5; i++) newUID[i - 1] = buf[i];

		CounterSet();
		countValue = 100;	/* 1.2ms for TIMEOUT */
		startCounter;		/* start timer up mode */
		i_reg = 0x01;
		while(i_reg == 0x01)
		{
		}					/* wait for end of RX or timeout */

		AnticollisionLoopA(select, CollPoss, newUID);	/* recursive call for anticollision procedure */
	}	/* if */

	if(more)
	{	/* perform anticollison command for 7 or 10 - byte UID - recursive call for cascade levels */
		AnticollisionLoopA(select, NVB, UID);	/* only the select field is different, everything else is the same */
		if(POLLING) found = 1;
	}	/* if */

	if(found)
	{
		LEDtypeAON;
	}
	else
	{
		LEDtypeAOFF;
	}
}		/* AnticollisionLoopA */

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void AnticollisionSequenceA(unsigned char REQA)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	unsigned char	i, select = 0x93, NVB = 0x20;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


          //added code
	buf[0] = ModulatorControl;
	buf[1] = 0x21;
	WriteSingle(buf, 2);
       //end added code


	buf[0] = ISOControl;
	buf[1] = 0x88;			/* recieve with no CRC */
	WriteSingle(buf, 2);

	/*
	 * delay_ms(5);
	 */
	if(REQA) buf[5] = 0x26; /* send REQA command */
	else
		buf[5] = 0x52;		/* send WUPA command */
	RequestCommand(&buf[0], 0x00, 0x0f, 1);
	irqCLR;					/* PORT2 interrupt flag clear */
	irqON;

	/*
	 * UIDsize = ((buf[2] >> 6) & 0x03) + 1;
	 */
	if(i_reg == 0xff || i_reg == 0x02)
	{
		for(i = 40; i < 45; i++) buf[i] = 0x00;
		AnticollisionLoopA(select, NVB, &buf[40]);
		if(POLLING) LEDtypeAON;
	}
	else
	{
		LEDtypeAOFF;
	}

	buf[0] = ISOControl;
	buf[1] = 0x08;			/* recieve with no CRC */
	WriteSingle(buf, 2);
	irqOFF;
}	/* AnticollisionSequenceA */

/*
 =======================================================================================================================
 =======================================================================================================================
 */

unsigned char Request14443A(unsigned char *pbuf, unsigned char lenght, unsigned char BitRate)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	unsigned char	index, j, command, RXBitRate, TXBitRate, reg[2];
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	TXBitRate = ((BitRate >> 4) & 0x0F) + 0x08;
	RXBitRate = (BitRate & 0x0F) + 0x08;

	reg[0] = ISOControl;
	reg[1] = TXBitRate;
	WriteSingle(reg, 2);

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

	reg[0] = ISOControl;
	reg[1] = RXBitRate;
	WriteSingle(reg, 2);

	command = 0x16;
	DirectCommand(&command);
	command = 0x17;
	DirectCommand(&command);

	i_reg = 0x01;

	CounterSet();
	countValue = 0xF000;	/* 60ms for TIMEOUT */
	startCounter;			/* start timer up mode */

	while(i_reg == 0x01)
	{
	}						/* wait for RX complete */

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
    Function sends the SlotMarker command for // ;
    the 14443B protocol. This command includes // ;
    the slot number. // ;
 =======================================================================================================================
 */
void SlotMarkerCommand(unsigned char number)
{
	/*buf[0] = 0x8f;
	buf[1] = 0x91;
	buf[2] = 0x3d;
	buf[3] = 0x00;
	buf[4] = 0x10;
	buf[5] = (number << 4) | 0x05;

	i_reg = 0x01;

	RAWwrite(&buf[0], 6);
	irqCLR;			///* PORT2 interrupt flag clear
	irqON;
	CounterSet();	///* TimerA set
	countValue = 0x4E20;	///* 20ms

	startCounter;
	while(i_reg == 0x01)
	{
	}*/

  //This method shows the carrier modulation

        buf[0] = 0x8f;
	buf[1] = 0x91;
	buf[2] = 0x3d;
	buf[3] = 0x00;
	buf[4] = 0x10;
	RAWwrite(&buf[0], 5);

	buf[5] = 0x3F;
	buf[6] = (number << 4) | 0x05;
	buf[7] = 0x00;
	
	i_reg = 0x01;
	
	RAWwrite(&buf[5], 3);



        irqCLR;			//PORT2 interrupt flag clear
        irqON;
	//CounterSet();				//TimerA set
	//countValue = 0x4E20;			//20ms
	//startCounter;	

        //LPM0;

        while(i_reg == 0x01)
        {
                CounterSet();			/* TimerA set */
		countValue = 0x9c40;	/* 20ms */
		startCounter;
		LPM0;


        }

}	

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void AnticollisionSequenceB(unsigned char command, unsigned char slots)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	unsigned char	i, collision = 0x00, j, found = 0,command3[2];
	unsigned int	k = 0;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


         //added code
       /* buf[0] = RXNoResponseWaitTime;
	buf[1] = 0x14;
	buf[2] = ModulatorControl;
	buf[3] = 0x20;
	WriteSingle(buf, 4);*/
       //end added code


	buf[0] = ModulatorControl;
	buf[1] = 0x20;
	WriteSingle(buf, 2);


	RXErrorFlag = 0x00;

	buf[0] = 0x8f;
	buf[1] = 0x91;
	buf[2] = 0x3d;
	buf[3] = 0x00;
	buf[4] = 0x30;
	buf[5] = 0x05;
	buf[6] = 0x00;
        //buf[6] = 0x20; //AFI Value

	if(slots == 0x04)
	{
		EnableSlotCounter();
		buf[7] |= 0x08;
	}


	buf[7] = slots;

	if(command == 0xB1) buf[7] |= 0x08; /* WUPB command else REQB command */

	i_reg = 0x01;



          RAWwrite(&buf[0], 8);

	irqCLR;						/* PORT2 interrupt flag clear */
	irqON;

	j = 0;
	while((i_reg == 0x01) && (j < 2))
	{
		j++;
		CounterSet();			/* TimerA set */
		countValue = 0x4E20;	/* 20ms */
		startCounter;
		LPM0;
	}						/* wait for end of TX */

	i_reg = 0x01;


	CounterSet();			/* TimerA set */
	countValue = 0x4E20;	/* 20ms */
        //countValue = 0x9c40;	/* 20ms at 13.56 MHz Clock*/
	startCounter;

	for(i = 1; i < 17; i++)
        {
		RXTXstate = 1;		/* the response will be stored in buf[1] upwards */



		while(i_reg == 0x01)
		{					///* wait for RX complete
			k++;
		      if(k == 0xFFF0)

                       {
				i_reg = 0x00;
				RXErrorFlag = 0x00;
				break;
			}
		}

		if(RXErrorFlag == 0x02) i_reg = RXErrorFlag;

		if(i_reg == 0xFF)
		{					/* recieved SID in buffer */
			if(POLLING)
			{
				found = 1;
			}
			else
			{
				kputchar('[');
				for(j = 1; j < RXTXstate; j++) Put_byte(buf[j]);
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

			collision = 0x01;
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

		if((slots == 0x00) || (slots == 0x01) || (slots == 0x02) || ((slots == 0x04) && (i == 16))) break;

                SlotMarkerCommand(i);



                //added code
              //  Put_byte(i);
                //end code
		i_reg = 0x01;

		if(!POLLING)
		{
			put_crlf();
		}
	}						/* for */

	if(slots == 0x04) DisableSlotCounter();

	irqOFF;

	if(found)
	{
		LEDtypeBON;
	}
	else
	{
		LEDtypeBOFF;
	}

	if(collision != 0x00) AnticollisionSequenceB(0x20, 0x02);	/* Call this function for 16 timeslots */
}	/* AnticollisionSequenceB */


