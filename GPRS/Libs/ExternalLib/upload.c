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

// Size of JSON Object
#define size 23

//PIR Sensor value
int PIR;
//PSEUDO Sensor
int PSEUDO;

int sent[sensor_END];

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

/*
// defines the JSON accepted by XIVELY
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
			// set sent for PIR to length of sensdata that has been copied to buff
			sent[sensor_pir] = strlen(sensdata[sensor_pir]);
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
			// set sent for PSEUDO to length of sensdata that has been copied to buff
			sent[sensor_pseudo] = strlen(sensdata[sensor_pseudo]);
			break;
	}
	sreadings[0] = '\0';	
	// calculate the length of the json_object required for making the header
	int len = strlen(buff);
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

char pirstr[4];
char pseudostr[4];

void UpdateTimestamp() {
	struct tm ts;
	
	time_t epoch_time;
	
	RTCCGet(&ts);
	
	epoch_time = mktime(&ts);
	
	sprintf(timestamp,"%lu",epoch_time);
	UARTWrite(1, timestamp);
	UARTWrite(1, "\r\n");
}
	
/* Function to Sample the readings from the sensors,
	Have to make sure that this function does not take much time, to excecute,
	so use no blocking function in this, as this function is called from Main thread,
	and may interfere with maintainence of GSM State Machine */
void SampleTask() {
	// waiting until Flyport has Initialized successfully, and is ready to sample data
	if(AppTaskStart == 0) {
		//UARTWrite(1, "\r\nwaiting for UPLOAD.c\r\n");
		return;
	}
	
	// when alarm raises
	if(RTCCAlarmStat()) {
			if(alarmcount == 0)
			{
				UpdateTimestamp();
			}
			
			if(alarmcount % profile.SamplingPeriod == 0) //Assumption: Sampling period is integer
				alarmread = 1;
			
			if(alarmread == 1) {
				struct tm mytime;
				RTCCGet(&mytime);
				UARTWrite(1, "Sampling Data - ");
				UARTWrite(1, asctime(&mytime));

				
				/*	FOR UPLOADING DATA TO XIVELY.COM
				struct tm now;
				RTCCGet(&now);
				strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M.%SZ", &now);
				
				taskENTER_CRITICAL();
				if(alarmcount%2 == 0)	PIR = 1;
				else
				PIR = PIRRead();			// get PIR data
				sprintf(pirstr,"{\"at\":\"%s\",\"value\":\"%d\"},",timestamp,PIR);	//Copy PIR data in to PIR data string
				//UARTWrite(1, pirstr);
				taskEXIT_CRITICAL();*/
				
				/*taskENTER_CRITICAL();
				if(alarmcount%2 == 0)	PSEUDO = 1;
				else
				PSEUDO = pseudoRead();			// get PSEUDO data
				sprintf(pseudostr,"{\"at\":\"%s\",\"value\":\"%d\"},",timestamp,PSEUDO);	//Copy PSEUDO data in to PSEUDO data string
				//UARTWrite(1, pseudostr);
				taskEXIT_CRITICAL();*/
				
				/// FOR UPLOADING DATA TO SENSORACT
				taskENTER_CRITICAL();
				if(alarmcount%2 == 0)	PIR = 1;
				else
				PIR = PIRRead();			// get PIR data
				sprintf(pirstr,"%d,",PIR);	//Copy PIR data in to PIR data string
				taskEXIT_CRITICAL();							
				
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
			
			//If the buffer of data gets filled, clear the buffer
			if(strlen(sensdata[sensor_pir])>=590 ||  strlen(sensdata[sensor_pseudo])>=590) {
				UARTWrite(1, "\r\nClearing Data\r\n");
				sensdata[sensor_pir][0] = '\0';
				sensdata[sensor_pseudo][0] = '\0';
			}
			alarmcount++;
		}
}


/* Function to start Sampling, and connect to server to send data also,
	continously poll, to check if there is something recieved by SMS, or on UART,
*/
void AppTask()
{			
	TCP_SOCKET sendSOCK;
	sendSOCK.number = INVALID_SOCKET;
	
	AppTaskStart = 1;
	
	TCPConnect(&sendSOCK, profile.ServerIP, profile.ServerPort);
	
	alarmcount = 0;		
	
	while(1)
	{       
		// If something is recieved on UART buffer calls UARTcomm
		if(UARTBufferSize(1) > 0) {
			UARTcomm();			
		}
		
		// If something is recieved on SMS, calls SMSUpdate
		if(incomingSMS == TRUE) {
			incomingSMS = FALSE;
			SMSUpdate();
		}
		
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
				
				UARTWrite(1, RequestBuffer);
				//vTaskDelay(50);
								
				// sending data
				int ret = TCPSend(&sendSOCK, RequestBuffer);
				
				// if sending failed, connect to server again and continue
				if(ret == -1) {
					TCPConnect(&sendSOCK, profile.ServerIP, profile.ServerPort);
					continue;
				}
				
				// recieving the response
				ret = TCPRecieve(&sendSOCK, bufHTTPheader);
				
				// is response recieved, clear the sent data from readings buffer
				if(ret == 0) {
					UARTWrite(1, "\r\nData Successfully Sent");
					
					// stop sampling more data, to copy the rest of data to start
					AppTaskStart = 0;
					int j=0;
					int k=0;
					UARTWrite(1, "\r\nClearing Sent Data");
					for(j=sent[i] ; j<strlen(sensdata[i]) ; j++) {
						sensdata[i][k++] = sensdata[i][j];
					}
					sensdata[i][k] = '\0';
					UARTWrite(1, "\r\nData Cleared");
					
					// sent data cleared, start sampling
					AppTaskStart = 1;
				}
				// if response is not recieved, connect to server again
				else {
					TCPConnect(&sendSOCK, profile.ServerIP, profile.ServerPort);
				}
			}				
		}
   }
}

