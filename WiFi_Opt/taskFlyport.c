#include "taskFlyport.h"
#include "upload.h"
#include "mrf24fj40ma.h"
#include "profile.h"
#include "pir.h"
#include "apds9300.h"
#include "ds1820.h"
#include "clock.h"
#include "optimization.h"
#include "task.h"

xTaskHandle hAppTask;



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
	
	WFCustomLoad();
	UARTWrite(1,"Connecting to custom...\r\n");
	WFConnect(WF_CUSTOM);

	while (WFStatus != CONNECTED);
	
	/*vTaskDelay(100);
	mrf24j40ma_init();//802.15.4 Reader init
	Set_Device_MAC_PANID(EADR0,EADR1,EADR2,EADR3,EADR4,EADR5,EADR6,EADR7,PANIDH,PANIDL,SADRH,SADRL);//Set PANID,MAC, Short Addr
	GET_Device_MAC_PANID();//check for some Std parameters of Reader(PANID, MAC, Short Addr)
	GET_Device_MAC_PANID();//check for some Std parameters of Reader(PANID, MAC, Short Addr)
	*/
	PIRInit();//Intialize PIR motion sensor
	DS1820Init();//Intialize Temperature sensor
	ClockInit();//Intialize RTCC
	APDSInit();//Intialize Light sensor
	Random_Delay();//Put random delay while initialization
	
	UARTWrite(1,"\r\nEntering in infinite loop...\r\n");
	
	
	
	if(profile.OffTime < DEFAULT_OFFTIME || profile.OnTime < DEFAULT_ONTIME)
	{
		profile.OptMode = OPT_DISABLED ;
	}
	if(profile.SecurityType == WF_SECURITY_WPA_WITH_KEY || profile.SecurityType == WF_SECURITY_WPA_WITH_PASS_PHRASE || profile.SecurityType == WF_SECURITY_WPA2_WITH_KEY || profile.SecurityType == WF_SECURITY_WPA2_WITH_PASS_PHRASE )
	{
		profile.OptMode = OPT_DISABLED;
	}
	
	if(profile.AppEnable)
	{
	//AppTask();
	if(hAppTask==NULL)
	{xTaskCreate(AppTask,(signed char*) "App" , (configMINIMAL_STACK_SIZE * 5), NULL, configMAX_PRIORITIES - 1, &hAppTask);
	}
	}
	
	//optimization
	if(profile.OptMode==OPT_ENABLED)
	{
	/*if(hOptTask == NULL)
	{
		xTaskCreate(optimization,(signed char*) "Opt" , (configMINIMAL_STACK_SIZE * 2), NULL, configMAX_PRIORITIES - 1, &hOptTask);
	}*/
	optimization();
	}
	
	

	while(1); //Keep in this function infinitely if Apptask is not called (or if the data is not to be uploaded)
}
