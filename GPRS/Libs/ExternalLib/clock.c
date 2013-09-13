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
	
	//char Buff[] = "adaDate: Thu, 08 Aug 2013 18:39:09 GMT\r\n";			// example buff for purpose of debugging
	
	// Retrieving Date from Buff
	char *ptr = strstr(Buff, "Date:");
	
	char wday[4], month[4];
	int mday, year, hr, min, sec;
		
	if(ptr == NULL) {
		UARTWrite(1, "Could not get Current Time\r\n");
	}
	else {
		int loc = ptr - Buff;
		
		char date[40];
		
		int i=0;
		while(Buff[loc] != '\r') {
			/*char str[10];					// for debugging purpose
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
		
		//int ret = 			// for debugging purpose
		sscanf(date, "Date  %3s %2d %3s %4d %2d %2d %2d GMT", wday, &mday, month, &year, &hr, &min, &sec);		
				
		/*char tmp[100];		// for debugging purpose
		
		sprintf(tmp, "\r\nret: %d, %s, %d-%s-%d %d:%d:%d\r\n", ret, wday, mday, month, year, hr, min, sec);
		
		UARTWrite(1, tmp);*/
	}
	
	// Setting the RTCC
	struct tm mytime;
	
	int wdayINT = getDAY(wday);		//get int value for Day of Week
	int monthINT = getMONTH(month);	 //get int value for Month of Year		
	
	// Adding Offset to GMT time of 5hr 30min i.e for IST
	{		
		min = min + 30;		// Add minute offset
		
		if(min > 59) {
			min = min - 60;
			hr = hr + 1;
		}
		
		hr = hr + 5;		// Add hour offset
		
		if(hr > 23) {
			hr = hr - 24;
			mday = mday + 1;
			wdayINT = wdayINT + 1;
		}
		
		int numberDays;
		switch(monthINT) {
			case 0:	case 2:	case 4:	case 6:	case 7:	case 9:	case 11:	
				numberDays = 31;
				break;
			
			case 3:	case 5:	case 8:	case 10:
				numberDays = 30;
				break;
				
			case 1:
				if(year%4 == 0) {
					numberDays = 29;
				}
				else {
					numberDays = 28;
				}
				break;
		}
		
		if(mday > numberDays) {
			mday = mday - numberDays;
			monthINT = monthINT + 1;
		}
		
		if(wdayINT > 6)
			wdayINT = wdayINT - 6;
		
		if(monthINT > 11) {
			monthINT = monthINT - 11;
			year = year + 1;
		}
	}
	
	//initialization of mytime
	mytime.tm_hour = hr;
	mytime.tm_min = min;
	mytime.tm_sec = sec;
	mytime.tm_mday = mday;
	mytime.tm_mon = monthINT;
	mytime.tm_year = year-1900;
	mytime.tm_wday = wdayINT;

	RTCCSet(&mytime);
	
	
	// Configuring the Alarm
	RTCCAlarmConf(&mytime, REPEAT_INFINITE, EVERY_SEC, NO_ALRM_EVENT);
	
	// Starting the Alarm
	RTCCAlarmSet(1);
	
	UARTWrite(1, "Clock Set\r\nDate: ");
	UARTWrite(1, asctime(&mytime));
}
