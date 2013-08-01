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
int alarmcount[sensor_END];
int alarmread[sensor_END];

char sampletime[sensor_END][25];

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
			strcpy(timestamp, sampletime[sensor_pir]);
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
			strcpy(timestamp, sampletime[sensor_pseudo]);
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

void UpdateTimestamp(i) {
	struct tm ts;
	
	time_t epoch_time;
	
	RTCCGet(&ts);
	
	epoch_time = mktime(&ts);
	
	sprintf(sampletime[i],"%lu",epoch_time);
	UARTWrite(1, sampletime[i]);
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
		int i=0;
		
		for( i=0 ; i<sensor_END ; i++ ) {
			if(alarmcount[i] == 0)
			{
				UpdateTimestamp(i);
			}
			
			if(alarmcount[i] % profile.SamplingPeriod == 0) //Assumption: Sampling period is integer
				alarmread[i] = 1;
			
			if(alarmread[i] == 1) {
				if(i == sensor_pir) {
					UARTWrite(1, "PIR : ");
					taskENTER_CRITICAL();
					if(alarmcount[sensor_pir]%2 == 0)	PIR = 1;
					else
					PIR = PIRRead();			// get PIR data
					sprintf(pirstr,"%d,",PIR);	//Copy PIR data in to PIR data string
					taskEXIT_CRITICAL();	
					
					taskENTER_CRITICAL();
					strcat(sensdata[sensor_pir],pirstr);
					taskEXIT_CRITICAL();
				}
				else if(i == sensor_pseudo) {
					UARTWrite(1, "PSEUDO : ");
					taskENTER_CRITICAL();
					if(alarmcount[sensor_pseudo]%2 == 0)	PSEUDO = 1;
					else
					PSEUDO = pseudoRead();			// get PSEUDO data
					sprintf(pseudostr,"%d,",PSEUDO);	//Copy PSEUDO data in to PSEUDO data string
					taskEXIT_CRITICAL();
					
					taskENTER_CRITICAL();
					strcat(sensdata[sensor_pseudo],pseudostr);
					taskEXIT_CRITICAL();
				}		
				struct tm mytime;
				RTCCGet(&mytime);
				UARTWrite(1, "Sampling Data - ");
				UARTWrite(1, asctime(&mytime));
				alarmread[i] = 0;
			}
						
			//If the buffer of data gets filled, clear the buffer
			if(strlen(sensdata[i])>=590) {
				UARTWrite(1, "\r\nClearing Data\r\n");
				sensdata[i][0] = '\0';
				//sensdata[sensor_pseudo][0] = '\0';
			}
			alarmcount[i]++;
		}
	}
}

// to check is the Flyport is connected to the server
BOOL isConnected = FALSE;

int number_Fail = 0;

/* Function to start Sampling, and connect to server to send data also,
	continously poll, to check if there is something recieved by SMS, or on UART,
*/
void AppTask()
{			
	TCP_SOCKET sendSOCK;
	sendSOCK.number = INVALID_SOCKET;
	
	alarmcount[sensor_pir] = 0;	
	alarmcount[sensor_pseudo] = 0;	
	AppTaskStart = 1;
	
	int ret = TCPConnect(&sendSOCK, profile.ServerIP, profile.ServerPort);
	if(ret == 0)
		isConnected = TRUE;
	 		
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
		
		if(number_Fail == 5) {
			LLModeEnable();
			
			STDModeEnable();	// Entering back to Standard Mode
	
			while(LastExecStat() == OP_EXECUTION) {					// Waiting until Standard Mode is Enabled	
				vTaskDelay(20);
				IOPut(p19, toggle);
			}
			
			vTaskDelay(20);
			if(LastExecStat() != OP_SUCCESS)
			{
				UARTWrite(1, "Errors Starting Standard Mode\r\n");	
				//return -1;	
			}	
			else
			{
				UARTWrite(1, "Standard Mode Enabled\r\n");	
				IOPut(p19, off);
				//return 0;
			}
			while(LastConnStatus() != REG_SUCCESS)		// waiting until GSM is registered, as reentering to Standard Mode resets the GSM module
			{
				vTaskDelay(20);
				IOPut(p21, toggle);
			}
			IOPut(p21, on);
		}
		
		int i;
		if(alarmcount[0] % profile.PublishPeriod == 0)
		{
			for(i=0 ; i<sensor_END ; i++) {			
				int length;
				char bufHTTPheader[1024];
				char RequestBuffer[1072];
				
				UARTWrite(1, "ALARM\r\n");
							
				if(!isConnected)
					ret = TCPConnect(&sendSOCK, profile.ServerIP, profile.ServerPort);					
				
				if(ret == -1) {
					number_Fail++;
					break;
				}
				else {
					isConnected = TRUE;
				}
				length = makeJson(bufHTTPheader, i); //After this function, JSON header is in the bufHTTPheader
				
				// send the data length to makeheader which makes and sends the header
				length = sprintf(RequestBuffer, post_header, profile.ServerURL, profile.ServerIP, length, bufHTTPheader);
				
				UARTWrite(1, RequestBuffer);
				//vTaskDelay(50);
								
				// sending data
				int ret = TCPSend(&sendSOCK, RequestBuffer);
				
				// if sending failed, connect to server again and continue
				if(ret == -1) {
					number_Fail++;
					isConnected = FALSE;
					break;
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
					alarmcount[i] = 0;
					// sent data cleared, start sampling
					AppTaskStart = 1;
					number_Fail = 0;
				}
				// if response is not recieved, connect to server again
				else {
					number_Fail++;
					isConnected = FALSE;
					break;
				}
			}				
		}
   }
}

