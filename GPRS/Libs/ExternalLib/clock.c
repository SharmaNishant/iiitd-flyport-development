#include "clock.h"
#include "TCPlib.h"
#include "TCPconn.h"
#include "RTCClib.h"
#include "time.h"

// Returns the No. of Day taking its Name
int getDAY(char *day) {
	if(strcmp(day, "Sun") == 0)
		return 0;
	else if(strcmp(day, "Mon") == 0)
		return 1;
	else if(strcmp(day, "Tue") == 0)
		return 2;
	else if(strcmp(day, "Wed") == 0)
		return 3;
	else if(strcmp(day, "Thu") == 0)
		return 4;
	else if(strcmp(day, "Fri") == 0)
		return 5;
	else if(strcmp(day, "Sat") == 0)
		return 6;
		
	return -1;
}

// Returns the No. of Month taking its Name
int getMONTH(char *month) {
	if(strcmp(month, "Jan") == 0)
		return 0;
	else if(strcmp(month, "Feb") == 0)
		return 1;
	else if(strcmp(month, "Mar") == 0)
		return 2;
	else if(strcmp(month, "Apr") == 0)
		return 3;
	else if(strcmp(month, "May") == 0)
		return 4;
	else if(strcmp(month, "Jun") == 0)
		return 5;
	else if(strcmp(month, "Jul") == 0)
		return 6;
	else if(strcmp(month, "Aug") == 0)
		return 7;
	else if(strcmp(month, "Sep") == 0)
		return 8;
	else if(strcmp(month, "Oct") == 0)
		return 9;
	else if(strcmp(month, "Nov") == 0)
		return 10;
	else if(strcmp(month, "Dec") == 0)
		return 11;
		
	return -1;
}

// Initialization of Clock
void ClockInit() {
	UARTWrite(1, "\r\nInitialising Clock\r\n");
	TCP_SOCKET sockTCP;
	sockTCP.number = INVALID_SOCKET;
	
	// Buffer to store data recieved from TCP connection
	char Buff[1100];

	// Getting Date info from www.google.com
	// Infinite loop until we recieve data from server successfully
	// we cannot move forward without getting the date
	int currentState=0;
	int ret;
	char toUART[10];
	while(currentState < 4) {
		switch(currentState) {
			case 0:	ret = TCPConnect(&sockTCP, "www.google.com", "80");	
					break;
			
			case 1:	ret = TCPSend(&sockTCP, "GET / HTTP/1.0\r\n\r\n");
					break;
					
			case 2:	ret = TCPRecieve(&sockTCP, Buff);					
					break;
		
			case 3:	TCPClose(&sockTCP);
					break;
		}
		UARTWrite(1, "Socket Status: ");
		sprintf(toUART, "%d\r\n", sockTCP.status);
		UARTWrite(1, toUART);
		UARTWrite(1, "Socket Number: ");
		sprintf(toUART, "%d\r\n", sockTCP.number);
		UARTWrite(1, toUART);						
		if(ret == 0)	currentState++;
		else 	currentState=0;
	}	
	
	//char Buff[] = "adaDate: Sat, 20 Jul 2013 15:19:17 GMT\r\n";			// example buff for purpose of debugging
	
	// Retrieving Date from Buff
	char *ptr = strstr(Buff, "Date:");
	
	char wday[4], month[4];
	int mday, year, hr, min, sec;
		
	if(ptr == NULL) {
		UARTWrite(1, "Could not get Current Time\r\n");
	}
	else {
		
		//UARTWrite(1, "Got Date String\r\n");
		int loc = ptr - Buff;
		
		char date[40];
		
		int i=0;
		while(Buff[loc] != '\r') {
			/*char str[10];
			sprintf(str, "%c,%d\r\n", Buff[loc], Buff[loc]);
			UARTWrite(1, str);*/
			if(Buff[loc] == ':') {
				date[i++] = ' ';
				loc++;
			}
			else if(Buff[loc] == ',') {
				loc++;
			}
			else
				date[i++] = Buff[loc++];
		}
		date[i] = '\0';
		
		UARTWrite(1, date);
		
		sscanf(date, "Date  %3s %2i %3s %4i %2i %2i %2i GMT", wday, &mday, month, &year, &hr, &min, &sec);		
	}
	
	// Setting the RTCC
	struct tm mytime;
 
	//initialization of mytime
	mytime.tm_hour = hr;
	mytime.tm_min = min;
	mytime.tm_sec = sec;
	mytime.tm_mday = mday;
	mytime.tm_mon = getMONTH(month);
	mytime.tm_year = year-1900;
	mytime.tm_wday = getDAY(wday);

	RTCCSet(&mytime);
	
	
	// Configuring the Alarm
	RTCCAlarmConf(&mytime, REPEAT_INFINITE, EVERY_SEC, NO_ALRM_EVENT);
	
	// Starting the Alarm
	RTCCAlarmSet(1);
	
	UARTWrite(1, "Clock Set\r\nDate: ");
	UARTWrite(1, asctime(&mytime));
}
