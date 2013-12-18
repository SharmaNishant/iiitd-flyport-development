#include <stdlib.h>
#include <avr/io.h>
#include <string.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "uart.h"
#include "ds18x20.h"
#include "onewire.h"
#include "apds9300.h"


/* 9600 baud */
#define UART_BAUD_RATE      9600
#define xtalCpu 8000000L//7372800L

double temp,lux;
int pir;
char buf1[20],buf2[20],buf3[20],buf[80];

int main(){

	uart_init(UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU));
	sei();
	//DDRB=0x04;
	//PORTB=0x00;
	DDRD|=(0<<PD3);
	PORTD|=(0<<PD3);
	i2c_init(); // init i2c lib

	while(1)
	{
		memset(buf,'\0',sizeof(buf));
		_delay_ms(900);
		//PORTB=PORTB^(1<<2);
		//uart1_putc('o');
		//uart1_puts("embedded");
		//DS1820Init();
		
		lux=light();
		temp=DS1820Read();
		dtostrf(temp,8,6,buf1);
		dtostrf(lux,8,6,buf2);
		if (PIND & (1<<PD3))
			{pir=1;}
		else 
			{pir=0;}
	    //sprintf(buf3,"%d",pir);
		//buf3=
		itoa(pir,buf3,10);
		_delay_ms(50);
		//uart_puts(buf1);
		//uart_puts("\n");
		//uart_puts(buf2);
		//uart_puts("\n");
		//uart_puts(buf3);
		/*
		uart_puts("\r\nTemp val:");
		uart_puts(buf1);
		uart_puts(",Lux_calc:");
		uart_puts(buf2);
        uart_puts("|Pir:");
	    uart_puts(buf3);		
		uart_puts("\n");
		*/
		//sprintf(buf,"Temp_val:%s,Lux_calc:%s|Pir:%s\r\n",buf1,buf2,buf3);
		//strcpy(buf,"Temp_val:s,Lux_calc:s|Pir:s\r\n");
		strcat(buf,"Temp_val:");
		strcat(buf,buf1);
		strcat(buf,",Lux_calc:");
		strcat(buf,buf2);
		strcat(buf,"|Pir:");
		strcat(buf,buf3);
		strcat(buf,"\n\r\n");
		_delay_ms(50);
		uart_puts(buf);
		//uart_puts("\n");
		
		
		
	}
	return 0;
}
