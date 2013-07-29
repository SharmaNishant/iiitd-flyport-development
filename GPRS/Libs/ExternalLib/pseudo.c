#include "HWLib.h"
#include "pseudo.h"

void pseudoInit()
{
	IOInit(PSEUDO_PORT, indown);
}

int pseudoRead()
{
	return IOGet(PSEUDO_PORT);
}
