#include "GSMcom.h"
#include "LowLevelLib.h"
#include "string.h"

checkOK = FALSE;
checkCONNECT = FALSE;
checkDATA = FALSE;
getDATE = FALSE;

char ReadString[500];
char Date[40];

void getOK() {
	//UARTWrite(1,"\r\ncheck\r\n");
	//UARTWrite(1,ReadString);
	if(strcmp(ReadString, "OK\r\n") == 0) {
		UARTWrite(1, "\r\ngot OK\r\n");
		return;
	}
	else
		ReadGSM();
}

void getCONNECT() {
	//UARTWrite(1,"\r\ncheck\r\n");
	//UARTWrite(1,ReadString);
	if(strcmp(ReadString, "CONNECT\r\n") == 0) {
		UARTWrite(1, "\r\ngot CONNECT\r\n");
		return;
	}
	else
		ReadGSM();
}

void fgetDATE() {
	//UARTWrite(1,"\r\ncheck\r\n");
	//UARTWrite(1,ReadString);
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
		/*char gets[2];
		sprintf(gets, "%c", get);
		UARTWrite(1, gets);*/
		if(get=='\n') {
			//UARTWrite(1, "<brk>");
			break;
		}
		/*if(get=='\r') {
			UARTWrite(1, "<ret>");
		}*/
		vTaskDelay(2);
	}
	ReadString[i] = '\0';
	UARTWrite(1, "get - ");
	UARTWrite(1, ReadString);
	vTaskDelay(5);
	
	if(strstr(ReadString, "ERROR") != NULL) {
		return;
	}
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

void CheckSignal() {
	LLWrite("AT+CSQ\r", 7);
	
	ReadGSM();
}

