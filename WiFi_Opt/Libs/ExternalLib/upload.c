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
#include "task.h"
#include "NETlib.h"
#include "optimization.h"
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

	int alarmcount;
int publish_flag;
int publish_check;


static TCP_SOCKET Socket = INVALID_SOCKET;
// Number of sensors connected to the Flyport
#define NUM_SENSORS 4
//Number of TAGS available in the vicinity
//#define MAX_TAGS 6
//#define RSSI_INDEX 40 // Can be reduced based upon data received per 10 sec into sensor data string
#define NODE_ID 1 // Fix me 
//char serverport[8]="9002"; // Fix me //server port for TagData
tWFNetwork xNetd;
extern BOOL ScanCompleted;
char msg[40];
//static TCP_SOCKET Socket = INVALID_SOCKET;
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
int channelname1 = 1; //channel for tagdata
int channelname2 = 2; //channel for tagdata

// units in which reading is to be expressed
char unit[20] = "";
// data readings to be sent in json_object
char version[] = "1.0.0";

// data buffer to store json_object
//char sensdata[NUM_SENSORS][128];

char tempdata[128];
char pirdata[128];
char lightdata[128];

//char senstagdata[MAX_TAGS][RSSI_INDEX];//RSSI_INDEX had to be optimsed // manoj
//char tagdata[128];// New string for loading tag data 
//char senstagdata[200];

char wifidata[300];
char senswifidata[300];

char sreadings[800];//JSon string to store sensor data
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

#define size 23
//#define size_2 59

void UpdateTimestamp()
{
	time_t epoch_time; 
	char temp[20];
	epoch_time = ClockTimestamp();
	sprintf(temp,"%lu",epoch_time);
	strcpy(timestamp,temp);
	UARTWrite(1, timestamp);//time stamp serial debug
	
}

int makeJson(char *buff, enum sensor_index index)
{
	int i;

	sprintf(samplingperiod, "%d", profile.SamplingPeriod);
	// clear the content buffer
	//buff[0] = '\0';
	vTaskSuspendAll();
	memset(buff, '\0', sizeof(buff));
	xTaskResumeAll();
	switch(index)
	{
	case sensor_temperature:
		vTaskSuspendAll();	
		strcpy(sreadings,tempdata);
		memset(tempdata, '\0',sizeof(tempdata));
		//taskEXIT_CRITICAL();
		sreadings[strlen(sreadings)-1]='\0';
		strcpy(unit, "celsius");
		strcpy(sensorname, "TemperatureSensor");
		for(i=0; i<size; ++i) 
		{
			strcat(buff, json_object[i]);
		}
		xTaskResumeAll();
		// clear the readings in tempreadings so that new 10 readings can be taken
		//sensdata[sensor_temperature][0] = '\0';
		break;

	case sensor_pir:
		vTaskSuspendAll();
		strcpy(sreadings,pirdata);
		memset(pirdata, '\0',sizeof(pirdata));
		//taskEXIT_CRITICAL();
		sreadings[strlen(sreadings)-1]='\0';
		strcpy(unit, "none");
		strcpy(sensorname, "PIRSensor");
		for(i=0; i<size; ++i) 
		{
			strcat(buff, json_object[i]);
		}	
		// clear the readings in pirreadings so that new 10 readings can be taken
		//sensdata[sensor_pir][0] = '\0';
		xTaskResumeAll();
		break;
		
	case sensor_light:
		vTaskSuspendAll();
		strcpy(sreadings,lightdata);
		memset(lightdata, '\0',sizeof(lightdata));
		//taskEXIT_CRITICAL();
		sreadings[strlen(sreadings)-1]='\0';
		strcpy(unit, "none");
		strcpy(sensorname, "LightSensor");
		for(i=0; i<size; ++i) 
		{
			strcat(buff, json_object[i]);
		}	
		// clear the readings in pirreadings so that new 10 readings can be taken
		//sensdata[sensor_light][0] = '\0';
		xTaskResumeAll();
		break;
	
	/*case sensor_tag:
		taskENTER_CRITICAL();
		strcpy(buff,senstagdata);
		taskEXIT_CRITICAL();
		// clear the readings so that new 10 readings can be taken
		senstagdata[0] = '\0';
		break;*/

	case sensor_wifi:
		vTaskSuspendAll();
		//taskENTER_CRITICAL();
		strcpy(buff,senswifidata);
		memset(wifidata, '\0',sizeof(wifidata));
		xTaskResumeAll();
		//taskEXIT_CRITICAL();
		//senswifidata[0]='\0';
		break;
	}
	/*vTaskSuspendAll();
	memset(sreadings, '\0', sizeof(sreadings));
	xTaskResumeAll();*/
	//sreadings[0] = '\0';	
	// calculate the length of the json_object required for making the header
	len = strlen(buff);
	return len;
}
EmptyData()
{
	vTaskSuspendAll();	//Sneihil
	memset(sreadings, '\0', sizeof(sreadings));
    memset(tempdata, '\0',sizeof(tempdata));
    memset(pirdata, '\0',sizeof(pirdata));
    memset(lightdata, '\0',sizeof(lightdata));
    memset(wifidata, '\0',sizeof(wifidata));
    xTaskResumeAll();	
}
volatile BOOL alarmupload;
volatile BOOL alarmread;

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


void AppTask()
{	
	char pirstr[4];
	char tempstr[10];
	char lightstr[10];
	int trigger=0;
	vTaskSuspendAll();	//Sneihil
	memset(sreadings, '\0', sizeof(sreadings));
    memset(tempdata, '\0',sizeof(tempdata));
    memset(pirdata, '\0',sizeof(pirdata));
    memset(lightdata, '\0',sizeof(lightdata));
    memset(wifidata, '\0',sizeof(wifidata));
    xTaskResumeAll();	//Sneihil
	
	volatile BYTE TAG_ID,RSSI_VAL,i;
	int c;
	char buf[100];//manoj
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
		while(alarmflag != 1) 
		vTaskDelay(1);
		alarmflag=0;
		
		if(alarmcount == 0)
		{
			UpdateTimestamp();
		}
		
		if(alarmcount % profile.SamplingPeriod == 0) //Assumption: Sampling period is integer
			alarmread = 1;
			alarmcount++;
		if(alarmread == 1)		
		{	
			alarmread = 0;
			vTaskSuspendAll();
			//taskENTER_CRITICAL();//Put string copy in seperate tasks to avoid intermixing of sensor param's during strcpy
			Temp = DS1820Read();		// get Temperature data
			sprintf(tempstr,"%0.1f,",Temp);//Copy Temp data in to Temp data string
			strcat(tempdata,tempstr);
			xTaskResumeAll();
			//taskEXIT_CRITICAL();

			//taskENTER_CRITICAL();
			vTaskSuspendAll();
			Light = APDSRead();			// get Light data
			sprintf(lightstr,"%0.1f,",Light);//Copy Light data in to Light data string
			strcat(lightdata,lightstr);
			xTaskResumeAll();
			//taskEXIT_CRITICAL();

			vTaskSuspendAll();
			//taskENTER_CRITICAL();
			PIR = PIRRead();			// get PIR data
			sprintf(pirstr,"%d,",PIR);	//Copy PIR data in to PIR data string
			strcat(pirdata,pirstr);
			xTaskResumeAll();
			//taskEXIT_CRITICAL();
			
			/*taskENTER_CRITICAL();
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
				RSSI_VAL = highRead(0x300+i+2);	//RSSI value
				lowWrite(WRITE_RXFLUSH, 0x01);	//end of RXIF check
				/*if(TAG_ID>=0&&TAG_ID<=5)//store stats from TAG_ID 0-5
				{
				sprintf(senstagdata[TAG_ID],"%d,",RSSI_VAL);
				}
				sprintf(tagdata,"%d,%d,%d,%d,%s\n",channelname1,NODE_ID,TAG_ID,RSSI_VAL,timestamp);				
				UARTWrite(1,tagdata);
				}
			}
			taskEXIT_CRITICAL();*/
			if(trigger==profile.ScanPeriod)		
			{
				if(profile.WifiEnable)
				{
				int s;
				UARTWrite(1,"Starting scan...\r\n");
				WFScan();		// WiFi scan starting
				vTaskDelay(200);
				while(1)
				{	
					//UARTWrite(1,"sCAN rUNNING..\r\n");
					if(ScanCompleted==TRUE)
					{
					//UARTWrite(1,"hello1");
					int i=1;
					int j,t;
					int x1;
					if(WFNetworkFound>=15)
					{
					for(i=1;i<16;++i)
					{
					xNetd= WFScanList(i);
					//UARTWrite(1,"hello2");
					sprintf(wifidata,"%d,%d,%s,%X%X%X%X%X%X,%d,%d,%s\n",channelname2,NODE_ID,xNetd.ssid,xNetd.bssid[0],xNetd.bssid[1],xNetd.bssid[2],xNetd.bssid[3],xNetd.bssid[4],xNetd.bssid[5],xNetd.signal,xNetd.channel,timestamp);
					UARTWrite(1,wifidata);
					strcat(senswifidata,wifidata);
					}	
					vTaskDelay(200);
					//UARTWrite(1,"hello3");
					wifidata[0]='\0';
					ScanCompleted = FALSE;		// End of the network scan
					break;
					}
					else
					{
					for(i=1;i<(WFNetworkFound+1);++i)
					{
					xNetd= WFScanList(i);
					//UARTWrite(1,"hello2");
					sprintf(wifidata,"%d,%d,%s,%X%X%X%X%X%X,%d,%d,%s\n",channelname2,NODE_ID,xNetd.ssid,xNetd.bssid[0],xNetd.bssid[1],xNetd.bssid[2],xNetd.bssid[3],xNetd.bssid[4],xNetd.bssid[5],xNetd.signal,xNetd.channel,timestamp);
					UARTWrite(1,wifidata);
					strcat(senswifidata,wifidata);
					}	
					vTaskDelay(200);
					//UARTWrite(1,"hello3");
					wifidata[0]='\0';
					ScanCompleted = FALSE;		// End of the network scan
					break;
					}
					}//break;
				}
				trigger=0;
				}
			}		
			/*taskENTER_CRITICAL(); //Put in critical section such that it is not interrupted by the post task
			strcat(sensdata[sensor_temperature],tempstr);
			taskEXIT_CRITICAL();
			
			taskENTER_CRITICAL();
			strcat(sensdata[sensor_light],lightstr);
			taskEXIT_CRITICAL();
			
			taskENTER_CRITICAL();
			strcat(sensdata[sensor_pir],pirstr);
			taskEXIT_CRITICAL();
			
			/*taskENTER_CRITICAL();
			strcat(senstagdata,tagdata);
			tagdata[0]='\0';
			taskEXIT_CRITICAL();*/
			
			/*taskENTER_CRITICAL();
			for(i=1;i<(WFNetworkFound+1);++i)
			{
			strcat(senswifidata,wifidata);
			UARTWrite(1,senswifidata);
			}
			wifidata[0]='\0';
			taskEXIT_CRITICAL();*/	
		
		}
		sprintf(buf,"trigger:%d\r\n",trigger);
		UARTWrite(1,buf);
		trigger++;
		//clean_data(); //to clean all the buffers if they are full
		if(alarmcount % profile.PublishPeriod == 0)
		{
			/*if(profile.SwitchEnable)
			{
				sprintf(msg, "RESUMING FROM SLEEP\r\n");
				UARTWrite(1, msg);
				//UpdateTimestamp();
				WFOn();
				//UpdateTimestamp();
				vTaskDelay(20);
				WFConnect(WF_CUSTOM);
				while (WFStatus != CONNECTED);
 
				if(Socket!=INVALID_SOCKET)
				{
					TCPClientClose( Socket);
					UARTWrite(1,"Clean Socket close\r\n");
				}
				UARTWrite(1,"Flyport connected.\r\n");
			}*/
			UpdateTimestamp();
			alarmupload = 1;
			alarmcount = 0;
		}
   }
}

void PostTask()
{
	char bufHTTPheader[1024];//2000 required for wifi sleep
	BOOL flagTCPisCON=FALSE;
	int z = 0;
	int i=0, length=0;
	char address[64];
	//static TCP_SOCKET Socket = INVALID_SOCKET;
	while(1)
	{
		while(alarmupload != 1) vTaskDelay(1); 
		alarmupload=0;
		taskENTER_CRITICAL();
		UARTWrite(1, "ALARM\r\n");
		taskEXIT_CRITICAL();
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
							{
								TCPClientClose( Socket);
							 	UARTWrite(1,"Clean Socket close\r\n");
							}
						/*memset(address, '\0', sizeof(address));                        
                		strcpy(address,profile.ServerIP);
                		strcat(address,":");
                		strcat(address,profile.ServerPort);
                		Socket=TCPClientOpen ( profile.ServerIP, profile.ServerPort);
                		vTaskDelay(70);//50
                		flagTCPisCON=TCPisConn(Socket);                                
                		vTaskDelay(70);//50	*/
					if(i>=0&&i<=2)
					{
					Socket=TCPClientOpen ( profile.ServerIP, profile.ServerPort);
					vTaskDelay(20);//50
					flagTCPisCON=TCPisConn(Socket);
					}
					if(i==3)
					{
					memset(address, '\0', sizeof(address));                        
					strcpy(address,profile.ServerIP);
					strcat(address,":");
					strcat(address,profile.DjangoPort);
					Socket=TCPClientOpen ( profile.ServerIP, profile.DjangoPort);
					vTaskDelay(70);//50
					flagTCPisCON=TCPisConn(Socket);                                
					vTaskDelay(70);
					/*Socket=TCPClientOpen ( profile.ServerIP, profile.DjangoPort);
					vTaskDelay(50);//50
					flagTCPisCON=TCPisConn(Socket);*/
					}
					/*if(i==4)
					{Socket=TCPClientOpen ( profile.ServerIP, serverport);
					vTaskDelay(50);//50
					flagTCPisCON=TCPisConn(Socket);
					}*/
						//if(flagTCPisCON==TRUE)
							//{UARTWrite(1,"Socket CoN\r\n");}
					
				    vTaskDelay(20);//50
				}
				z++;
			}
			
			if(flagTCPisCON)
			{
				memset(bufHTTPheader, '\0', sizeof(bufHTTPheader));
				UARTWrite(1,"Connected to Server!!\r\n");
				// obtain length of the data content
				length = makeJson(bufHTTPheader, i); //After this function, JSON header is in the bufHTTPheader
				//UARTWrite(1,bufHTTPheader);
				// send the data length to makeheader which makes and sends the header
				length = sprintf(RequestBuffer, post_header, profile.ServerURL, profile.ServerIP, length, bufHTTPheader);
				// send the json_object to the server
				TCPWrite(Socket, RequestBuffer, length);
				//delay
				vTaskDelay(20);//200
				//Disconnect
				UARTWrite(1,bufHTTPheader);
				UARTWrite(1,"Disconnecting...\r\n"); //After sending every sensor data, the connection is disconnected
				TCPClientClose ( Socket );
				flagTCPisCON=FALSE;
				UARTWrite(1,"Done.\r\n");
				
				//if(Socket==INVALID_SOCKET)//tcp state debug
				//{UARTWrite(1,"IS-1\r\n");
				//}
			}
		}
		taskENTER_CRITICAL();
		UARTWrite(1,"End.\r\n");
		taskEXIT_CRITICAL();
		//EmptyData();
		if(publish_flag==0)
		publish_check=1;
	}
}



void tcp_socket_handle()
{
	if(Socket!=INVALID_SOCKET)
	{
		TCPClientClose( Socket);
		taskENTER_CRITICAL();
		UARTWrite(1,"Clean Socket close by socket handle\r\n");
		taskEXIT_CRITICAL();
	}
}



void publish_handle()
{
	publish_flag=0;
	UARTWrite(1,"Handling Publish");
	if(profile.AppEnable==1)
	{	publish_check=0;
		UpdateTimestamp();
		alarmupload=1;
		alarmcount=0;
		while(publish_check==0);
	}
	publish_flag=1;
}


void clean_data()
{
	if(strlen(sreadings)>=300 || strlen(tempdata) > 128 || strlen(pirdata)> 128 || strlen(lightdata)> 128 || strlen(wifidata)>= 295)
	{
		UARTWrite(1,"\ncleaning buffers\n");
		EmptyData();}
}

