#include "taskFlyport.h"
#include "upload.h"
#include "mrf24fj40ma.h"
#include "profile.h"
#include "pir.h"
#include "apds9300.h"
#include "ds1820.h"
#include "clock.h"
#include "uart.h"


#ifdef uart

int msglen;
int i,k;
char inmsg[500];
char *fetch_temp,*fetch_lux,*fetch_pir;
char luxstr[10],temps[10],pirs[10];

void UART2_init()
	{
		IOInit(p6,UART2TX);  // output D2_out
		IOInit(p5,UART2RX);  // input  D1_in
		UARTInit(2,9600);
		UARTFlush(2);
		UARTOn(2);
	}

void FlyportTask()
{
	int temp_flag=0;
	int lux_flag=0;
	int pir_flag=0;
	vTaskDelay(100);
	ProfileInit();
	ETHCustomLoad();
	ETHRestart(ETH_CUSTOM);
	//	Flyport waiting for the cable connection
	
	while (!MACLinked);

	UARTWrite(1,"Flyport ethernet connected to the cable... hello world!\r\n");
		UART2_init();
		UARTWrite(1,"UART INIT 2 Done\r\n");
		vTaskDelay(100);
		if(profile.AppEnable)
			UpTask();
	while(1)
	{
		//UARTWrite(1,"main loop");
		vTaskDelay(40);
		//UARTFlush(2);
		msglen = UARTBufferSize(2);
		if(msglen > 0)
		{
			taskENTER_CRITICAL();	
			memset(temps,'\0',sizeof(temps));
			memset(luxstr,'\0',sizeof(luxstr));
			memset(pirs,'\0',sizeof(pirs));
			
			
			
			vTaskDelay(20);
			msglen = UARTBufferSize(2);
			UARTRead(2, inmsg, msglen);
			inmsg[msglen+1] = '\0';
			UARTFlush(2);
			//UARTWrite(1,inmsg);
			fetch_temp = strstr (inmsg,":");
			fetch_lux  = strstr (inmsg,"Lux_calc:");
			fetch_pir = strstr (inmsg,"Pir:");
			lux_flag=1;
			temp_flag=1;
			pir_flag=1;
			for(i=0;i<=8;++i)
			{
				
				if(fetch_temp[i+1]==',')
					temp_flag=0;
				if(fetch_lux[i+9]=='|')
					lux_flag=0;
				if(fetch_pir[i+4]=='\n')
					pir_flag=0;
				if(temp_flag)
				{
					temps[i]=fetch_temp[i+1];
				}
				if(lux_flag)
				{
					luxstr[i] =fetch_lux[i+9];
				}
				if(pir_flag)
				{
					pirs[i] =fetch_pir[i+4];
					//k=i;
				}
			}		
			//pirstr[k+1]='\0';
			//UARTWrite(1,pirstr);
			UARTWrite(1,"\ntmp:");		
			UARTWrite(1,temps);		
			UARTWrite(1,"\nlux:");
			UARTWrite(1,luxstr);		
			UARTWrite(1,"\nPir:");
			UARTWrite(1,pirs);		
			UARTWrite(1,"\r\n");
			memset(inmsg,'\0',sizeof(inmsg));
			taskEXIT_CRITICAL();
			if(profile.AppEnable)
				AppTask(pirs,temps,luxstr);
		}	
	}
}


char* getlight()
{
	return luxstr;
}

char* gettemp()
{
	return temps;
}

char* getpir()
{
	return pirs;	
}

#else
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
#endif