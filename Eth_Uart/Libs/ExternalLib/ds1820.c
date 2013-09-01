#include "HWlib.h"
#include "taskFlyport.h"
#include "ds1820.h"

//This code is taken from existing OpenPicus project at http://community.openpicus.com/node/1212

//////////////////////////////////////////////////////////////////////////////
// OW_RESET - performs a reset on the one-wire bus and
// returns the presence detect. 
//
unsigned char ow_reset(void)
{
	unsigned char presence;
	
	IOInit(DQ_PORT,out);
	IOPut(DQ_PORT,OFF);		//pull DQ line low
	
	Delay10us(48);			// leave it low for 480us
	IOInit(DQ_PORT,inup);	// allow line to return high
	
	Delay10us(7);			// wait for presence
	presence = IOGet(DQ_PORT);	// get presence signal
	
	Delay10us(42);			// wait for end of timeslot
	return(presence); // presence signal returned
} // 0=presence, 1 = no part


//////////////////////////////////////////////////////////////////////////////
// READ_BIT - reads a bit from the one-wire bus. 
//
unsigned char read_bit(void)
{
	unsigned char i;

	IOInit(DQ_PORT,out);
	IOPut(DQ_PORT,OFF);		// pull DQ low to start timeslot

	IOInit(DQ_PORT,inup);	// then return high
	for (i=0; i<3; i++); 	// delay 15us from start of timeslot
	return(IOGet(DQ_PORT)); // return value of DQ line
}


//////////////////////////////////////////////////////////////////////////////
// WRITE_BIT - writes a bit to the one-wire bus, passed in bitval.
//
void write_bit(char bitval)
{

	IOInit(DQ_PORT,out);
	IOPut(DQ_PORT,OFF);		// pull DQ low to start timeslot
	if(bitval==1) 
		IOPut(DQ_PORT,ON);	// return DQ high if write 1
	Delay10us(10);			// hold value for remainder of timeslot

	IOPut(DQ_PORT,ON);		// pull DQ high
}


//////////////////////////////////////////////////////////////////////////////
// READ_BYTE - reads a byte from the one-wire bus.
//
unsigned char read_byte(void)
{
	unsigned char i;
	unsigned char value = 0;
	for (i=0;i<8;i++)
	{
		if(read_bit()) 
			value|=0x01<<i; 	// reads byte in, one byte at a time and then
								// shifts it left

		Delay10us(12);			// wait for rest of timeslot
	}
	return(value);
}



//////////////////////////////////////////////////////////////////////////////
// WRITE_BYTE - writes a byte to the one-wire bus.
//
void write_byte(char val)
{
	unsigned char i;
	unsigned char temp;
	for (i=0; i<8; i++) 	// writes byte, one bit at a time
	{
		temp = val>>i; 		// shifts val right 'i' spaces
		temp &= 0x01; 		// copy that bit to temp
		write_bit(temp); 	// write bit in temp into
	}
	Delay10us(10);
}

void DS1820Init()
{
	DS1820Read();
}

	double DS1820Read(void)
{
	unsigned char get[10], buf[64];
	char temp_lsb,temp_msb,temp_dec;	int k,p;
	double temp_c;
	
	void con_bin_2_temp(void){
	//DS18B20 temp calc from reg data
	temp_dec=(temp_lsb&0x0F);
	temp_lsb=temp_lsb>>4;
	temp_lsb=(temp_lsb&0x0F);
	temp_msb=(temp_msb&0x07);
	temp_msb=temp_msb<<4;
	temp_c=(double)temp_msb+(double)temp_lsb+((double)temp_dec/16);
	}	
	
	taskENTER_CRITICAL();
	#if defined(DS18B20_9bit_resolution)
	ow_reset();
	write_byte(0xCC); //Skip ROM
	write_byte(0x4E); //Write Scratch Pad
	write_byte(0x00); // Data write for Byte2 of Scratch pad i.e T_H reg
	write_byte(0x00); // Data write for Byte3 of Scratch pad i.e T_L reg
	write_byte(0x1F); // Data write for Byte4 of Scratch pad i.e config reg---1F=9bit/3F=10/5F=11/7F=12bit resolution
	Delay10us(1);
	#endif
	#if defined(DS18B20_higher_resolution)
	ow_reset();
	write_byte(0xCC); //Skip ROM
	write_byte(0x4E); //Write Scratch Pad
	write_byte(0x00); // Data write for Byte2 of Scratch pad i.e T_H reg
	write_byte(0x00); // Data write for Byte3 of Scratch pad i.e T_L reg
	write_byte(0x7F); // Data write for Byte4 of Scratch pad i.e config reg---1F=9bit/3F=10/5F=11/7F=12bit resolution
	Delay10us(1);
	#endif
	
	p=ow_reset();
	write_byte(0xCC); //Skip ROM
	write_byte(0x44); // Start Conversion
	//delay(5);
	Delay10us(75);//change to 75 from 50 as per datasheet
	ow_reset();
	write_byte(0xCC); // Skip ROM
	write_byte(0xBE); // Read Scratch Pad
	for (k=0;k<9;k++){get[k]=read_byte();}//store scratch pad data in to char array
	taskEXIT_CRITICAL();

	
	temp_msb = get[1]; // Sign byte + lsbit
	temp_lsb = get[0]; // Temp data plus lsb
	
	#if defined(DS1820_9bit)
	if (temp_msb)
	{
		temp_c = (temp_lsb)*0.5;
	}
	else
		temp_c = temp_lsb * 0.5;		// temperature is positive, resolution 0.5°C/digit
	#endif
	
	#if defined(DS1820_higher_resolution)		
	temp_lsb=temp_lsb>>1;
	temp_c=(double)temp_lsb-0.25+(((double)get[7]-(double)get[6])/(double)get[7]);
	#endif	
	
	#if defined(DS18B20_9bit_resolution)
	con_bin_2_temp();
	#endif
	
	#if defined(DS18B20_higher_resolution)
	con_bin_2_temp();
	#endif
	
	
	//sprintf(buf,"\n %dScratchPAD DATA = %X %X %X %X %X %X %X %X %X \n",p, get[8],get[7],get[6],get[5],get[4],get[3],get[2],get[1],get[0]);
    //UARTWrite(1,buf);
	return(temp_c);
}