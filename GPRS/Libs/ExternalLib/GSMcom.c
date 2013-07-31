/*
	**** This file contains functions that are to be used only in LOW LEVEL MODE ****
*/

#include "GSMcom.h"
#include "LowLevelLib.h"
#include "string.h"

// flags to set check for specific sequence in GSM response
checkOK = FALSE;
checkCONNECT = FALSE;
checkDATA = FALSE;
getDATE = FALSE;

// buffer to store GSM response
char ReadString[100];
char Date[40];

// functions to check for specific sequence in GSM response, and wait for response until sequence is recieved
void getOK() {
	if(strcmp(ReadString, "OK\r\n") == 0) {
		UARTWrite(1, "\r\ngot OK\r\n");
		return;
	}
	else
		ReadGSM();
}

void getCONNECT() {
	if(strcmp(ReadString, "CONNECT\r\n") == 0) {
		UARTWrite(1, "\r\ngot CONNECT\r\n");
		return;
	}
	else
		ReadGSM();
}

void fgetDATE() {
	if(strstr(ReadString, "Date:") != NULL) {
		UARTWrite(1, "\r\ngot DATE\r\n");
		strcpy(Date, ReadString);
		getDATE = FALSE;
	}
	ReadGSM();
}

void getDATA() {
	char tmp[] = "+KTCP_DATA: ";
	UARTWrite(1,"\r\nChecking for Data\r\n");
	int i=0;
	//while(i<strlen(tmp)) {
		while(strstr(ReadString, "KTCP_DATA")==NULL) {
			//UARTWriteCh(1,ReadString[i]);
			//i++;
			ReadGSM();
		}
		//else
			UARTWrite(1, ReadString);
	//}
}

// function to get GSM response, and store it in ReadString
void ReadGSM() {
	
	while (LLBufferSize() == 0) {
		IOPut(p19, toggle);
		vTaskDelay(20);
	}
	char get;//, str[20];
	int i=0;
	while (LLBufferSize() > 0)
	{
		IOPut(p19, toggle);
		LLRead(&get, 1);
		ReadString[i++] = get;
		if(get=='\n') {
			break;
		}		
		vTaskDelay(2);
	}
	ReadString[i] = '\0';
	//UARTWrite(1, "get - ");
	UARTWrite(1, ReadString);
	vTaskDelay(5);
	
	// if ERROR is recieved, return
	if(strstr(ReadString, "ERROR") != NULL) {
		return;
	}
	// if any flag is set TRUE, go to corresponding function, to check for sequence
	if(getDATE == TRUE)
		fgetDATE();
	if(checkOK == TRUE)
		getOK();
	if(checkCONNECT == TRUE)
		getCONNECT();	
		
	checkOK = FALSE;
	checkCONNECT = FALSE;
	getDATE = FALSE;
}

// Prints the Signal Quality on UART
void CheckSignal() {
	LLWrite("AT+CSQ\r", 7);
	
	ReadGSM();
}

