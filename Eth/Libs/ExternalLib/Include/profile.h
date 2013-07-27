#define DHCP_DISABLED	0
#define DHCP_ENABLED	1

#define NETTYPE_ADHOC	0
#define NETTYPE_INFRA	1

//default applicaton values
#define DEFAULT_SPERIOD		1
#define DEFAULT_PPERIOD		10
#define DEFAULT_APIKEY		"abcdefghijklmnopqrstuvxyzabcdef"
#define DEFAULT_DEVICENAME	"SensorBoard"
#define DEFAULT_LOCATION	"Lab"
#define DEFAULT_SERVERIP	"192.168.1.1"
#define DEFAULT_SERVERPORT	"9000"
#define DEFAULT_SERVERURL	"/upload/wavesegment"
#define DEFAULT_SNTPENABLE	0
#define DEFAULT_APPENABLE	0

//default network configuration
#define DEFAULT_DHCP		DHCP_DISABLED
#define DEFAULT_NODEIP		"192.168.1.115"
#define DEFAULT_SUBNET		"255.255.255.0"
#define DEFAULT_GATEWAY		"192.168.1.1"
#define DEFAULT_DNS1		"192.168.1.1"
#define DEFAULT_DNS2		"192.168.1.1"

//default WiFi values
#define DEFAULT_SSID		"FlyportAdHoc"
#define DEFAULT_NETTYPE		NETTYPE_ADHOC
#define DEFAULT_SECURITY	WF_SECURITY_OPEN
#define DEFAULT_PASSKEY		""

//Parameters that are to be configured for uploading the sensed data to a central server
typedef struct tagPROFILE
{
	int Signature; //Unused
	
	//Profile variables
	unsigned char SamplingPeriod; //Need to be an integer
	unsigned char PublishPeriod; //Need to be an integer
	char ApiKey[33];
	char DeviceName[32];
	char Location[32];
	char ServerIP[16];
	char ServerPort[8];
	char ServerURL[48];
	unsigned char SNTPEnable; //SNTP server is the default specified in the internal flyport library (SNTP.c in the Microchip TCP/IP Stack)
	unsigned char AppEnable; // Decides whether to send data to central server or not; In Ad-hoc mode (and by default) AppEnable is disabled

	//TCP/IP variables
	unsigned char DHCPEnabled;
	char IPAddress[16];
	char SubnetMask[16];
	char Gateway[16];
	char DNS1[16];
	char DNS2[16];

	//Wireless variables
	char SSID[16];
	unsigned char NetType;
	unsigned char SecurityType;
	char SecurityKey[48];
} PROFILE;

extern PROFILE profile;

//Functions
void ProfileInit();
void ProfileSave();
void NETSave();
//void WifiSave();
void ProfileDefault();


