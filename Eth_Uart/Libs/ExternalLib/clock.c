#include "clock.h"
#include "rtcc.h"
#include "TCPLib.h"
#include "profile.h"
#include "time.h"
void ClockInit() //Initializes RTCC based on SNTP and also configures RTCC to raise an alarm every second
{
	time_t now;
	struct tm *ts;
	DWORD epoch=0;
	DWORD epochtime=0xA2C2A;
	t_RTCC mytime, myalarm;
	
	if(profile.SNTPEnable)
	{
		UARTWrite(1,"waiting...");
		while(epoch<epochtime) {
			vTaskDelay(50);
			epoch=SNTPGetUTCSeconds();//Get SNTP time
		}
		UARTWrite(1, "done!\r\n");
	}
	
	//Get SNTP time
	//epoch = SNTPGetUTCSeconds(); //+ 19800; //Converting GMT to Indian time//manoj
	now = (time_t)epoch;
	ts = gmtime(&now); //Time Library related conversion  for GMT format
	//ts = localtime(&now); //Time Library related conversion  //Manoj

	//Set the RTC according to SNTP time
	mytime.year = ts->tm_year; //mytime is internal structure for Flyport defined for RTC
	mytime.month = ts->tm_mon+1;
	mytime.dweek = ts->tm_wday;
	mytime.day = ts->tm_mday;
	mytime.hour = ts->tm_hour;
	mytime.min = ts->tm_min;
	mytime.sec = ts->tm_sec;
	RTCCWrite (&mytime);
	
	myalarm = mytime; //Configuring RTC to raise an alarm every second (This will be used for sampling the sensors and transmitting the data)
	myalarm.sec = myalarm.sec + 1;
	RTCCSetAlarm(&myalarm, REPEAT_INFINITE, EVERY_SEC);
	RTCCRunAlarm(1);
}

time_t ClockTimestamp()
{
	t_RTCC myrtcc;
	time_t epoch_time;
	struct tm ts; 
	//char rtccString[200];
	RTCCRead(&myrtcc);
	/*sprintf(rtccString, "RTC %.4d-%.2d-%.2d / %.2d:%.2d.%.2d / %d\r\n",(myrtcc.year), myrtcc.month, myrtcc.day,
					myrtcc.hour, myrtcc.min, myrtcc.sec, myrtcc.dweek);
	UARTWrite(1, rtccString);*/
	ts.tm_year = 100+myrtcc.year;
	ts.tm_mon = myrtcc.month-1;
	ts.tm_wday = myrtcc.dweek;
	ts.tm_mday	= myrtcc.day;
	ts.tm_hour = myrtcc.hour;
	ts.tm_min = myrtcc.min;
	ts.tm_sec = myrtcc.sec;
	ts.tm_isdst = 0;//DST flag explicitly set to NULL to avoid random assigment during mktime call
	
	taskENTER_CRITICAL();//making it task critical as this function was making 3600 jump issue
	epoch_time = mktime(&ts);//jump was due to isdst flag in tm structure
	taskEXIT_CRITICAL();
	return epoch_time;
}
