#include "taskFlyport.h"
#include "upload.h"
#include "mrf24fj40ma.h"
#include "profile.h"
#include "pir.h"
#include "apds9300.h"
#include "ds1820.h"
#include "clock.h"

int Random_Delay()//random delay function 
{
	int i=rand();
	char buf[20];
	sprintf(buf,"\n %d : %d",i,i/1000);
	UARTWrite(1,buf);
	i=i/1000;
	DelayMs(i);
}

void FlyportTask()
{
	vTaskDelay(100);
	ProfileInit();

	ETHCustomLoad();
	UARTWrite(1,"Connecting to custom...\r\n");
	ETHRestart(ETH_CUSTOM);
	//	Flyport waiting for the cable connection
	
	while (!MACLinked);
	vTaskDelay(100);
	UARTWrite(1,"Flyport ethernet connected to the cable... hello world!\r\n");
	
	PIRInit();//Intialize PIR motion sensor
	DS1820Init();//Intialize Temperature sensor
	ClockInit();//Intialize RTCC
	APDSInit();//Intialize Light sensor
	Random_Delay();//Put random delay while initialization
	
	UARTWrite(1,"\r\nEntering in infinite loop...\r\n");
	
	if(profile.AppEnable)
	{
	AppTask();
	}
	while(1);
}
