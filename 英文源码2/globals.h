
//Global variables-------------------------------
//
//
//Can be used in all header and C files

#define BUF_LENGTH 300		//Number of characters in a frame
#define EnableInterrupts _EINT()

extern char rxdata;			//RS232 RX data byte
extern unsigned char buf[BUF_LENGTH];
extern signed char RXTXstate;	//used for transmit recieve byte count
extern unsigned char flags;	//stores the mask value (used in anticollision)
extern unsigned char RXErrorFlag;
extern unsigned char RXflag;		//indicates that data is in buffer

extern unsigned char i_reg;	//interrupt register

extern unsigned char CollPoss;


extern unsigned char RXdone;
extern unsigned char ENABLE;
extern unsigned char POLLING;


//-----------------------------------------------
