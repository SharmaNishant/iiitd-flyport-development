#include "HWLib.h"
#include "pir.h"

void PIRInit()
{
	IOInit(PIR_PORT, indown);
}

int PIRRead()
{
	return IOGet(PIR_PORT);
}
