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
	
	ProfileInit();	// Loading Profile
	
	ClockInit();	// initialising RTCC/clock
	
	struct tm mytime;
	
	AppTask();	// start sampling/sending data
	
	// This part only excecutes is AppTask() is not running
	UARTWrite(1, "\r\nEntering infinite loop...\r\n");
	
    while(1)
    {
		if(RTCCAlarmStat()) {
			RTCCGet(&mytime);
			UARTWrite(1, "\r\nFlyportTask ALARM\r\n");
			UARTWrite(1, asctime(&mytime));
		}
	}
}
