#include "apds9300.h"
#include "HWlib.h"
#include <math.h>

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


void APDSInit()
{
	I2CInit(LOW_SPEED);
	I2CStart();
	I2CWrite(0b01110010); //address of the light sensor (dependent on an external pin that in our hardware is left to float)
	I2CWrite(0b10100000); //cmd register
	I2CWrite(0b00000011); //powerup
	I2CStop();
	DelayMs(100);
}

float APDSRead()
{
	int t1_l, t1_h, t2_l, t2_h;
	float Lux;//lux values returned after computation of register values 
	unsigned int CH0,CH1;//16-Bit values are updated in to these from high low byte of registers
	I2CStart();
	I2CWrite(0b01110010); //address
	I2CWrite(0b10101100); //write Ch for CH0 low byte
	I2CStop();
	DelayMs(1);
	I2CStart();
	I2CWrite(0b01110011); //address
	t1_l = I2CRead(1);
	I2CStop();
	DelayMs(1);
	I2CStart();
	I2CWrite(0b01110010); //address
	I2CWrite(0b10101101); //write Dh for CH0 high bye
	I2CStop();
	DelayMs(1);
	I2CStart();
	I2CWrite(0b01110011); //address
	t1_h = I2CRead(1);
	I2CStop();
	DelayMs(1);
	I2CStart();
	I2CWrite(0b01110010); //address
	I2CWrite(0b10101110); //write Eh for CH1 low byte
	I2CStop();
	DelayMs(1);
	I2CStart();
	I2CWrite(0b01110011); //address
	t2_l = I2CRead(1);
	I2CStop();
	DelayMs(1);
	I2CStart();
	I2CWrite(0b01110010); //address
	I2CWrite(0b10101111); //write Fh for CH1 high bye
	I2CStop();
	DelayMs(1);
	I2CStart();
	I2CWrite(0b01110011); //address
	t2_h = I2CRead(1);
	I2CStop();
	CH0=(t1_h<<8)|t1_l;//placing 8bit values to 16-bit CH0 Variable for calculation
	CH1=(t2_h<<8)|t2_l;//placing 8bit values to 16-bit CH1 Variable for calculation
	Lux=Lux_calc(CH1,CH0);//Function for LUX(Light intensity cal as per in APDS9300 datasheet)
	return Lux;
}
