#include "upload.h"
#include "pir.h"
#include "pseudo.h"
#include "FreeRTOS.h"
#include "task.h"
#include "profile.h"
#include "RTCClib.h"
#include "LowLevellib.h"
#include "clock.h"
#include "time.h"
#include "TCPconn.h"
#include "GSM_Events.h"

// this should be > (No. of Readings * 2) + 5 for SENSORACT
# define READINGS_BUFFER 150

// Size of JSON Object
#define size 23

//PIR Sensor value
int PIR;
//PSEUDO Sensor
int PSEUDO;

int sent[sensor_END];

char sensdata[sensor_END][READINGS_BUFFER];

int AppTaskStart = 0;
int alarmcount[sensor_END];
char tstamp[sensor_END][25];

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

char lenPrint[25];

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

int makeJson(char *buff, int index)
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
			strcpy(timestamp, tstamp[sensor_pir]);
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
			strcpy(timestamp, tstamp[sensor_pseudo]);
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

// strings to store sensor readings when sampled
char pirstr[4];
char pseudostr[4];

void UpdateTimestamp(int i) {
	struct tm ts;
	
	time_t epoch_time;
	
	RTCCGet(&ts);
	
	epoch_time = mktime(&ts);
	
	sprintf(tstamp[i],"%lu",epoch_time);
	UARTWrite(1, tstamp[i]);
	UARTWrite(1, "\r\n");
}

/* Function to Sample the readings from the sensors,
	Have to make sure that this function does not take much time, to excecute,
	so use no blocking function in this, as this function is called from Main thread,
	and may interfere with maintainence of GSM State Machine.
	
	***Do not use any loop inside this function***	*/
void SampleTask() {
	// waiting until Flyport has Initialized successfully, and is ready to sample data
	if(AppTaskStart == 0) {
		//UARTWrite(1, "\r\nwaiting for UPLOAD.c\r\n");
		return;
	}
	
	// when alarm raises
	if(RTCCAlarmStat()) {
		{
			if(alarmcount[sensor_pir] == 0)		// update PIR timestamp, after PIR data is sent to server (or when initialised) 
			{
				UpdateTimestamp(sensor_pir);
			}
			
			if(alarmcount[sensor_pir] % profile.SamplingPeriod == 0) {//Assumption: Sampling period is integer
				UARTWrite(1, "PIR : ");
				taskENTER_CRITICAL();			
				//PIR = PIRRead();			// get PIR data
				if(alarmcount[sensor_pir] % 2 == 0)	PIR = 1;
				else	PIR = 0;
				sprintf(pirstr,"%d,",PIR);	//Copy PIR data in to PIR data string
				taskEXIT_CRITICAL();	
				
				taskENTER_CRITICAL();
				strcat(sensdata[sensor_pir],pirstr);
				taskEXIT_CRITICAL();
				struct tm mytime;
				RTCCGet(&mytime);
				UARTWrite(1, "Sampling Data - ");
				UARTWrite(1, asctime(&mytime));
			}
			//If the buffer of data gets filled, clear the buffer
			if(strlen(sensdata[sensor_pir])>=READINGS_BUFFER-5) {
				UARTWrite(1, "\r\nPIR: Clearing Data\r\n");
				//sensdata[sensor_pir][0] = '\0';
				
				memset(sensdata[sensor_pir], '\0', sizeof(sensdata[sensor_pir]));
			}					
			
			alarmcount[sensor_pir]++;
		}
		{
			if(alarmcount[sensor_pseudo] == 0) // update PSEUDO timestamp, after PSEUDO data is sent to server (or when initialised)
			{
				UpdateTimestamp(sensor_pseudo);
			}
			
			if(alarmcount[sensor_pseudo] % profile.SamplingPeriod == 0) {//Assumption: Sampling period is integer
				UARTWrite(1, "PSEUDO : ");
				taskENTER_CRITICAL();
				
				//PSEUDO = pseudoRead();			// get PSEUDO data
				
				if(alarmcount[sensor_pseudo] % 2 == 0)	PSEUDO = 1;
				else	PSEUDO = 0;
				
				sprintf(pseudostr,"%d,",PSEUDO);	//Copy PSEUDO data in to PSEUDO data string
				taskEXIT_CRITICAL();
				
				taskENTER_CRITICAL();
				strcat(sensdata[sensor_pseudo],pseudostr);
				taskEXIT_CRITICAL();
				struct tm mytime;
				RTCCGet(&mytime);
				UARTWrite(1, "Sampling Data - ");
				UARTWrite(1, asctime(&mytime));
				
				sprintf(lenPrint, "strlen(PIR):%d\t", strlen(sensdata[sensor_pir]));
				UARTWrite(1, lenPrint);
				
				sprintf(lenPrint, "strlen(PSEUDO):%d\r\n", strlen(sensdata[sensor_pseudo]));
				UARTWrite(1, lenPrint);
			}
			//If the buffer of data gets filled, clear the buffer
			if(strlen(sensdata[sensor_pseudo])>=READINGS_BUFFER-5) {
				UARTWrite(1, "\r\nPSEUDO: Clearing Data\r\n");
				//sensdata[sensor_pseudo][0] = '\0';
				
				memset(sensdata[sensor_pseudo], '\0', sizeof(sensdata[sensor_pseudo]));
			}																					
			alarmcount[sensor_pseudo]++;	
		}
			
	}
}


// variable to store no. of failures to send data
int number_Fail = 0;

/* Function signals to start Sampling, and connect to server during publish period, send data, reciecve response and the close the socket,
	
	Also,
	continously poll, to check if there is something recieved by SMS, or on UART,
*/
void AppTask()
{			
	// initialising alarmcount, and start Sampling
	alarmcount[sensor_pir] = 0;	
	alarmcount[sensor_pseudo] = 0;	
	AppTaskStart = 1;
	
	 	
	/* variable to signal send data (regardless of publish period)
			1. for the first time
			2. when sending failed, then we continously try to send data, until sent successfully
	*/
	BOOL cont = TRUE;
	
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
		
		// If sending failed for 5 times, we go to LowLevelMode and back to StandardMode to reset GSM Modem
		if(number_Fail == 5) {
			LLModeEnable();
			
			STDModeEnable();	// Entering back to Standard Mode
	
			IOPut(p19, on);
			while(LastExecStat() == OP_EXECUTION) {					// Waiting until Standard Mode is Enabled	
				SampleTask();
				
			}
			IOPut(p19, off);
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
			IOPut(p21, off);
			while(LastConnStatus() != REG_SUCCESS)		// waiting until GSM is registered, as reentering to Standard Mode resets the GSM module
			{
				SampleTask();
			}
			IOPut(p21, on);
			
			number_Fail = 0;
		}
		
		/* Publishing Data to server
			1. when publish time reached, or
			2. first time, or
			3. last sending failed
		*/
		int i;
		if(alarmcount[0] % profile.PublishPeriod == 0 || cont)
		{
			cont = TRUE;
			UARTWrite(1, "ALARM\r\n");
			
			TCP_SOCKET sendSOCK;		// creating socket
			sendSOCK.number = INVALID_SOCKET;
			
			sprintf(lenPrint, "POST_strlen(PIR):%d\r\n", strlen(sensdata[sensor_pir]));
			UARTWrite(1, lenPrint);
			
			sprintf(lenPrint, "POST_strlen(PSEUDO):%d\r\n", strlen(sensdata[sensor_pseudo]));
			UARTWrite(1, lenPrint);
			
			int ret = TCPConnect(&sendSOCK, profile.ServerIP, profile.ServerPort);		// connecting to server			
			
			if(ret == -1) {
				number_Fail++;		// if connecting failed, we close socket and try again
			}
			else {			
				// If connection established
				for(i=0 ; i<sensor_END ; i++) {			// sending data and recieve response for each sensor
					cont = TRUE;
					
					int length;
					char bufHTTPheader[1024];
					char RequestBuffer[1072];
					
					length = makeJson(bufHTTPheader, i); //After this function, JSON header is in the bufHTTPheader
					
					// send the data length to makeheader which makes and sends the header
					length = sprintf(RequestBuffer, post_header, profile.ServerURL, profile.ServerIP, length, bufHTTPheader);
					
					UARTWrite(1, RequestBuffer);
					//vTaskDelay(50);
									
					// sending data
					ret = TCPSend(&sendSOCK, RequestBuffer);
					
					// if sending failed, stop the loop
					if(ret == -1) {
						number_Fail++;
						break;
					}
					
					// recieving the response
					ret = TCPRecieve(&sendSOCK, bufHTTPheader);
					
					// is response recieved, clear the sent data from readings buffer, and set cont variable to FALSE
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
						//sensdata[i][k] = '\0';
						
						char *ptr = &sensdata[i][k];
						memset(ptr, '\0', strlen(sensdata[i])-k);
						
						UARTWrite(1, "\r\nData Cleared");
						alarmcount[i] = 0;
						// sent data cleared, start sampling
						AppTaskStart = 1;
						number_Fail = 0;
						
						
						
						cont = FALSE;
					}
					// if response is not recieved, stop the loop
					else {
						number_Fail++;
						break;
					}
				}
			}
			
			// closing the socket
			TCPClose(&sendSOCK);						
		}
   }
}
