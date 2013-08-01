#include "TCPconn.h"
#include "DATAlib.h"
#include "Hilo.h"
#include "profile.h"
#include "GSMcom.h"
#include "upload.h"

int currentState;

char toUART[100];

/*
	Each function below returns
		-1 for FAILURE
		0 for SUCCESS
*/

/* Makes a TCP connection in Standard Mode
	If connection fails, MAX RETRIES = 2
*/
int TCPConnect(TCP_SOCKET *sockTcp, char *IP, char *Port) {

	currentState = 0;
	int Count = 0;
	while(currentState != 3) {
		
		if(Count == 3)
			return -1;
			
		switch(currentState) {
			case 0:	Count++;
					// Setup APN parameters
					UARTWrite(1, "Setup APN params\r\n");
					APNConfig(profile.APN, profile.Login, profile.Password, profile.IPAddress, profile.DNS1, profile.DNS2);
					
					vTaskDelay(20);
					
					// If we are not sampling, then we add delay until command is excecuted, else we call Sample Task continously
					if(AppTaskStart == 0) {
						while(LastExecStat() == OP_EXECUTION) {		// Checking if the APNConfig command is excecuted
							vTaskDelay(20);
							IOPut(p19, toggle);
						}
					}
					else {
						IOPut(p19, on);
						while(LastExecStat() == OP_EXECUTION) {
							SampleTask();
						}
						IOPut(p19, off);
					}
					// Choose next step:
					if(LastExecStat() != OP_SUCCESS)		// If the APNConfig command is excecuted successfully
						currentState = 0; // Repeat from case 1
					else
						currentState++; // execute next step
					break;
					
			case 1:	UARTWrite(1, "Connecting to TCP Server...");
					UARTWrite(1, IP);
					UARTWrite(1, ":");
					UARTWrite(1, Port);
					UARTWrite(1, "\r\n");
					//TCPClientOpen(&sockTcp, IP, Port);
					TCPClientOpen(sockTcp, IP, Port);
					
					// If we are not sampling, then we add delay until command is excecuted, else we call Sample Task continously
					if(AppTaskStart == 0) {
						while(LastExecStat() == OP_EXECUTION) {		// Checking if the TCPClientOpen command is excecuted
							vTaskDelay(20);
							IOPut(p19, toggle);
						}
					}
					else {
						IOPut(p19, on);
						while(LastExecStat() == OP_EXECUTION) {
							SampleTask();
						}
						IOPut(p19, off);
					}
					
					vTaskDelay(20);
					
					if(LastExecStat() != OP_SUCCESS)		// If the TCPClientOpen command is excecuted successfully
					{
						IOPut(p19, off);
						UARTWrite(1, "Errors on TCPClientOpen function!\r\n");	
						currentState = 0; // Repeat from case 1	
					}	
					else
					{
						IOPut(p19, off);
						UARTWrite(1, "\r\n TCPClientOpen OK \r\n");
						UARTWrite(1, "Socket Number: ");
						//sprintf(toUART, "%d\r\n", sockTcp.number);
						sprintf(toUART, "%d\r\n", sockTcp->number);
						UARTWrite(1, toUART);	
						currentState++; // execute next step
					}
					break;
					
			case 2:	UARTWrite(1, "Updating TCP_SOCKET Status...");
					//TCPStatus(&sockTcp);
					TCPStatus(sockTcp);
					
					// If we are not sampling, then we add delay until command is excecuted, else we call Sample Task continously
					if(AppTaskStart == 0) {
						while(LastExecStat() == OP_EXECUTION) {		// Checking if the TCPStatus command is excecuted
							vTaskDelay(20);
							IOPut(p19, toggle);
						}
					}
					else {
						IOPut(p19, on);
						while(LastExecStat() == OP_EXECUTION) {
							SampleTask();
						}
						IOPut(p19, off);
					}
					
					vTaskDelay(20);
					
					if(LastExecStat() != OP_SUCCESS)		// If the TCPStatus command is excecuted successfully
					{
						IOPut(p19, off);
						UARTWrite(1, "Errors on updating TCPStatus!\r\n");	
						currentState = 0; // Repeat from case 1	
					}
					else
					{
						IOPut(p19, off);
						UARTWrite(1, "\r\n TCP Socket Status:\r\n");
						//sprintf(toUART, "Status: %d\r\n", sockTcp.status);
						sprintf(toUART, "Status: %d\r\n", sockTcp->status);
						UARTWrite(1, toUART);
						//sprintf(toUART, "RxLen: %d\r\n", sockTcp.rxLen);
						sprintf(toUART, "RxLen: %d\r\n", sockTcp->rxLen);
						UARTWrite(1, toUART);	
						currentState++; // completed Task
					}
					break;
		}
	}
	return 0;
}

// Sends msg to TCP connection in Standard Mode, noted by the TCP_SOCKET
int TCPSend(TCP_SOCKET *sockTcp, char *msg) {
	
	UARTWrite(1, "Sending data...");
 		 
	TCPWrite(sockTcp, msg, strlen(msg));
	
	// If we are not sampling, then we add delay until command is excecuted, else we call Sample Task continously
	if(AppTaskStart == 0) {
		while(LastExecStat() == OP_EXECUTION) {		// Checking if the TCPWrite command is excecuted
			vTaskDelay(20);
			IOPut(p19, toggle);
		}
	}
	else {
		IOPut(p19, on);
		while(LastExecStat() == OP_EXECUTION) {
			SampleTask();
		}
		IOPut(p19, off);
	}
	
	vTaskDelay(20);
	
	
	if(LastExecStat() != OP_SUCCESS)		// If the TCPWrite command is excecuted successfully
	{
		IOPut(p19, off);
		UARTWrite(1, "Errors sending TCP data!\r\n");	
		return -1;
	}	
	else
	{
		IOPut(p19, off);
		UARTWrite(1, "\r\n TCP data sent:\r\n");
		sprintf(toUART, "Status: %d\r\n\r\n\r\n", sockTcp->status);
		UARTWrite(1, toUART);	
		return 0;
	}

}

// Sends data to TCP connection in Low Level Mode
int lTCPSend(TCP_SOCKET *sockTcp, char *msg) {
	LLModeEnable();		// Entering Low Level Mode
	
	UARTWrite(1, "Sending data...");
 		 
	char toGSM[20];
	sprintf(toGSM, "AT+KTCPSND=%d,%d\r", sockTcp->number, strlen(msg));		// sends data to given socket no.
	int i=0;
	
	UARTWrite(1, toGSM);
	LLWrite(toGSM,strlen(toGSM));
	checkCONNECT = TRUE;
	ReadGSM();							// gets "CONNECT" keyword from the GSM Modem
	while (LLBufferSize() != 0)
		ReadGSM();
	
	char EOFpattern[] = "--EOF--Pattern--";
	
	UARTWrite(1, "\r\nWriting Data\r\n");
	LLWrite(msg,strlen(msg));			// sending msg
	
	vTaskDelay(50);
	LLWrite(EOFpattern, strlen(EOFpattern));		// sending EOF_PATTERN
	
	UARTWrite(1, "\r\nData Sent\r\n");
	
	checkOK = TRUE;
	ReadGSM();		// getting "OK" from GSM Modem
	
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
	vTaskDelay(20);
	
	return 0;
}

// Recieves data from TCP connection in Standard Mode, noted by the TCP_SOCKET and puts data in inBuff
int TCPRecieve(TCP_SOCKET *sockTcp, char inBuff[]) {
	currentState = 0;
	
	int RxFlag=0;
	int Count = 0;
	while(currentState != 2) {
		if(Count == 50) {
			UARTWrite(1, "Read TIMEOUT\r\n");
			return -1;
		}
		switch(currentState) {
			case 0:	Count++;
					TCPStatus(sockTcp);
					
					// If we are not sampling, then we add delay until command is excecuted, else we call Sample Task continously
					if(AppTaskStart == 0) {
						while(LastExecStat() == OP_EXECUTION) {		// Checking if the TCPStatus command is excecuted
							vTaskDelay(20);
							IOPut(p19, toggle);
						}
					}
					else {
						IOPut(p19, on);
						while(LastExecStat() == OP_EXECUTION) {
							SampleTask();
						}
						IOPut(p19, off);
					}
					
					vTaskDelay(20);
					if(LastExecStat() != OP_SUCCESS)		// If the TCPStatus command is excecuted successfully
					{
						UARTWrite(1, "Errors on TCPStatus!\r\n");	
						currentState = 0; // Repeat from case 1	
					}	
					else
					{	
						if(sockTcp->rxLen > 0)
							currentState++; // execute next step
						else if(RxFlag == 0) {
							currentState = 0; // repeat
							vTaskDelay(50);
						}
						else
							currentState = 2;
					}
					break;
					
			case 1:	RxFlag = sockTcp->rxLen;
					TCPRead(sockTcp, inBuff, RxFlag);
					inBuff[RxFlag] = '\0';
					
					// If we are not sampling, then we add delay until command is excecuted, else we call Sample Task continously
					if(AppTaskStart == 0) {
						while(LastExecStat() == OP_EXECUTION) {		// Checking if the TCPRead command is excecuted
							vTaskDelay(20);
							IOPut(p19, toggle);
						}
					}
					else {
						IOPut(p19, on);
						while(LastExecStat() == OP_EXECUTION) {
							SampleTask();
						}
						IOPut(p19, off);
					}
					vTaskDelay(20);
					if(LastExecStat() != OP_SUCCESS)		// If the TCPRead command is excecuted successfully
					{
						UARTWrite(1, "Errors on reading TCP buffer!\r\n");	
						return -1;
					}	
					else
					{
						UARTWrite(1, inBuff); // Write data to serial Monitor	
						UARTWrite(1, "\r\n<brk>\r\n");
						currentState = 0; // Update again TCP_SOCKET Status
					}
					break;
		}
	}
	return 0;
}

// Recieves data from TCP connection in Low Level Mode, noted by the TCP_SOCKET
int lTCPRecieve(TCP_SOCKET *sockTcp, char inBuff[]) {
	LLModeEnable();		// Entering Low Level Mode
	
	char toGSM[30];
	ReadGSM();
	getDATA();			// waiting to get notification for data recieved from GSM module
	//vTaskDelay(500);
	
	UARTWrite(1,"\r\nGOT DATA\r\n");
	
	sprintf(toGSM, "AT+KTCPRCV=%d,1460\r", sockTcp->number);
	LLWrite(toGSM, strlen(toGSM));
	checkOK = TRUE;
	getDATE = TRUE;
	ReadGSM();			// recieving Data
	while (LLBufferSize() != 0)
		ReadGSM();
	
	strcpy(inBuff, Date);
		
	sprintf(toGSM, "AT+KTCPCLOSE=%d,1\r", sockTcp->number);		// closing TCP socket
	LLWrite(toGSM, strlen(toGSM));
	checkOK = TRUE;
	ReadGSM();
	while (LLBufferSize() != 0)
		ReadGSM();
		
	LLWrite("AT+KTCPDEL=1\r", 13);		// deleting TCP socket
	checkOK = TRUE;
	ReadGSM();
	while (LLBufferSize() != 0)
		ReadGSM();
		
	STDModeEnable();		// Entering back to Standard Mode
	
	while(LastExecStat() == OP_EXECUTION) {						// Waiting until Standard Mode is Enabled
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
	vTaskDelay(20);
	
	return 0;
}


// Closing TCP connection in Standard Mode, noted by TCP_SOCKET
int TCPClose(TCP_SOCKET *sockTcp) {
	UARTWrite(1, "\r\n\r\nClosing socket...");
	//TCPClientClose(&sockTcp);
	TCPClientClose(sockTcp);

	if(AppTaskStart == 0) {
		while(LastExecStat() == OP_EXECUTION) {
			vTaskDelay(20);
			IOPut(p19, toggle);
		}
	}
	else {
		IOPut(p19, on);
		while(LastExecStat() == OP_EXECUTION) {
			SampleTask();
		}
		IOPut(p19, off);
	}
	
	vTaskDelay(20);
	if(LastExecStat() != OP_SUCCESS)
	{
		IOPut(p19, off);
		UARTWrite(1, "Errors on TCPClientClose!\r\n");	
		return -1;	
	}	
	else
	{
		UARTWrite(1, "Socket Closed!\r\n");	
		IOPut(p19, off);
		return 0;
	}
	
}
