#include "upload.h"
#include "pir.h"
#include "pseudo.h"
#include "FreeRTOS.h"
#include "task.h"
#include "profile.h"
#include "clock.h"
#include "time.h"
#include "TCPconn.h"
#include "GSM_Events.h"

#define size 23

//PIR Sensor value
int PIR;
//PSEUDO Sensor
int PSEUDO;

char sensdata[sensor_END][600];

int AppTaskStart = 0;
int alarmupload;
int alarmcount=0;
int alarmread=0;

//xTaskHandle hPostTask;

char sreadings[600];//JSon string to store sensor data

char cvalue[10];

// name of the sensor employed
char sensorname[40] = "";
// ID of the sensor
char sensorid[5] = "1";
// current timestamp
char timestamp[25] = "";
// sampling period
char samplingperiod[4] = "";
// channel name
char channelname[20] = "channel1";

// units in which reading is to be expressed
char unit[20] = "";
// data readings to be sent in json_object
char version[] = "1.0.0";

int len;

/*
// defines the JSON accepted by SensorAct
char *json_object[] = {
    "{\"id\" : \"", sensorname,
    "\",\"datapoints\":[", sreadings,
    "],\"current_value\":\"", cvalue,
    "\"}\r\n]\r\n}\r\n}\r\n"
};

int makeJson(char *buff, enum sensor_index index)
{
	int i, len;

	//sprintf(samplingperiod, "%d", profile.SamplingPeriod);

	// clear the content buffer
	buff[0] = '\0';

	switch(index)
	{
		case sensor_pir:
						taskENTER_CRITICAL();
						strcpy(sreadings,sensdata[sensor_pir]);
						taskEXIT_CRITICAL();
						//UARTWrite(1,sreadings);
						sreadings[strlen(sreadings)-1]='\0';
						sprintf(cvalue, "%d", PIR);
						strcpy(sensorname, "PIR");
						for(i=0; i<size; ++i) 
						{
							strcat(buff, json_object[i]);
						}	
						// clear the readings in pirreadings so that new 10 readings can be taken
						sensdata[sensor_pir][0] = '\0';
						break;
		
		case sensor_pseudo:
						taskENTER_CRITICAL();
						strcpy(sreadings,sensdata[sensor_pseudo]);
						taskEXIT_CRITICAL();
					//	UARTWrite(1,sreadings);
						sreadings[strlen(sreadings)-1]='\0';
						sprintf(cvalue, "%d", PSEUDO);
						strcpy(sensorname, "PSEUDO");
						for(i=0; i<size; ++i) 
						{
							strcat(buff, json_object[i]);
						}	
						// clear the readings in pirreadings so that new 10 readings can be taken
						sensdata[sensor_pseudo][0] = '\0';
						break;
	
	}
	sreadings[0] = '\0';	
	// calculate the length of the json_object required for making the header
	len = strlen(buff);
	return len;
}

// HTTP Header format
char post_header[] = 
	"{\r\n"
	"\"method\" : \"put\",\r\n"
	"\"resource\" : \"%s\",\r\n"
	"\"params\" : {},\r\n"
	"\"headers\" : {\"X-ApiKey\":\"h9AR6IBz5moaK5elchPPjTirPaG6eIfNfByJZU3SGMcx0ApV\"},\r\n"
	"\"body\" :\r\n"
    "{\r\n"
	"\"version\" : \"1.0.0\",\r\n"
	"\"datastreams\" : [%s";
*/

// defines the JSON accepted by SensorAct
char *json_object[] = {
    "{\"secretkey\":\"", profile.ApiKey,
    "\",\"data\":",
    "{\"loc\":\"", profile.Location,
    "\",\"dname\":\"", profile.DeviceName,
    "\",\"sname\":\"", sensorname,
    "\",\"sid\":", sensorid,
    ",\"timestamp\":", timestamp,
    ",\"speriod\":", samplingperiod,
    ",\"channels\":",
    "[{\"cname\":\"", channelname,
    "\",\"unit\":\"", unit,
    "\",\"readings\":[", sreadings,
    "]}]}}\r\n"
};

int makeJson(char *buff, enum sensor_index index)
{
	int i;

	sprintf(samplingperiod, "%d", profile.SamplingPeriod);

	// clear the content buffer
	buff[0] = '\0';

	switch(index)
	{
		case sensor_pir:
			taskENTER_CRITICAL();
			strcpy(sreadings,sensdata[sensor_pir]);
			taskEXIT_CRITICAL();
			sreadings[strlen(sreadings)-1]='\0';
			strcpy(unit, "none");
			strcpy(sensorname, "PIRSensor");
			for(i=0; i<size; ++i) 
			{
				strcat(buff, json_object[i]);
			}	
			// clear the readings in pirreadings so that new 10 readings can be taken
			//sensdata[sensor_pir][0] = '\0';
			break;
			
		case sensor_pseudo:
			taskENTER_CRITICAL();
			strcpy(sreadings,sensdata[sensor_pseudo]);
			taskEXIT_CRITICAL();
			sreadings[strlen(sreadings)-1]='\0';
			strcpy(unit, "none");
			strcpy(sensorname, "PseudoSensor");
			for(i=0; i<size; ++i) 
			{
				strcat(buff, json_object[i]);
			}	
			// clear the readings in pirreadings so that new 10 readings can be taken
			//sensdata[sensor_pseudo][0] = '\0';
			break;
	}
	sreadings[0] = '\0';	
	// calculate the length of the json_object required for making the header
	len = strlen(buff);
	return len;
}

// HTTP Header format
char post_header[] = 
    "POST %s HTTP/1.1\r\n"
    "Host: %s\r\n"
	"Accept: */*\r\n"
	"Content-Length: %d\r\n"
    "Content-Type: application/json; charset=UTF-8\r\n"
	"Connection: keep-alive\r\n"
    "\r\n"
	"%s";

char pirstr[10];
char pseudostr[10];
//char timestamp[50];

void UpdateTimestamp() {
	struct tm ts;
	
	time_t epoch_time;
	
	RTCCGet(&ts);
	
	epoch_time = mktime(&ts);
	
	sprintf(timestamp,"%lu",epoch_time);
	UARTWrite(1, timestamp);
	UARTWrite(1, "\r\n");
}
	
void SampleTask() {
	if(AppTaskStart == 0) {
		//UARTWrite(1, "\r\nwaiting for UPLOAD.c\r\n");
		return;
	}
	
	if(RTCCAlarmStat()) {
		
			if(alarmcount == 0)
			{
				UpdateTimestamp();
			}
			
			if(alarmcount % profile.SamplingPeriod == 0) //Assumption: Sampling period is integer
				alarmread = 1;
			
			if(alarmread == 1) {
				//UARTWrite(1, "\r\nCollecting Data\r\n");
				
				/*struct tm now;
				RTCCGet(&now);
				strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M.%SZ", &now);
				
				taskENTER_CRITICAL();
				if(alarmcount%2 == 0)	PIR = 1;
				else
				PIR = PIRRead();			// get PIR data
				sprintf(pirstr,"{\"at\":\"%s\",\"value\":\"%d\"},",timestamp,PIR);	//Copy PIR data in to PIR data string
				//UARTWrite(1, pirstr);
				taskEXIT_CRITICAL();*/
				
				taskENTER_CRITICAL();
				if(alarmcount%2 == 0)	PIR = 1;
				else
				PIR = PIRRead();			// get PIR data
				sprintf(pirstr,"%d,",PIR);	//Copy PIR data in to PIR data string
				taskEXIT_CRITICAL();
				
				/*taskENTER_CRITICAL();
				if(alarmcount%2 == 0)	PSEUDO = 1;
				else
				PSEUDO = pseudoRead();			// get PSEUDO data
				sprintf(pseudostr,"{\"at\":\"%s\",\"value\":\"%d\"},",timestamp,PSEUDO);	//Copy PSEUDO data in to PSEUDO data string
				//UARTWrite(1, pseudostr);
				taskEXIT_CRITICAL();*/
				
				taskENTER_CRITICAL();
				if(alarmcount%2 == 0)	PSEUDO = 1;
				else
				PSEUDO = pseudoRead();			// get PSEUDO data
				sprintf(pseudostr,"%d,",PSEUDO);	//Copy PSEUDO data in to PSEUDO data string
				taskEXIT_CRITICAL();
				
				taskENTER_CRITICAL();
				strcat(sensdata[sensor_pir],pirstr);
				taskEXIT_CRITICAL();
				
				taskENTER_CRITICAL();
				strcat(sensdata[sensor_pseudo],pseudostr);
				taskEXIT_CRITICAL();
				
				alarmread = 0;
			}
			if(strlen(sensdata[sensor_pir])>=540 ||  strlen(sensdata[sensor_pseudo])>=540) {
				UARTWrite(1, "\r\nClearing Data\r\n");
				sensdata[sensor_pir][0] = '\0';
				sensdata[sensor_pseudo][0] = '\0';
			}
			alarmcount++;
		}
}


void AppTask()
{	
	
	
	TCP_SOCKET sendSOCK;
	sendSOCK.number = INVALID_SOCKET;
	
	/*
	UARTWrite(1,"Creating Task\r\n");
	
	if(hPostTask == NULL)
	{
		//Creates the task dedicated to user code
		xTaskCreate(PostTask,(signed char*) "Post" , (configMINIMAL_STACK_SIZE * 4), NULL, tskIDLE_PRIORITY + 1, &hPostTask);	
	}
	vTaskDelay(5);
	UARTWrite(1,"Task initiated\r\n");
	*/
	
	//alarmflag = 0;
	
	AppTaskStart = 1;
	
	TCPConnect(&sendSOCK, profile.ServerIP, profile.ServerPort);
	
	alarmcount = 0;		
	
	while(1)
	{       
		//UARTWrite(1, "\r\nupload.c\r\n");
		
		if(UARTBufferSize(1) > 0) {
			UARTcomm();
			
		}
		
		if(incomingSMS == TRUE) {
			incomingSMS = FALSE;
			SMSUpdate();
		}
		
		//struct tm mytime;
		
		//RTCCGet(&mytime);
			
		//UARTWrite(1, "\r\nFlyportTask ALARM\r\n");
		//UARTWrite(1, asctime(&mytime));
		
	/*	while(!RTCCAlarmStat())
			vTaskDelay(1);
		
		if(alarmcount == 0)
		{
			//UpdateTimestamp();
		}
					
		if(alarmcount % profile.SamplingPeriod == 0) //Assumption: Sampling period is integer
			alarmread = 1;
			
		if(alarmread == 1)
		{	
		/*	alarmread = 0;
			
			struct tm now;
			RTCCGet(&now);
			strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M.%SZ", &now);
			
			taskENTER_CRITICAL();
			PIR = PIRRead();			// get PIR data
			sprintf(pirstr,"{\"at\":\"%s\",\"value\":\"%d\"},",timestamp,PIR);	//Copy PIR data in to PIR data string
			//UARTWrite(1, pirstr);
			taskEXIT_CRITICAL();
			
			taskENTER_CRITICAL();
			PSEUDO = pseudoRead();			// get PSEUDO data
			sprintf(pseudostr,"{\"at\":\"%s\",\"value\":\"%d\"},",timestamp,PSEUDO);	//Copy PSEUDO data in to PIR data string
			//UARTWrite(1, pseudostr);
			taskEXIT_CRITICAL();
			
			taskENTER_CRITICAL();
			strcat(sensdata[sensor_pir],pirstr);
			taskEXIT_CRITICAL();
			
			taskENTER_CRITICAL();
			strcat(sensdata[sensor_pseudo],pseudostr);
			taskEXIT_CRITICAL();
			
			while(alarmread != 0);
		}*/
		
		//alarmcount++;
		if(alarmcount % profile.PublishPeriod == 0)
		{
			alarmupload = 1;
			alarmcount = 0;
			
			int length;
			char bufHTTPheader[1024];
			char RequestBuffer[1072];
			
			UARTWrite(1, "ALARM\r\n");
			
			int i;
			for(i=0;i<sensor_END;i++) {
				length = makeJson(bufHTTPheader, i); //After this function, JSON header is in the bufHTTPheader
				
				// send the data length to makeheader which makes and sends the header
				length = sprintf(RequestBuffer, post_header, profile.ServerURL, profile.ServerIP, length, bufHTTPheader);
				
				//UARTWrite(1, RequestBuffer);
				//vTaskDelay(50);
				
				//int ret = -1;
				//while(ret != 0)
				int ret = TCPSend(&sendSOCK, RequestBuffer);
				
				if(ret == -1) {
					TCPClose(&sendSOCK);
					TCPConnect(&sendSOCK, profile.ServerIP, profile.ServerPort);
				}
				else {
					sensdata[sensor_pir][0] = '\0';
					sensdata[sensor_pseudo][0] = '\0';
				}
			}
					
		}
   }
}

