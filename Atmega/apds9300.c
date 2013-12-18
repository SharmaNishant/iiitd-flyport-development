

#include "i2cmaster.h"
#include "apds9300.h"



int temp_h0,temp_l0,temp_h1,temp_l1;
char buf[20];
double CH0,CH1;
double lux;


float Lux_calc(float CH1,float CH0)
{
		float k = CH1/CH0;
		float Lux=0;

		if((k>=0)&& (k<=0.52))
			Lux=(0.0315*CH0)-(0.0593*CH0*pow(k,1.4));
		else if((k>0.52)&& (k<=0.65))
			Lux=(0.0229*CH0)-(0.0291*CH1);
		else if((k>0.65)&& (k<=0.80))
			Lux=(0.0157*CH0)-(0.0180*CH1);
		else if((k>0.80)&& (k<=1.30))
			Lux=(0.00338*CH0)-(0.00260*CH1);
		else 
			Lux=0;
		return Lux;
}


double light(void)
{

	  i2c_start_wait(APDS9300+I2C_WRITE);      // set device address and write mode
      i2c_write(0xE0);
	  i2c_write(0x03);
	  i2c_stop();

	  i2c_start_wait(APDS9300+I2C_WRITE);
	  i2c_write(0xEC);
	  i2c_stop();

	  i2c_start_wait(APDS9300+I2C_READ);
	  temp_l0 = i2c_readAck();       
      temp_h0 = i2c_readAck();    
  //    uart_putc(temp_l0);
//	uart_putc(temp_h0);
      
 
	  i2c_start_wait(APDS9300+I2C_WRITE);
	  i2c_write(0xEE);
	  i2c_stop();

	  i2c_start_wait(APDS9300+I2C_READ);
	  temp_l1 = i2c_readAck();       
      temp_h1 = i2c_readAck();     
	 
//	 uart_putc(temp_l1);
//	uart_putc(temp_h1);
      i2c_stop();

     CH0=temp_h0*256+temp_l0;
	 CH1=temp_h1*256+temp_l1;
	 lux=Lux_calc(CH1,CH0) ;
	 
	 return lux;
	 //uart1_puts("\nLux_calc:");
	 //dtostrf(lux,8,6,buf);
	 //uart1_puts(buf);
}