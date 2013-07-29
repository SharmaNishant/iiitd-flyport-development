 #include "taskFlyport.h"
#include "RTCC.h"
#include "time.h"
#include "profile.h"
#include "clock.h"
#include "FreeRTOS.h"
#include "string.h"
#include "task.h"
#include "upload.h"

void FlyportTask()
{
	vTaskDelay(20);
    UARTWrite(1,"Flyport Task Started...\r\n");
	
	// Wait for GSM Connection successfull
    while(LastConnStatus() != REG_SUCCESS)
    {
    	vTaskDelay(20);
    	IOPut(p21, toggle);
    }
    IOPut(p21, on);
	vTaskDelay(20);
    UARTWrite(1,"Flyport registered on network!\r\n");
	
	ProfileInit();
	
	ClockInit();
	
	struct tm mytime;
	/*
	//UARTWrite(1, "\r\nStarting UARTUpdate Thread\r\n");
	xTaskHandle hUARTUpdate2;
	//if(hUARTUpdate == NULL)
	//{
		//Creates the task dedicated to user code
		xTaskCreate(UARTcomm,(signed char*) "GUIUpdate" , (configMINIMAL_STACK_SIZE * 2), NULL, tskIDLE_PRIORITY, &hUARTUpdate2);	
	//}
	vTaskDelay(10);
	*/
	AppTask();
	
	UARTWrite(1, "\r\nEntering infinite loop...\r\n");
	
    while(1)
    {
		//while(UARTread == 1)		
		//	vTaskDelay(500);
		//UARTwrite++;		
		if(RTCCAlarmStat()) {
			RTCCGet(&mytime);
			
			//time_t current = mktime(&mytime);
			UARTWrite(1, "\r\nFlyportTask ALARM\r\n");
			UARTWrite(1, asctime(&mytime));
		}
		//UARTwrite--;
	}
}
