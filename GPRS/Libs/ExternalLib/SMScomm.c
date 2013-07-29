#include "SMScomm.h"
#include "taskFlyport.h"

int CheckSMS() {
	UARTWrite(1, "\r\nChecking SMS\r\n");
	SMSRead(2, 0);
	
	while(LastExecStat() == OP_EXECUTION)
		vTaskDelay(1);

	if(LastExecStat() != OP_SUCCESS)
	{
		return 0;
	}	
	else
	{
		UARTWrite(1, "\r\nGot SMS\r\n");
		UARTWrite(1, LastSmsText());
		
		SMSDelete(2, 0);
		while(LastExecStat() == OP_EXECUTION);
		if(LastExecStat() != OP_SUCCESS)
			UARTWrite(1, "\r\nError Deleting SMS\r\n");
		else
			UARTWrite(1, "\r\nDeleted SMS\r\n");
		return 1;
	}
}

int SendSMS(char *to, char *msg) {
	SMSSend(to, msg, FALSE);
	
	while(LastExecStat() == OP_EXECUTION);
	
	if(LastExecStat() != OP_SUCCESS) {
		UARTWrite(1, "Unable to Reply\r\n");
		return 0;
	}
	else {	
		UARTWrite(1, "SMS Replied\r\n");
		return 1;
	}
}
