#include "taskFlyport.h"
#include "rtcc.h"
#include "pir.h"
#include "upload.h"
#include "profile.h"
#include "time.h"
#include "clock.h"
char input[20];
char rtccString[50];
//extern BOOL alarmflag;
/*t_RTCC myrtcc;
t_RTCC myalarm;
t_RTCC nowrtcc;*/

int on_flag,off_flag;
int opt_trig; //trigger which is implemented as a virtual clock

//Interrupt function for motion triggering
void external_interrupt_function()
{
UARTWrite(1,"Motion Sensed -> Timer Reset!!\r\n");
opt_trig=0; // reseting the trigger so that clock will restart every time we sense any motion
if(WFStatus==TURNED_OFF)
	{
		on_flag=1; //setting ON_flag to 1 to wake up the module
	}
}



//void opt_handle_time()
void optimization()
{
	UARTWrite(1,"Optimization Mode Enabled");
	opt_trig=0;
	on_flag=0;
	off_flag=0;
	if(profile.OnType==MOTION) //intializing motion triggered interrupt
	{
		//IOInit(PIR_PORT,inup);
		IOInit(PIR_PORT, EXT_INT2);
		INTInit(2, external_interrupt_function, 0);
		INTEnable(2);	
	}
	/*// Set RTCC
	myrtcc.year = 11; 
	myrtcc.month = 11;
	myrtcc.dweek = 3; 
	myrtcc.day = 2;
	myrtcc.hour = 16;
	myrtcc.min = 23;
	myrtcc.sec = 1;
	// Write settings on internal registers
	RTCCWrite(&myrtcc);
	// Create alarm configuration
	myalarm = myrtcc;
	myalarm.sec = myalarm.sec + 20;
	// Set Alarm configuration to internal registers
	RTCCSetAlarm(&myalarm, REPEAT_INFINITE, EVERY_SEC);
	// Active alarm
	RTCCRunAlarm(1); // 1 turn on, 0 turn off*/

	alarmflag_opt = 0;
	while(1)
	{
		
		if(on_flag==1)
		{
			on_flag=0;
			opt_trig=0;
			sprintf(input,"\nLast Wifi Status : %d\n\n",WFStatus);
			UARTWrite(1,input);
			UARTWrite(1,"Wi-Fi transceiver activation...\r\n");
			WFOn();
			while (WFStatus == TURNED_OFF);
			UARTWrite(1,"Wi-Fi On\r\n");
			//to check if Custom profile exists then load it or otherwise load the default profile after turning on wif
				if(NETCustomExist())
				{
					WFConnect(WF_CUSTOM);
				}
				else
				{
					WFConnect(WF_DEFAULT);
				}
			while (WFStatus != CONNECTED);
			tcp_socket_handle();//to reset tcp socket
		}
		if(off_flag==1)
		{
			off_flag=0;
			opt_trig=0;
			if(profile.AppEnable)
				publish_handle();//to publish any data present before going to sleep
			sprintf(input,"\nLast Wifi Status : %d\n\n",WFStatus);
			UARTWrite(1,input);
			if(profile.OptLevel==LVL_WIFI)
			{
				UARTWrite(1,"Turning Wifi OFF.. \r\n");
				WFHibernate();
			}
			else if(profile.OptLevel==LVL_MICROCONTROLLER)
			{
				UARTWrite(1,"Turning Wifi OFF and Putting Flyport to SLEEP.. \r\n");
				WFSleep();
			}
		}
		
		// Check RTCC alarm flag 
		//Added alarmflag_opt in rttc.c file and clock.h
		if(alarmflag_opt == 1)
		{
			opt_trig++;
			alarmflag_opt = 0;
			vTaskDelay(1);
			//UARTWrite(1, "OPT Rtcc ALARM!!!\r\n\n\n");
			if(WFStatus==TURNED_OFF)
			{
				if(profile.OnType==TIME)
				{
					if(opt_trig>=profile.OffTime)
					{
						on_flag=1;
					}			
				}
			}
			else
			{
				if(opt_trig>=profile.OnTime)
				{
					off_flag=1;
				}
			}
		}
	}
}



/*
opt_handle_motion()
{
IOInit(PIR_PORT,inup);
IOInit(PIR_PORT, EXT_INT2);
INTInit(2, external_interrupt_function, 1);
INTEnable(2);
//to be implemented soon

}*/

