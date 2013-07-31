#include "SMScomm.h"
#include "taskFlyport.h"

/* Function to send SMS,
	return 0 if successfully send,
	return -1 if failed
*/
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
