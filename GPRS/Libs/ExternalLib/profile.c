#include "profile.h"
#include "GSM_Events.h"
#include "libpic30.h"
#include "string.h"
#include "SMScomm.h"

PROFILE profile;

//int UARTread;
//int UARTwrite;

void ProfileSave()
{
	_erase_flash(0x2A400);
	int in1=0;
	int *wmem = (int*)&profile;
	unsigned int offset;
	
	UARTWrite(1, "Saving profile \r\n");

	NVMCON = 0x4003; // Initialize NVMCON
	TBLPAG = 0x2A400>>16; 						// Initialize PM Page Boundary SFR
	offset = 0x2A400 & 0xFFFF; 				// Initialize lower word of address
	__builtin_tblwtl(offset, 0x0A0B);     	// Write to address low word
	
	asm("DISI #5"); 	
	__builtin_write_NVM(); 	

	while (in1 < sizeof(PROFILE))
	{

		unsigned long progAddr = 0x2A400+in1; 			// Address of word to program
		unsigned int progDataL = (unsigned int) *(wmem);			// Data to program lower word

		NVMCON = 0x4003; // Initialize NVMCON
		TBLPAG = progAddr>>16; 						// Initialize PM Page Boundary SFR
		offset = progAddr & 0xFFFF; 				// Initialize lower word of address
		__builtin_tblwtl(offset, progDataL);     	// Write to address low word
		
		asm("DISI #5"); 	
		__builtin_write_NVM(); 	

		in1=in1+2;
		wmem++;
	}
}

void ProfileLoad()
{
	int in1 = 0;
	int *rmem = (int*) &profile;
	int rdoffset;
	int vRead,vRead1;
	long int addr1 = 0x2A400;
	
	UARTWrite(1, "Loading profile \r\n");

	while (in1 < sizeof(PROFILE))
	{
		TBLPAG = (((addr1+in1) & 0x7F0000)>>16);
		rdoffset = ((addr1+in1) & 0x00FFFF);
		asm("tblrdh.w [%1], %0" : "=r"(vRead1)     : "r"(rdoffset));
		asm("tblrdl.w [%1], %0" : "=r"(vRead)     : "r"(rdoffset));	

		*rmem = vRead;
		in1=in1+2;
		rmem = rmem + 1;
	}
}

void ProfileDefault()
{
	UARTWrite(1, "Creating Default profile \r\n");
	profile.SamplingPeriod = DEFAULT_SPERIOD;
	profile.PublishPeriod = DEFAULT_PPERIOD;
	strcpy(profile.ApiKey, DEFAULT_APIKEY);
	strcpy(profile.DeviceName, DEFAULT_DEVICENAME);
	strcpy(profile.Location, DEFAULT_LOCATION);
	strcpy(profile.ServerIP, DEFAULT_SERVERIP);
	strcpy(profile.ServerPort, DEFAULT_SERVERPORT);
	strcpy(profile.ServerURL, DEFAULT_SERVERURL);
	
	strcpy(profile.APN, DEFUALT_APN);
	strcpy(profile.Login, DEFUALT_LOGIN);
	strcpy(profile.Password, DEFUALT_PASS);
	strcpy(profile.IPAddress, DEFAULT_IP);
	strcpy(profile.DNS1, DEFAULT_DNS1);
	strcpy(profile.DNS2, DEFAULT_DNS2);
	
	ProfileSave();
}

BOOL ProfileExist()
{
	int rdoffset;
	int vRead,vRead1;
    TBLPAG = ((0x2A400 & 0x7F0000)>>16);
    rdoffset = (0x2A400 & 0x00FFFF);
	asm("tblrdh.w [%1], %0" : "=r"(vRead1)     : "r"(rdoffset));
    asm("tblrdl.w [%1], %0" : "=r"(vRead)     : "r"(rdoffset));
		
	if (vRead == 0)
		return 1;
	else
		return 0;
}

void ProfileInit()
{
	//UARTread = 0;
	//UARTwrite = 0;
	if(ProfileExist())
		ProfileLoad();
	else
		ProfileDefault();
}


void ProfileDelete()
{
	_erase_flash(0x2A400);
}

void SendProfile() {
	taskENTER_CRITICAL();
	UARTWrite(1,"@values\r\n");
	char stringSend[10];
	sprintf(stringSend, "%d\r\n%d\r\n", profile.SamplingPeriod, profile.PublishPeriod);
	UARTWrite(1,stringSend);
	UARTWrite(1,profile.ApiKey);
	UARTWrite(1,"\r\n");
	UARTWrite(1,profile.DeviceName);
	UARTWrite(1,"\r\n");
	UARTWrite(1,profile.Location);
	UARTWrite(1,"\r\n");
	UARTWrite(1,profile.ServerIP);
	UARTWrite(1,"\r\n");
	UARTWrite(1,profile.ServerPort);
	UARTWrite(1,"\r\n");
	UARTWrite(1,profile.ServerURL);
	UARTWrite(1,"\r\n");
	UARTWrite(1,profile.APN);
	UARTWrite(1,"\r\n");
	UARTWrite(1,profile.Login);
	UARTWrite(1,"\r\n");
	UARTWrite(1,profile.Password);
	UARTWrite(1,"\r\n");
	UARTWrite(1,profile.IPAddress);
	UARTWrite(1,"\r\n");
	UARTWrite(1,profile.DNS1);
	UARTWrite(1,"\r\n");
	UARTWrite(1,profile.DNS2);
	UARTWrite(1,"\r\n@end-values\r\n");
	taskEXIT_CRITICAL();
}

void setProfile(int n, char *value) {
	
	switch(n) {
		case 0:	profile.SamplingPeriod = atoi(value);
				break;
		case 1:	profile.PublishPeriod = atoi(value);
				break;
		case 2:	strcpy(profile.ApiKey, value);
				break;
		case 3:	strcpy(profile.DeviceName, value);
				break;
		case 4:	strcpy(profile.Location, value);
				break;
		case 5:	strcpy(profile.ServerIP, value);
				break;
		case 6:	strcpy(profile.ServerPort, value);
				break;
		case 7:	strcpy(profile.ServerURL, value);
				break;
		case 8:	strcpy(profile.APN, value);
				break;
		case 9:	strcpy(profile.Login, value);
				break;
		case 10:	strcpy(profile.Password, value);
					break;
		case 11:	strcpy(profile.IPAddress, value);
					break;					
		case 12:	strcpy(profile.DNS1, value);
					break;
		case 13:	strcpy(profile.DNS2, value);
	}
}

void ReadProfile() {
	taskENTER_CRITICAL();
	UARTWrite(1, "@sendOK\r\n");
	vTaskDelay(20);
		
	char str[100];
	
	int arg=0;
	while(1) {
		while(UARTBufferSize(1) == 0);
		vTaskDelay(10);
		int len = UARTBufferSize(1);
		UARTRead(1, str, len);
		str[len] = '\0';
		//UARTWrite(1, str);
		//UARTWrite(1, "\r\n");
		if(strcmp(str, "@values\r\n") == 0);
		else if(strcmp(str, "@end-values\r\n") == 0) {
			break;			
		}
		else {	
			setProfile(arg++, str);
		}
		UARTWrite(1, "#OK#\r\n");
	}
	UARTWrite(1, "#OK#\r\n");
	ProfileSave();
	UARTWrite(1, "@recvOK\r\n");
	taskEXIT_CRITICAL();
}

void UARTcomm() {
	UARTWrite(1, "\r\nListening to UART...\r\n");
		
	//while(1) {
		char STR[1000];
		/*
		while(UARTBufferSize(1) == 0) {
			//UARTWrite(1,"Waiting for UART\r\n");
			vTaskDelay(1000);
		}*/
		//UARTread = 1;
		UARTWrite(1,"got something\r\n");
		vTaskDelay(50);
		//while(1) {
			//if(UARTwrite == 0) {
				//UARTWrite(1,"got something\r\n");
				int len = UARTBufferSize(1);
				UARTRead(1, STR, len);
				STR[len] = '\0';
				//UARTWrite(1,STR);
				//UARTWrite(1,"\r\n");				
				if(strcmp(STR, "@fetch\r\n") == 0) {					
					SendProfile();
					vTaskDelay(100);
				}
				else if(strcmp(STR, "@update\r\n") == 0) {
					ReadProfile();
					UARTWrite(1, "\r\nProfile Updated\r\n");
				}
				//break;
			//}
			//else
			//	vTaskDelay(1);
		//}
		//UARTread = 0;
	//}
}

int SetParam(char *msg, char *ptr, int offset, int param) {
	if(ptr == NULL)
		return -1;
	else {
		int loc = ptr - msg;
		loc+=offset;
		
		char tmp[49];
		
		int i=0;
		
		while(msg[loc] != '#') {
			/*char str[10];
			sprintf(str, "%c,%d\r\n", Buff[loc], Buff[loc]);
			UARTWrite(1, str);
			if(msg[loc] == ':') {
				tmp[i++] = ' ';
				loc++;
			}
			else if(tmp[loc] == ',') {
				loc++;
			}
			else*/
				tmp[i++] = msg[loc++];
		}
		tmp[i] = '\0';
		
		setProfile(param, tmp);
		return 0;
	}
}

void SMSParse(char *to, char *msg) {
	char str[161];
	
	if(strstr(msg, "PARAM#") != NULL) {			
		sprintf(str, "Sampling Period: %d\r\nPublish Period: %d\r\nAPI Key: %s\r\nDevice Name: %s\r\nLocation: %s\r\n", profile.SamplingPeriod, profile.PublishPeriod, profile.ApiKey, profile.DeviceName, profile.Location);
		SendSMS(to, str);
		sprintf(str, "Server IP: %s\r\nServer Port: %s\r\nServer URL: %s", profile.ServerIP, profile.ServerPort, profile.ServerURL);
		SendSMS(to, str);
		sprintf(str, "APN: %s\r\nLogin: %s\r\nPassword: %s\r\nIP Address: %s\r\nDNS 1: %s\r\nDNS 2: %s", profile.APN, profile.Login, profile.Password, profile.IPAddress, profile.DNS1, profile.DNS2);
		SendSMS(to, str);
	}
	
	else if(strstr(msg, "STATUS#") != NULL) {
		return "Will Send Status in Near Future";
	}
	
	else if(strstr(msg, "SET#") != NULL) {
		char tmp[161];
		
		if(strstr(msg, "SPERIOD ")) {
			char *ptr = strstr(msg, "SPERIOD ");
			
			int ret = SetParam(msg, ptr, strlen("SPERIOD "), 0);
			
			if(ret == 0) {
				sprintf(tmp, "Sampling Period SET: %d\r\n", profile.SamplingPeriod);
			}
			else {
				sprintf(tmp, "Sampling Period NOT SET: %d\r\n", profile.SamplingPeriod);
			}			
			SendSMS(to, tmp);
		}
		
		if(strstr(msg, "PPERIOD ")) {
			char *ptr = strstr(msg, "PPERIOD ");
			
			int ret = SetParam(msg, ptr, strlen("PERIOD "), 1);
			
			if(ret == 0) {
				sprintf(tmp, "Publish Period SET: %d\r\n", profile.PublishPeriod);
			}
			else {
				sprintf(tmp, "Publish Period NOT SET: %d\r\n", profile.PublishPeriod);
			}
			SendSMS(to, tmp);
		}
		
		if(strstr(msg, "API ")) {
			char *ptr = strstr(msg, "API ");
			
			int ret = SetParam(msg, ptr, strlen("API "), 2);
			
			if(ret == 0) {
				sprintf(tmp, "API Key SET: %s\r\n", profile.ApiKey);
			}
			else {
				sprintf(tmp, "API Key NOT SET: %s\r\n", profile.ApiKey);
			}
			SendSMS(to, tmp);
		}
		
		if(strstr(msg, "DEVICENAME ")) {
			char *ptr = strstr(msg, "DEVICENAME ");
			
			int ret = SetParam(msg, ptr, strlen("DEVICENAME "), 3);
			
			if(ret == 0) {
				sprintf(tmp, "Device Name SET: %s\r\n", profile.DeviceName);
			}
			else {
				sprintf(tmp, "Device Name NOT SET: %s\r\n", profile.DeviceName);
			}
			SendSMS(to, tmp);
		}
		
		if(strstr(msg, "LOCATION ")) {
			char *ptr = strstr(msg, "LOCATION ");
			
			int ret = SetParam(msg, ptr, strlen("LOCATION "), 4);
			
			if(ret == 0) {
				sprintf(tmp, "Location SET: %s\r\n", profile.Location);
			}
			else {
				sprintf(tmp, "Location NOT SET: %s\r\n", profile.Location);
			}
			SendSMS(to, tmp);
		}
		
		if(strstr(msg, "SERVERIP ")) {
			char *ptr = strstr(msg, "SERVERIP ");
			
			int ret = SetParam(msg, ptr, strlen("SERVERIP "), 5);
			
			if(ret == 0) {
				sprintf(tmp, "Server IP SET: %s\r\n", profile.ServerIP);
			}
			else {
				sprintf(tmp, "Server IP NOT SET: %s\r\n", profile.ServerIP);
			}
			SendSMS(to, tmp);
		}
		
		if(strstr(msg, "SERVERPORT ")) {
			char *ptr = strstr(msg, "SERVERPORT ");
			
			int ret = SetParam(msg, ptr, strlen("SERVERPORT "), 6);
			
			if(ret == 0) {
				sprintf(tmp, "Server Port SET: %s\r\n", profile.ServerPort);
			}
			else {
				sprintf(tmp, "Server Port NOT SET: %s\r\n", profile.ServerPort);
			}
			
			SendSMS(to, tmp);
		}
		
		if(strstr(msg, "SERVERURL ")) {
			char *ptr = strstr(msg, "SERVERURL ");
			
			int ret = SetParam(msg, ptr, strlen("SERVERURL "), 7);
			
			if(ret == 0) {
				sprintf(tmp, "Server URL SET: %s\r\n", profile.ServerURL);
			}
			else {
				sprintf(tmp, "Server URL NOT SET: %s\r\n", profile.ServerURL);
			}
			
			SendSMS(to, tmp);
		}
		
		if(strstr(msg, "APN ")) {
			char *ptr = strstr(msg, "APN ");
			
			int ret = SetParam(msg, ptr, strlen("APN "), 8);
			
			if(ret == 0) {
				sprintf(tmp, "APN SET: %s\r\n", profile.APN);
			}
			else {
				sprintf(tmp, "APN NOT SET: %s\r\n", profile.APN);
			}
			SendSMS(to, tmp);
		}
		
		if(strstr(msg, "LOGIN ")) {
			char *ptr = strstr(msg, "LOGIN ");
			
			int ret = SetParam(msg, ptr, strlen("LOGIN "), 9);
			
			if(ret == 0) {
				sprintf(tmp, "Login SET: %s\r\n", profile.Login);
			}
			else {
				sprintf(tmp, "Login NOT SET: %s\r\n", profile.Login);
			}
			SendSMS(to, tmp);			
		}
		
		if(strstr(msg, "PASSWORD ")) {
			char *ptr = strstr(msg, "PASSWORD ");
			
			int ret = SetParam(msg, ptr, strlen("PASSWORD "), 10);
			
			if(ret == 0) {
				sprintf(tmp, "Password SET: %s\r\n", profile.Password);
			}
			else {
				sprintf(tmp, "Password NOT SET: %s\r\n", profile.Password);
			}
			SendSMS(to, tmp);
		}
		
		if(strstr(msg, "IPADDRESS ")) {
			char *ptr = strstr(msg, "IPADDRESS ");
			
			int ret = SetParam(msg, ptr, strlen("IPADDRESS "), 11);
			
			if(ret == 0) {
				sprintf(tmp, "IP Address SET: %s\r\n", profile.IPAddress);
			}
			else {
				sprintf(tmp, "IP Address NOT SET: %s\r\n", profile.IPAddress);
			}
			SendSMS(to, tmp);
		}
		
		if(strstr(msg, "DNS1 ")) {
			char *ptr = strstr(msg, "DNS1 ");
			
			int ret = SetParam(msg, ptr, strlen("DNS1 "), 12);
			
			if(ret == 0) {
				sprintf(tmp, "DNS1 SET: %s\r\n", profile.DNS1);
			}
			else {
				sprintf(tmp, "DNS1 NOT SET: %s\r\n", profile.DNS1);
			}
			SendSMS(to, tmp);
		}
		
		if(strstr(msg, "DNS2 ")) {
			char *ptr = strstr(msg, "DNS2 ");
			
			int ret = SetParam(msg, ptr, strlen("DNS2 "), 13);
			
			if(ret == 0) {
				sprintf(tmp, "DNS2 SET: %s\r\n", profile.DNS2);
			}
			else {
				sprintf(tmp, "DNS2 NOT SET: %s\r\n", profile.DNS2);
			}
			SendSMS(to, tmp);
		}
	
		taskENTER_CRITICAL();
		ProfileSave();
		taskEXIT_CRITICAL();
	}
}

void SMSUpdate() {
	
	SMSRead(incomingIndexSMS, incomingMemSMS);
	
	vTaskDelay(20);
	while(LastExecStat() == OP_EXECUTION)
			vTaskDelay(1);
	
	vTaskDelay(20);
	if(LastExecStat() == OP_SUCCESS)
	{
		char sender[strlen(LastSmsSender())];
		strcpy(sender, LastSmsSender());
		
		char text[strlen(LastSmsText())];
		strcpy(text, LastSmsText());
		
		UARTWrite(1, "done!\r\n");
		UARTWrite(1, "SMS sender: ");
		UARTWrite(1, sender);
		UARTWrite(1, "\r\nSMS text: ");
		UARTWrite(1, text);
		UARTWrite(1, "\r\n");
		
		SMSParse(sender, text);
				
		SMSDelete(incomingIndexSMS, incomingMemSMS);
	
		while(LastExecStat() == OP_EXECUTION);
		
		if(LastExecStat() != OP_SUCCESS)
			UARTWrite(1, "Unable to Delete SMS\r\n");
		else
			UARTWrite(1, "SMS Deleted\r\n");
	}
	else {
		UARTWrite(1, "Unable to read SMS\r\n");
	}
}
