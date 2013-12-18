//#include "onewire.h"
#include "ds18x20.h"
#include <stdlib.h>
#include <avr/io.h>
#include <string.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void DS1820Init()
{
//	temp=DS1820Read();
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
	
	#if defined(DS18B20_9bit_resolution)
	ow_reset();
	ow_byte_wr(0xCC); //Skip ROM
	ow_byte_wr(0x4E); //Write Scratch Pad
	ow_byte_wr(0x00); // Data write for Byte2 of Scratch pad i.e T_H reg
	ow_byte_wr(0x00); // Data write for Byte3 of Scratch pad i.e T_L reg
	ow_byte_wr(0x1F); // Data write for Byte4 of Scratch pad i.e config reg---1F=9bit/3F=10/5F=11/7F=12bit resolution
	_delay_us(10);
	#endif
	#if defined(DS18B20_higher_resolution)
	ow_reset();
	ow_byte_wr(0xCC); //Skip ROM
	ow_byte_wr(0x4E); //Write Scratch Pad
	ow_byte_wr(0x00); // Data write for Byte2 of Scratch pad i.e T_H reg
	ow_byte_wr(0x00); // Data write for Byte3 of Scratch pad i.e T_L reg
	ow_byte_wr(0x7F); // Data write for Byte4 of Scratch pad i.e config reg---1F=9bit/3F=10/5F=11/7F=12bit resolution
	_delay_us(10);
	#endif
	
	p=ow_reset();
	ow_byte_wr(0xCC); //Skip ROM
	ow_byte_wr(0x44); // Start Conversion
	//delay(5);
	_delay_us(750);//change to 75 from 50 as per datasheet
	ow_reset();
	ow_byte_wr(0xCC); // Skip ROM
	ow_byte_wr(0xBE); // Read Scratch Pad
	for (k=0;k<9;k++){get[k]=ow_byte_rd();}//store scratch pad data in to char array

	
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
    //uart1_puts(buf);
	return(temp_c);
}