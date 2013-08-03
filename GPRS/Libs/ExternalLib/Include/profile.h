#include "HWlib.h"

//default applicaton values
#define DEFAULT_SPERIOD		5
#define DEFAULT_PPERIOD		900
#define DEFAULT_APIKEY		"3773bd8cf9594ca7a2a6c0074f73ace7"
#define DEFAULT_DEVICENAME	"Node-RJ"
#define DEFAULT_LOCATION	"RJ-Home"
#define DEFAULT_SERVERIP	"sensoract.iiitd.edu.in"
#define DEFAULT_SERVERPORT	"9000"
#define DEFAULT_SERVERURL	"/upload/wavesegment"

//GPRS network configuration
#define DEFUALT_APN		"mtnl.net"
#define DEFUALT_LOGIN	"mtnl"
#define DEFUALT_PASS	"mtnl123"
#define DEFAULT_IP		"0.0.0.0"
#define DEFAULT_DNS1	"0.0.0.0"
#define DEFAULT_DNS2	"0.0.0.0"


//Parameters that are to be configured for uploading the sensed data to a central server
typedef struct tagPROFILE
{
	int Signature; //Unused
	
	//Profile variables
	unsigned int SamplingPeriod; //Need to be an integer
	unsigned int PublishPeriod; //Need to be an integer
	char ApiKey[50];
	char DeviceName[32];
	char Location[32];
	char ServerIP[30];
	char ServerPort[8];
	char ServerURL[48];
	
	//GPRS variables
	char APN[10];
	char Login[10];
	char Password[10];
	char IPAddress[16];
	char DNS1[16];
	char DNS2[16];
	
} PROFILE;

extern PROFILE profile;

//extern int UARTread;
//extern int UARTwrite;

//Functions
BOOL ProfileExist();
void ProfileInit();
void ProfileSave();
void ProfileDefault();
void ProfileDelete();

void UARTcomm();
void SMSUpdate();

