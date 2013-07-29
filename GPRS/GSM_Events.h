#include "GenericTypeDefs.h"
#include "HWlib.h"
#include "HILOlib.h"

extern int incomingIndexSMS;
extern BYTE incomingMemSMS;
extern BOOL incomingSMS;

void OnRing(char* phoneNumber);
void OnNoCarrier(char* phoneNumber);
void OnBusy(char* phoneNumber);
void OnSMSReceived(BYTE memtype, int index);
void OnSMSSentReport(int msgreference, int msgreport);
void OnError(int error, int errorNumber);
void OnRegistration(BYTE Status);
