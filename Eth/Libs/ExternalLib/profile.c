#include "NETlib.h"
#include "profile.h"
#include "libpic30.h"

PROFILE profile;

void NETSave()
{
	NETSetParam(DHCP_ENABLE, profile.DHCPEnabled ? ENABLED : DISABLED);
	NETSetParam(MY_IP_ADDR, profile.IPAddress);
	NETSetParam(SUBNET_MASK, profile.SubnetMask);
	NETSetParam(MY_GATEWAY, profile.Gateway);
	NETSetParam(PRIMARY_DNS, profile.DNS1);
	NETSetParam(SECONDARY_DNS, profile.DNS2);
	NETCustomSave();
}

/*void WifiSave()
{
	NETSetParam(SSID_NAME, profile.SSID);
	NETSetParam(NETWORK_TYPE, profile.NetType ? INFRASTRUCTURE : ADHOC);
	WFSetSecurity(profile.SecurityType, profile.SecurityKey, strlen(profile.SecurityKey), 0);
	NETCustomSave();
}*/

void ProfileSave()
{
	_erase_flash(0x2A400);
	int in1=0;
	int *wmem = (int*)&profile;
	unsigned int offset;
	
	UARTWrite(1, "Saving profile \r\n");

	NVMCON = 0x4003; // Initialize NVMCON
	TBLPAG = 0x2A400>>16; 						// Initialize PM Page Boundary SFR
	offset = 0x2A400 & 0xFFFF; 				// Initialize lower word of address
	__builtin_tblwtl(offset, 0x0A0B);     	// Write to address low word
	
	asm("DISI #5"); 	
	__builtin_write_NVM(); 	

	while (in1 < sizeof(PROFILE))
	{

		unsigned long progAddr = 0x2A400+in1; 			// Address of word to program
		unsigned int progDataL = (unsigned int) *(wmem);			// Data to program lower word

		NVMCON = 0x4003; // Initialize NVMCON
		TBLPAG = progAddr>>16; 						// Initialize PM Page Boundary SFR
		offset = progAddr & 0xFFFF; 				// Initialize lower word of address
		__builtin_tblwtl(offset, progDataL);     	// Write to address low word
		
		asm("DISI #5"); 	
		__builtin_write_NVM(); 	

		in1=in1+2;
		wmem++;
	}
}

void ProfileLoad()
{
	int in1 = 0;
	int *rmem = (int*) &profile;
	int rdoffset;
	int vRead,vRead1;
	long int addr1 = 0x2A400;
	
	UARTWrite(1, "Loading profile \r\n");

	while (in1 < sizeof(PROFILE))
	{
		TBLPAG = (((addr1+in1) & 0x7F0000)>>16);
		rdoffset = ((addr1+in1) & 0x00FFFF);
		asm("tblrdh.w [%1], %0" : "=r"(vRead1)     : "r"(rdoffset));
		asm("tblrdl.w [%1], %0" : "=r"(vRead)     : "r"(rdoffset));	

		*rmem = vRead;
		in1=in1+2;
		rmem = rmem + 1;
	}
}

void ProfileDefault()
{
	UARTWrite(1, "Creating Default profile \r\n");
	profile.SamplingPeriod = DEFAULT_SPERIOD;
	profile.PublishPeriod = DEFAULT_PPERIOD;
	strcpy(profile.ApiKey, DEFAULT_APIKEY);
	strcpy(profile.DeviceName, DEFAULT_DEVICENAME);
	strcpy(profile.Location, DEFAULT_LOCATION);
	strcpy(profile.ServerIP, DEFAULT_SERVERIP);
	strcpy(profile.ServerPort, DEFAULT_SERVERPORT);
	strcpy(profile.ServerURL, DEFAULT_SERVERURL);
	profile.SNTPEnable = DEFAULT_SNTPENABLE;
	profile.AppEnable = DEFAULT_APPENABLE;
	profile.DHCPEnabled = DEFAULT_DHCP;
	strcpy(profile.IPAddress, DEFAULT_NODEIP);
	strcpy(profile.SubnetMask, DEFAULT_SUBNET);
	strcpy(profile.Gateway, DEFAULT_GATEWAY);
	strcpy(profile.DNS1, DEFAULT_DNS1);
	strcpy(profile.DNS2, DEFAULT_DNS2);
	/*strcpy(profile.SSID, DEFAULT_SSID);
	profile.NetType = DEFAULT_NETTYPE;
	profile.SecurityType = DEFAULT_SECURITY;
	strcpy(profile.SecurityKey, DEFAULT_PASSKEY);*/
	
	NETSave();
	//WifiSave();
	ProfileSave();
}

void ProfileInit()
{
	if(NETCustomExist())
	{
		NETCustomLoad();
		ProfileLoad();
	}
	else
	{
		ProfileDefault();
	}
}


