#include "mrf24fj40ma.h"
#include "HWlib.h"
#include <p24FJ256GA106.h>

#define MRF24J40_RESET  P17 //p19 EARLIER
#define MRF24J40_WAKE   p5  //p21
#define MRF24J40_CS   	p14
#define MRF24J40_INT   	p4

#define SPI_MASTER 0x013E //0x0120 // select 8-bit master mode, CKE=1, CKP =0
#define SPI_ENABLE 0x8000 // enable SPI port, clear status
typedef union _MRF24J40_IFS
{
    BYTE Val;
    struct _MRF24J40_IFS_bits
    {
        BYTE RF_TXIF :1;
        BYTE :2;
        BYTE RF_RXIF :1;
        BYTE :4;
    }bits;
} MRF24J40_IFREG;

void mrf24j40ma_init()
{
	int i,t=0;
	
	__builtin_write_OSCCONL(OSCCON & 0xBF); // Unlock registers
	// Configure SPI2 Pinout
	RPOR1bits.RP2R = 11; // Assign RP2 to SCK2  (output)
	RPOR1bits.RP3R = 10; // Assign RP3 to SDO2  (output)
	RPINR22bits.SDI2R = 11; // Assign RP11 to SDI2 (input)
	RPOR11bits.RP23R = 12; // Assign RP23 to SS2 (output)
	__builtin_write_OSCCONL(OSCCON | 0x40); // Lock register
	
	IOInit(MRF24J40_CS, out);
	IOPut(MRF24J40_CS, on);
	
	IOInit(MRF24J40_RESET, out);
	IOPut(MRF24J40_RESET, on);
	
	IOInit(MRF24J40_INT,indown);//Earlier it was input pull down after \int\-stat issue
	//IOPut(MRF24J40_INT,off);
	
	SPI2CON1 = SPI_MASTER;
	SPI2CON2 = 0;
	SPI2STAT = SPI_ENABLE;
	
	/*
    //PHY_RESETn = 0;
	IOPut(MRF24J40_RESET, off);
	//reset the MRF24J40 by pulling the RESET pin13 low
    for(j=0;j<(WORD)300;j++){}
    //PHY_RESETn = 1;
    IOPut(MRF24J40_RESET, on);
	for(j=0;j<(WORD)300;j++){}
	*/
	/* do a soft reset */
	UARTWrite(1,"A\r\n");//UART test Debug
	
	lowWrite((WRITE_SOFTRST),0x07);		// reset everything (power, baseband, MAC) (also does wakeup if in sleep)
	UARTWrite(1,"B\r\n");//UART test Debug
	
	do
	{
		i = lowRead(READ_SOFTRST);
		t++;
	}
	while((i&0x07) != (UINT8)0x00);  	 	// wait for hardware to clear reset bits
	
	//while (lowRead(READ_SOFTRST) & 0x07);
	DelayMs(1);
	UARTWrite(1,"C\r\n");//UART test Debug
	
	lowWrite(WRITE_PACON2,0x98);
				//PACON2 = 0x98, Initialize FFOEN=1 and TXONTS = 0x6
	lowWrite(WRITE_TXSTBL,0x95);
				//Initialize RFSTBL = 0x9
	highWrite(RFCON1,0x01);
				//Initialize VCOOPT=1
	highWrite(RFCON2,0x80);
				//Enable PLL
	highWrite(RFCON6,0x90);
				//Initialize TXFIL=1, 20MRECVR=1
	highWrite(RFCON7, 0x80);
				//Initialize SLPCLKSEL = 0x2 (100KHz internal oscialltor)
	highWrite(RFCON8, 0x10);
				//Initialize RFVCO =1
	highWrite(SLPCON1, 0x21);
				//Initialize SLPCON0 to define INTEDGE as high 
				//means this pin will give rising state as Interrupt will come
	highWrite(SLPCON0, 0x03);//Earlier this was 0x02 but that time sleep clock was enabled bit0 of this register
				//Initialize CLKOUTEN=1 and SLPCLKDIV = 0x01
	lowWrite(WRITE_BBREG2,0x80);
				//Set CCA mode to ED
	lowWrite(WRITE_CCAEDTH,0x60);
				//Set CCA-ED Threshold
	lowWrite(WRITE_BBREG6,0x40);
				//Set appended RSSI value to RX FIFO
    lowWrite(WRITE_INTCON,0xF7);
		//INTCON (0x32) = 0xF6 - Enables only RXIF interrupts	
    SetChannel(CHANNEL_11);
		//set operating channel as channel 11
	/*for(j=0;j<(WORD)800;j++)
	{
		Nop();
	}
	*/	//Delay for 192 us after RF State Machine Reset
    do
    {
        i = highRead(RFSTATE);
    }
    while((i&0xA0) != 0xA0); 
	//while((highRead(RFSTATE)&0xA0) != 0xA0);
	//Wait until the RFSTATE machine indicates RX state
	
	UARTWrite(1,"D\r\n");//UART test Debug
	
	lowWrite(WRITE_ORDER, 0xff);
	lowWrite(WRITE_RXMCR, 0x29);
	// Automatic acknowledgement is not transmitted for received packets by TXMAC
	//ERRpkt mode bit is set to 0
	//Promiscuous packet mode bit is set to 1
	//PAN coordinator set
	UARTWrite(1,"\n Wireless Initiated");
	DelayMs(1);
		
}
/*
To set some custom value for device MAC address, PAN Id, Short Address. All these param have to be initiazed once 
at start time as POR values are NULL.  
*/
void Set_Device_MAC_PANID(BYTE E0,BYTE E1,BYTE E2,BYTE E3,BYTE E4,BYTE E5,BYTE E6,BYTE E7,BYTE PH,BYTE PL,BYTE SH,BYTE SL)
{
	lowWrite(WRITE_EADR0, E0);//Write Mac Addresss Byte 0
	lowWrite(WRITE_EADR1, E1);//Write Mac Addresss Byte 1
	lowWrite(WRITE_EADR2, E2);//Write Mac Addresss Byte 2
	lowWrite(WRITE_EADR3, E3);//Write Mac Addresss Byte 3
	lowWrite(WRITE_EADR4, E4);//Write Mac Addresss Byte 4
	lowWrite(WRITE_EADR5, E5);//Write Mac Addresss Byte 5
	lowWrite(WRITE_EADR6, E6);//Write Mac Addresss Byte 6
	lowWrite(WRITE_EADR7, E7);//Write Mac Addresss Byte 7
	
	lowWrite(WRITE_PANIDH,PH);//Write PANID High byte 
	lowWrite(WRITE_PANIDL,PL);//Write PANID Low byte 
	lowWrite(WRITE_SADRH ,SH);//Write Short Addr High byte
	lowWrite(WRITE_SADRL,SL);//Write Short Addr Low byte 
}
void GET_Device_MAC_PANID(void)
{	
	int x,y,x1,x2,x3,x4,x5,x6,x7,x8,y1,y2,a,b,c,d;char buf[100];
	x=lowRead(READ_PANIDL);
	y=lowRead(READ_PANIDH);
	
	x1=lowRead(READ_EADR0);
	x2=lowRead(READ_EADR1);
	x3=lowRead(READ_EADR2);
	x4=lowRead(READ_EADR3);
	x5=lowRead(READ_EADR4);
	x6=lowRead(READ_EADR5);
	x7=lowRead(READ_EADR6);
	x8=lowRead(READ_EADR7);
	
	y1=lowRead(READ_SADRH);
	y2=lowRead(READ_SADRL);
	
	a=lowRead(READ_RXMCR);
	b=lowRead(READ_INTCON);
	c=lowRead(READ_INTSTAT);
	d=lowRead(READ_ORDER);
	
	sprintf(buf,"\nPANIDL %X PANIDH %X EADR %X:%X:%X:%X:%X:%X:%X:%X SADRH %X SADRL %X RXMCR %X INTCON %X INTSTAT %X ORDER %X",x,y,x1,x2,x3,x4,x5,x6,x7,x8,y1,y2,a,b,c,d);
	UARTWrite(1,buf);
	
}

void check_interrupt(void)
{	int c;
	c=lowRead(READ_INTSTAT);
	
	if(c&(0x08))
	{   //UARTWrite(1,"\r\n Rx");
		volatile BYTE i,j,RSSI_VAL,k;
		volatile INT16 ReaderID=1,UTCTime=9;
		char StoreString[250];
		//packet received
		//Need to read it out
		i=highRead(0x300); //first byte of the receive buffer is the packet length (this 
		j=highRead(0x308);		//TAG ID
		k=highRead(0x309);		//Battery level indicator 
		RSSI_VAL = highRead(0x300+i+2);	//RSSI value
		lowWrite(WRITE_RXFLUSH, 0x01);	//end of RXIF check
		if(k == 'L')sprintf(StoreString,"R %X TAGID %X RSSI %X BATVAL %c UTC %lX",ReaderID,j,RSSI_VAL,k,UTCTime);//write ReaderID,TagId,RSSI value
		else sprintf(StoreString,"R %X TAGID %X RSSI %X UTC %lX",ReaderID,j,RSSI_VAL,UTCTime);//write ReaderID,TagId,RSSI value
		strcat(StoreString,"\r");
		strcat(StoreString,"\n");
		UARTWrite(1,StoreString);
		//Flush the receiver buffer//imp to recieve further content
		lowWrite(WRITE_RXFLUSH,0x01);	
	}
}
/*Periodically call this in the maindemo.c. This routine will see if
there is any rxed data on the SPI from the mrf24j40 transceiver.
If data is rxed a http post packet is formed and the httppostcsmapending
flag is set.
*/
void MRF24J40Process()
{
	//MRF24J40_IFREG flags;              							        
	char buf[5];//,StoreString[200];	
	int m;//ReaderID=128,UTCTime=5;
	//flags.Val
	m=lowRead(READ_INTSTAT);		//read the interrupt status register to see what caused the interrupt
	sprintf(buf,"\nINTSTAT is %d ",m);
	UARTWrite(1,buf);
	//if(m&0x08)
	//{UARTWrite(1,"rx data\r\n");}
	//else
	//{UARTWrite(1,"no data\r\n");}
	
	/*
	if(flags.bits.RF_RXIF)
	{   
		volatile BYTE i,j,RSSI_VAL,k;
		//packet received
		//Need to read it out
		i=highRead(0x300); //first byte of the receive buffer is the packet length (this 
		j=highRead(0x308);		//TAG ID
		k=highRead(0x309);		//Battery level indicator 
		RSSI_VAL = highRead(0x300+i+2);	//RSSI value
		lowWrite(WRITE_RXFLUSH, 0x01);	//end of RXIF check
		if(k == 'L')sprintf(StoreString,"R %X\t%X\t%X\t%c\t%lX",ReaderID,j,RSSI_VAL,k,UTCTime);//write ReaderID,TagId,RSSI value
		else sprintf(StoreString,"R %X\t%X\t%X\t%lX",ReaderID,j,RSSI_VAL,UTCTime);//write ReaderID,TagId,RSSI value
		strcat(StoreString,"\r");
		strcat(StoreString,"\n");
		UARTWrite(1,StoreString);
		//HttpPostPending = 1;
	}*/
}
void spiPut(unsigned char v)				// write 1 byte to SPI
{
	unsigned char i;
	while(SPI2STATbits.SPITBF); 	// wait for TX buffer to empty (should already be)
	SPI2BUF=v;						// write byte to TX buffer
	while(!SPI2STATbits.SPIRBF);	// wait for RX buffer to fill
	i=SPI2BUF;						// read RX buffer (don't know why we need to do this here, but we do)
}

unsigned char spiGet(void)							// read 1 byte from SPI
{
	while(SPI2STATbits.SPITBF); 			// wait for TX buffer to empty
	SPI2BUF=0x00;							// write to TX buffer (force CLK to run for TX transfer)
	while(!SPI2STATbits.SPIRBF);			// wait for RX buffer to fill
	return(SPI2BUF);						// read RX buffer
}

// reads byte from radio at long "address"
UINT8 highRead(UINT16 address)
{
	UINT8 toReturn;
//
//#ifndef SPI_INTERRUPTS
//	UINT8 tmpRFIE = RFIE;
//	RFIE = 0;
//	RADIO_CS = 0;
//#endif
	IOPut(MRF24J40_CS, off);
	spiPut(((address>>3)&0x7F)|0x80);
	spiPut(((address<<5)&0xE0));
	toReturn = spiGet();
/*#ifndef SPI_INTERRUPTS
	RADIO_CS = 1;
	RFIE = tmpRFIE;
#endif*/
	IOPut(MRF24J40_CS, on);
	return toReturn;
}

// writes "value" to radio at long "address"
void highWrite(UINT16 address, UINT8 value)
{
/*#ifndef SPI_INTERRUPTS
	UINT8 tmpRFIE = RFIE;
	RFIE = 0;										// disable radio ints during communication
	RADIO_CS = 0;									// select radio SPI bus
#endif*/
	IOPut(MRF24J40_CS, off);
	spiPut((((UINT8)(address>>3))&0x7F)|0x80);
	spiPut((((UINT8)(address<<5))&0xE0)|0x10);
	spiPut(value);
	IOPut(MRF24J40_CS, on);
/*#ifndef SPI_INTERRUPTS
	RADIO_CS = 1;									// de-select radio SPI bus
	RFIE = tmpRFIE;									// restore interrupt state
#endif*/
}

// reads byte from radio at short "address"
UINT8 lowRead(UINT8 address)
{
	UINT8 toReturn;
/*#ifndef SPI_INTERRUPTS
	UINT8 tmpRFIE = RFIE;
	RFIE = 0;										// disable radio ints during communication
	RADIO_CS = 0;									// select radio SPI bus
#endif*/
	IOPut(MRF24J40_CS, off);
	spiPut(address);
	toReturn = spiGet();
	IOPut(MRF24J40_CS, on);
/*#ifndef SPI_INTERRUPTS
	RADIO_CS = 1;									// de-select radio SPI bus
	RFIE = tmpRFIE;									// restore interrupt state
#endif*/
	return toReturn;
}

// writes "value" to radio at short "address"
void lowWrite(UINT8 address, UINT8 value)
{
/*#ifndef SPI_INTERRUPTS
	UINT8 tmpRFIE = RFIE;
	RFIE = 0;
	RADIO_CS = 0;
#endif*/
	IOPut(MRF24J40_CS, off);
	spiPut(address);
	spiPut(value);
	IOPut(MRF24J40_CS, on);
/*#ifndef SPI_INTERRUPTS
	RADIO_CS = 1;
	RFIE = tmpRFIE;
#endif*/
}

void SetChannel(unsigned int Channel) 
{
    WORD j;
	highWrite(RFCON0, (Channel|0x02));
    lowWrite(WRITE_RFCTL,0x04);
    for(j=0;j<(WORD)800;j++)
	{
		Nop();
	}
	//Delay for 192 us after RF State Machine Reset
	lowWrite(WRITE_RFCTL,0x00);
}
/*
void SetChannel(BYTE channel)
{
    WORD j;
    highWrite(RFCON0, (channel | 0x02));
				//RFCON0 = 0x02 for Channel number 11
    lowWrite(WRITE_RFCTL,0x04);
	for(j=0;j<(WORD)800;j++)
	{
		Nop();
	}
		//Delay for 192 us after RF State Machine Reset
    lowWrite(WRITE_RFCTL,0x00);
				//Reset RF State machine
				//no delay loop is present after performing RF State machine reset
}
*/