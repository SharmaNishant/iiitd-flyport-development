#include "upload.h"
#include "time.h"
#include "rtcc.h"
#include "TCPlib.h"
#include "profile.h"
#include "pir.h"
#include "ds1820.h"
#include "clock.h"
#include "apds9300.h"
#include "mrf24fj40ma.h"
#include "uart.h"
typedef union _MRF24J40_IFS
{
    BYTE Val;
    struct _MRF24J40_IFS_bits
    {
        BYTE RF_TXIF :1;
        BYTE :2;
        BYTE RF_RXIF :1;
        BYTE :4;
    }bits;
} MRF24J40_IFREG;
MRF24J40_IFREG flags;


	char pirstr[4];
	char tempstr[10];
	char lightstr[10];



// Number of sensors connected to the Flyport
#define NUM_SENSORS 3//4 if Tag is included 
//Number of TAGS available in the vicinity
#define MAX_TAGS 6
#define RSSI_INDEX 40 // Can be reduced based upon data received per 10 sec into sensor data string
#define NODE_ID 1  // Fix me 
char serverport[8]="8001"; // Fix me //server port for TagData

//size of the json_object
int len;

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
//char channel[21][12];//channel allocation for TAGS Data

// units in which reading is to be expressed
char unit[20] = "";
// data readings to be sent in json_object
char version[] = "1.0.0";

// data buffer to store json_object
char sensdata[NUM_SENSORS][128];
//char senstagdata[MAX_TAGS][RSSI_INDEX];//RSSI_INDEX had to be optimsed // manoj
char tagdata[128];// New string for loading tag data 
char senstagdata[1000];

char sreadings[128];//JSon string to store sensor data
//char tagreadings[MAX_TAGS][RSSI_INDEX];//JSON String to store RSSI stats form TAG



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
    "]}]}}\n"
};
/*			
char *json_object2[]={
			"{\"secretkey\":\"", profile.ApiKey,
			"\",\"data\":"
			,"{\"loc\":\"", profile.Location,
			"\",\"dname\":\"",profile.DeviceName,
			"\",\"sname\":\"",sensorname,
			"\",\"sid\":\"",sensorid,
			"\",\"timestamp\":\"",timestamp,
			"\",\"speriod\":\"",samplingperiod,
			"\",\"channels\":"
			"[{\"cname\":\"","channel0",
			"\",\"unit\":\"",unit,
			"\",\"readings\":[",tagreadings[0],"]}",
			",{\"cname\":\"","channel1",
			"\",\"unit\":\"",unit,
			"\",\"readings\":[",tagreadings[1],"]}",
			",{\"cname\":\"","channel2",
			"\",\"unit\":\"",unit,
			"\",\"readings\":[",tagreadings[2],"]}",
			",{\"cname\":\"","channel3",
			"\",\"unit\":\"",unit,
			"\",\"readings\":[",tagreadings[3],"]}",
			",{\"cname\":\"","channel4",
			"\",\"unit\":\"",unit,
			"\",\"readings\":[",tagreadings[4],"]}",
			",{\"cname\":\"","channel5",
			"\",\"unit\":\"",unit,
			"\",\"readings\":[",tagreadings[5],"]}]",			
			"}","}\n"
			};
*/
#define size 23
#define size_2 59

void UpdateTimestamp()
{
	time_t epoch_time; 
	char temp[20];
	epoch_time = ClockTimestamp();
	sprintf(temp,"%lu",epoch_time);
	strcpy(timestamp,temp);
	//UARTWrite(1, timestamp);//time stamp serial debug
	
}

int makeJson(char *buff, enum sensor_index index)
{
	int i;

	sprintf(samplingperiod, "%d", profile.SamplingPeriod);
	UARTWrite(1,"having a great time");

	// clear the content buffer
	buff[0] = '\0';

	switch(index)
	{
	case sensor_temperature:
		taskENTER_CRITICAL();
		strcpy(sreadings,sensdata[sensor_temperature]);
		taskEXIT_CRITICAL();
		sreadings[strlen(sreadings)-1]='\0';
		strcpy(unit, "celsius");
		strcpy(sensorname, "TemperatureSensor");
		for(i=0; i<size; ++i) 
		{
			strcat(buff, json_object[i]);
		}
		// clear the readings in tempreadings so that new 10 readings can be taken
		sensdata[sensor_temperature][0] = '\0';
		break;

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
		sensdata[sensor_pir][0] = '\0';
		break;
		
	case sensor_light:
		taskENTER_CRITICAL();
		strcpy(sreadings,sensdata[sensor_light]);
		taskEXIT_CRITICAL();
		sreadings[strlen(sreadings)-1]='\0';
		strcpy(unit, "none");
		strcpy(sensorname, "LightSensor");
		for(i=0; i<size; ++i) 
		{
			strcat(buff, json_object[i]);
		}	
		// clear the readings in pirreadings so that new 10 readings can be taken
		sensdata[sensor_light][0] = '\0';
		break;
	
//manoj	case sensor_tag:
//manoj		taskENTER_CRITICAL();
		/*for(i=0;i<MAX_TAGS;++i)
		{
			strcpy(tagreadings[i],senstagdata[i]);
			j=strlen(tagreadings[i]);
			tagreadings[i][j-1]='\0';
		}*/	
//manoj		strcpy(buff,senstagdata);
//manoj		taskEXIT_CRITICAL();
		//strcpy(unit, "none");
		//strcpy(sensorname, "Tag");
		/*for(i=0; i<size_2; ++i) 
		{
			strcat(buff, json_object2[i]);
		}*/	
		// clear the readings in pirreadings so that new 10 readings can be taken
//manoj		senstagdata[0] = '\0';
//manoj		break;
	}
	sreadings[0] = '\0';	
	// calculate the length of the json_object required for making the header
	len = strlen(buff);
	return len;
}


volatile BOOL alarmupload;
volatile BOOL alarmread;
volatile int alarmcount;

//Thread to handle JSON post
xTaskHandle hPostTask;

//PIR Sensor value
int PIR;
//Temperature Sensor value
double Temp;
//Light Sensor value
double Light;

//Request Buffer
char RequestBuffer[1024];

// HTTP Header format
char post_header[] = 
    "POST %s HTTP/1.1\r\n"
    "Host: %s\r\n"
	"Accept: */*\r\n"
	"Content-Length: %d\r\n"
    "Content-Type: application/json; charset=UTF-8\r\n"
	"Connection: close\r\n"
    "\r\n"
	"%s";

#ifdef uart
void UpTask()
{
	UARTWrite(1,"Creating Task\r\n");
	if(hPostTask == NULL)
	{
		//Creates the task dedicated to user code
		xTaskCreate(PostTask,(signed char*) "Post" , (configMINIMAL_STACK_SIZE * 4), NULL, tskIDLE_PRIORITY + 1, &hPostTask);	
	}
	UARTWrite(1,"Task initiated\r\n");
}


//void AppTask(char upir[10],char utemp[10],char ulight[10])
void AppTask(char* upir,char* utemp,char* ulight)
{	
		if(alarmcount == 0)
		{
			UpdateTimestamp();
		}
		
		//if(alarmread == 1)
		{	alarmread = 0;
			/*
			taskENTER_CRITICAL();//Put string copy in seperate tasks to avoid intermixing of sensor param's during strcpy
			//Temp = DS1820Read();		// get Temperature data
			sprintf(tempstr,"%0.1f,",utemp);//Copy Temp data in to Temp data string
			taskEXIT_CRITICAL();

			taskENTER_CRITICAL();
			//Light = APDSRead();			// get Light data
			sprintf(lightstr,"%0.1f,",ulight);//Copy Light data in to Light data string
			taskEXIT_CRITICAL();

			taskENTER_CRITICAL();
			//PIR = PIRRead();			// get PIR data
			sprintf(pirstr,"%d,",upir);	//Copy PIR data in to PIR data string
			taskEXIT_CRITICAL();*/
		UARTWrite(1,ulight);
		UARTWrite(1," -> light value\n");
			taskENTER_CRITICAL(); //Put in critical section such that it is not interrupted by the post task
			strcat(sensdata[sensor_temperature],utemp);
			strcat(sensdata[sensor_temperature],",");
			
			strcat(sensdata[sensor_light],ulight);
			strcat(sensdata[sensor_light],",");
			
			strcat(sensdata[sensor_pir],upir);
			strcat(sensdata[sensor_pir],",");
			taskEXIT_CRITICAL();
		/*	
			taskENTER_CRITICAL();
			strcat(senstagdata,tagdata);
			tagdata[0]='\0';
			taskEXIT_CRITICAL();
		*/
		}
		alarmcount++;
		
		if(alarmcount % profile.PublishPeriod == 0)
		{
			alarmupload = 1;
			alarmcount = 0;
			UARTWrite(1,"alarm\n");
		}
   }
#else
void AppTask()
{	
	
	volatile BYTE TAG_ID,RSSI_VAL,i;
	int c;
	char buf[20];//manoj
	UARTWrite(1,"Creating Task\r\n");
	if(hPostTask == NULL)
	{
		//Creates the task dedicated to user code
		xTaskCreate(PostTask,(signed char*) "Post" , (configMINIMAL_STACK_SIZE * 4), NULL, tskIDLE_PRIORITY + 1, &hPostTask);	
	}
	UARTWrite(1,"Task initiated\r\n");
	
	alarmflag = 0;
	alarmcount = 0;
	while(1)
	{       
		while(alarmflag != 1) vTaskDelay(1);
		alarmflag=0;
		
		if(alarmcount == 0)
		{
			UpdateTimestamp();
		}
		
		
		
		if(alarmcount % profile.SamplingPeriod == 0) //Assumption: Sampling period is integer
			alarmread = 1;
			
		if(alarmread == 1)
		{	alarmread = 0;
			
			taskENTER_CRITICAL();//Put string copy in seperate tasks to avoid intermixing of sensor param's during strcpy
			Temp = DS1820Read();		// get Temperature data
			sprintf(tempstr,"%0.1f,",Temp);//Copy Temp data in to Temp data string
			taskEXIT_CRITICAL();

			taskENTER_CRITICAL();
			Light = APDSRead();			// get Light data
			sprintf(lightstr,"%0.1f,",Light);//Copy Light data in to Light data string
			taskEXIT_CRITICAL();

			taskENTER_CRITICAL();
			PIR = PIRRead();			// get PIR data
			sprintf(pirstr,"%d,",PIR);	//Copy PIR data in to PIR data string
			taskEXIT_CRITICAL();
			
			/*
			taskENTER_CRITICAL();
			if(IOGet(MRF24J40_INT))
			{	
				flags.Val=lowRead(READ_INTSTAT);
				if(flags.bits.RF_RXIF==0)
				{
				++c;//Increment counter if reader_bug persists 
				UARTWrite(1,"802.15.4_Bug_Res");
				}
				if(c==20)
				{
				mrf24j40ma_init();//802.15.4 Reader init
				Set_Device_MAC_PANID(EADR0,EADR1,EADR2,EADR3,EADR4,EADR5,EADR6,EADR7,PANIDH,PANIDL,SADRH,SADRL);//Set PANID, MAC, Short Addr
				GET_Device_MAC_PANID();//check for some Std parameters of Reader(PANID, MAC, Short Addr)
				c=0;
				}
				if(flags.bits.RF_RXIF)
				{
				c=0;//Reset the reader_bug_catch counter
				i=highRead(0x300); //first byte of the receive buffer is the packet length (this 
				TAG_ID=highRead(0x308);		//TAG ID
				//TAG_ID=TAG_ID-100;//As tagID starts from 100-120 so we made it from 0-20
				//k=highRead(0x309);		//Battery level indicator 
				RSSI_VAL = highRead(0x300+i+2);	//RSSI value
				lowWrite(WRITE_RXFLUSH, 0x01);	//end of RXIF check
				/*if(TAG_ID>=0&&TAG_ID<=5)//store stats from TAG_ID 0-5
				{
				sprintf(senstagdata[TAG_ID],"%d,",RSSI_VAL);
				}*/
	//manoj			sprintf(tagdata,"%d,%d,%d,%s\n",NODE_ID,TAG_ID,RSSI_VAL,timestamp);
				//sprintf(buf,"\ntag %d sig %d",TAG_ID,RSSI_VAL);
				//UARTWrite(1,buf);
				//UARTWrite(1,senstagdata[TAG_ID]);
	//manoj			}
				//check_interrupt();
	//manoj		}
	//manoj		taskEXIT_CRITICAL();
				
			taskENTER_CRITICAL(); //Put in critical section such that it is not interrupted by the post task
			strcat(sensdata[sensor_temperature],tempstr);
			taskEXIT_CRITICAL();
			
			taskENTER_CRITICAL();
			strcat(sensdata[sensor_light],lightstr);
			taskEXIT_CRITICAL();
			
			taskENTER_CRITICAL();
			strcat(sensdata[sensor_pir],pirstr);
			taskEXIT_CRITICAL();
		/*	
			taskENTER_CRITICAL();
			strcat(senstagdata,tagdata);
			tagdata[0]='\0';
			taskEXIT_CRITICAL();
		*/
		}
		
		alarmcount++;
		if(alarmcount % profile.PublishPeriod == 0)
		{
			alarmupload = 1;
			alarmcount = 0;
		}
   }
}
#endif



void PostTask()
{
	char bufHTTPheader[1024];
	BOOL flagTCPisCON=FALSE;
	int z = 0;
	int i=0, length=0;
	static TCP_SOCKET Socket = INVALID_SOCKET;
	
	while(1)
	{
		while(alarmupload != 1) vTaskDelay(1);
		alarmupload=0;
		
		UARTWrite(1, "ALARM\r\n");
		
		// send all sensors data
		for(i=0;i<NUM_SENSORS;i++) //For each sensor, connection is established, data is posted and then connected is disconnected before moving to the new sensor
		{
			z=0;
			while(flagTCPisCON==FALSE && z <= MAX_RETRIES)
			{	//sprintf(tmp,"%s  %s  Z is %d\n",profile.ServerIP,profile.ServerPort,z);
				//UARTWrite(1,tmp);
					
				if(z==MAX_RETRIES) //If TCP connection is not established after max retries
				{	
					flagTCPisCON=FALSE;
					TCPClientClose( Socket);
					UARTWrite(1,"Error connecting to Server!!\r\n");
				}
				else
				{	
						if(Socket!=INVALID_SOCKET)
							{TCPClientClose( Socket);
							 UARTWrite(1,"Clean Socket close\r\n");
							}
					if(i>=0&&i<=2)
					{Socket=TCPClientOpen ( profile.ServerIP, profile.ServerPort);
					vTaskDelay(20);//50
					flagTCPisCON=TCPisConn(Socket);
					}
					if(i==3)
					{Socket=TCPClientOpen ( profile.ServerIP, serverport);
					vTaskDelay(20);//50
					flagTCPisCON=TCPisConn(Socket);
					}
						//if(flagTCPisCON==TRUE)
							//{UARTWrite(1,"Socket CoN\r\n");}
					
				    vTaskDelay(20);//50
				}
				z++;
			}
			
			if(flagTCPisCON)
			{
				UARTWrite(1,"Connected to Server!!\r\n");
				// obtain length of the data content
				length = makeJson(bufHTTPheader, i); //After this function, JSON header is in the bufHTTPheader
				// send the data length to makeheader which makes and sends the header
				length = sprintf(RequestBuffer, post_header, profile.ServerURL, profile.ServerIP, length, bufHTTPheader);
				// send the json_object to the server
				TCPWrite(Socket, RequestBuffer, length);
				//delay
				vTaskDelay(20);//200
				UARTWrite(1,bufHTTPheader);
				//Disconnect
				UARTWrite(1,"Disconnecting...\r\n"); //After sending every sensor data, the connection is disconnected
				TCPClientClose ( Socket );
				flagTCPisCON=FALSE;
				//UARTWrite(1,"Done.\r\n");
				
				//if(Socket==INVALID_SOCKET)//tcp state debug
				//{UARTWrite(1,"IS-1\r\n");
				//}
			}
		}
		UARTWrite(1,"End.\r\n");
	}
}
