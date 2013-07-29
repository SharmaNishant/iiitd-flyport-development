#include "GSM_Events.h"
	
/****************************************************************************
  FUNCTIONS TO HANDLE THE GSM EVENTS
****************************************************************************/

// INCOMING CALL
void OnRing(char* phoneNumber)
{
	#if defined(STACK_USE_UART)
	char buf[20];
	UARTWrite(1,"Event: On Ring\r\nPhone number: ");
	sprintf(buf, "%s", phoneNumber);
	UARTWrite(1,buf);
	UARTWrite(1,"\r\n");
	#endif
}

// No Carrier
void OnNoCarrier(char* phoneNumber)
{
	#if defined(STACK_USE_UART)
	char buf[20];
	UARTWrite(1,"Event: On No Carrier\r\nPhone number: ");
	sprintf(buf, "%s", phoneNumber);
	UARTWrite(1,buf);
	UARTWrite(1,"\r\n");
	#endif
}

// Busy
void OnBusy(char* phoneNumber)
{
	#if defined(STACK_USE_UART)
	char buf[20];
	UARTWrite(1,"Event: On Busy\r\nPhone number: ");
	sprintf(buf, "%s", phoneNumber);
	UARTWrite(1,buf);
	UARTWrite(1,"\r\n");
	#endif
}

int incomingIndexSMS;
BYTE incomingMemSMS;
BOOL incomingSMS = FALSE;

void OnSMSReceived(BYTE memtype, int index)
{
	#if defined(STACK_USE_UART)
	UARTWrite(1,"Event: On SMS Received\r\nmemory type: ");
	if(memtype == SM_MEM)
		UARTWrite(1, "SIM card");
	else
		UARTWrite(1, "Module Memory");
	UARTWrite(1, "\r\nmemory index: ");
	char smsRec[7];
	sprintf(smsRec, "%d\r\n", index);
	UARTWrite(1, smsRec);
	#endif
	
	incomingIndexSMS = index;
    incomingMemSMS = memtype;
    incomingSMS = TRUE;
}

void OnSMSSentReport(int msgreference, int msgreport)
{
	#if defined(STACK_USE_UART)
	char buf[30];
	UARTWrite(1,"Event: On SMS Sent Report\r\n");
	sprintf(buf, "message reference:%d\r\n", msgreference);
	UARTWrite(1, buf);
	sprintf(buf, "report value:%d\r\n", msgreport);
	UARTWrite(1, buf);
	#endif
}

void OnError(int error, int errorNumber)
{
	#if defined(STACK_USE_UART)
	char numErr[10];
	UARTWrite(1,"Event: On Error\r\nerror: ");
	sprintf(numErr, "%d\r\n", error);
	UARTWrite(1, numErr);
	UARTWrite(1, "error number: ");
	sprintf(numErr, "%d\r\n", errorNumber);
	UARTWrite(1, numErr);
	#endif
}

void OnRegistration(BYTE Status)
{
	#if defined(STACK_USE_UART)
	UARTWrite(1,"Event: On Registration\r\n");
	switch(Status)
	{
		case 0:
			UARTWrite(1, "Not registered\r\n");
			IOPut(p21, off);
			break;
		case 1:
			UARTWrite(1, "Registered on Network\r\n");
			IOPut(p21, on);
			break;
		case 2:
			UARTWrite(1, "Searching for new operator\r\n");
			IOPut(p21, off);
			break;
		case 3:
			UARTWrite(1, "Registration denied\r\n");
			IOPut(p21, off);
			break;
		case 4:
			UARTWrite(1, "Unkown registration status\r\n");
			IOPut(p21, off);
			break;
		case 5:
			UARTWrite(1, "Roaming\r\n");
			IOPut(p21, off);
			break;
	}		
	#endif
}
