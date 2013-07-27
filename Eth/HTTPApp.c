/* **************************************************************************																					
 *  Software License Agreement
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  This is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License (version 2) as published by 
 *  the Free Software Foundation AND MODIFIED BY OpenPicus team.
 *  
 *  ***NOTE*** The exception to the GPL is included to allow you to distribute
 *  a combined work that includes OpenPicus code without being obliged to 
 *  provide the source code for proprietary components outside of the OpenPicus
 *  code. 
 *  OpenPicus software is distributed in the hope that it will be useful, but 
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details. 
 * 
 * 
 * Warranty
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * WE ARE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 **************************************************************************/
 
 
/****************************************************************************
  SECTION 	Include
****************************************************************************/
#include "NETlib.h"
#include "TCPIP Stack/TCPIP.h"
#include "profile.h"
#include "ds1820.h"
#include "apds9300.h"
#if defined(STACK_USE_HTTP2_SERVER)




/****************************************************************************
  SECTION	Define
****************************************************************************/
#define __HTTPAPP_C


/****************************************************************************
  SECTION 	Authorization Handlers
****************************************************************************/
char String_post[128];

/*****************************************************************************
  FUNCTION	BYTE HTTPNeedsAuth(BYTE* cFile)

  This function is used by the stack to decide if a page is access protected.
  If the function returns 0x00, the page is protected, if returns 0x80, no 
  authentication is required
*****************************************************************************/
#if defined(HTTP_USE_AUTHENTICATION)
BYTE HTTPNeedsAuth(BYTE* cFile)
{
	//	If you want to restrict the access to some page, include it in the folder "protect"
	//	here you can change the folder, or add others
	if(memcmppgm2ram(cFile, (ROM void*)"protect", 7) == 0)
		return 0x00;		// Authentication will be needed later

	// You can match additional strings here to password protect other files.
	// You could switch this and exclude files from authentication.
	// You could also always return 0x00 to require auth for all files.
	// You can return different values (0x00 to 0x79) to track "realms" for below.

	return 0x80;			// No authentication required
}
#endif

/*****************************************************************************
  FUNCTION	BYTE HTTPCheckAuth(BYTE* cUser, BYTE* cPass)
	
  This function checks if username and password inserted are acceptable

  ***************************************************************************/
#if defined(HTTP_USE_AUTHENTICATION)
BYTE HTTPCheckAuth(BYTE* cUser, BYTE* cPass)
{
	if(strcmppgm2ram((char *)cUser,(ROM char *)"admin") == 0
		&& strcmppgm2ram((char *)cPass, (ROM char *)"flyport") == 0)
		return 0x80;		// We accept this combination
	
	// You can add additional user/pass combos here.
	// If you return specific "realm" values above, you can base this 
	//   decision on what specific file or folder is being accessed.
	// You could return different values (0x80 to 0xff) to indicate 
	//   various users or groups, and base future processing decisions
	//   in HTTPExecuteGet/Post or HTTPPrint callbacks on this value.
	
	return 0x00;			// Provided user/pass is invalid
}
#endif

/****************************************************************************
  SECTION	GET Form Handlers
****************************************************************************/
  
/****************************************************************************
  FUNCTION	HTTP_IO_RESULT HTTPExecuteGet(void)
	
  This function processes every GET request from the pages. In the example, 
  it processes only the led.cgi function, but you can add code to process 
  other GET requests.
*****************************************************************************/
HTTP_IO_RESULT HTTPExecuteGet(void)
{
	BYTE filename[20];
	
	// Load the file name
	// Make sure BYTE filename[] above is large enough for your longest name
	MPFSGetFilename(curHTTP.file, filename, 20);

	
	if(!memcmppgm2ram(filename, "protect/reboot.cgi",18))
	{
		Reset();
	}
	
	if(!memcmppgm2ram(filename, "protect/reset.cgi",17))
	{
		ProfileDefault();
		Reset();
	}
	
	return HTTP_IO_DONE;
}



/***************************************************************************
  SECTION	Dynamic Variable Callback Functions
****************************************************************************/

/****************************************************************************
  FUNCTION	void HTTPPrint_varname(void)
	
  Internal: See documentation in the TCP/IP Stack API or HTTP2.h for details.
 ***************************************************************************/

 /* In this section, we are creating different TCP packets corresponding to each of the field
 that is to be captured in the web page on the client side */
 
void HTTPPrint_SPERIOD()
{
	char val[4];
	sprintf(val, "%d", profile.SamplingPeriod);
	TCPPutString(sktHTTP, (BYTE*)val);
	return;
}

void HTTPPrint_PPERIOD()
{
	char val[4];
	sprintf(val, "%d", profile.PublishPeriod);
	TCPPutString(sktHTTP, (BYTE*)val);
	return;
}

void HTTPPrint_APIKEY()
{
	TCPPutString(sktHTTP, (BYTE*)profile.ApiKey);
	return;
}

void HTTPPrint_DEVICENAME()
{
	TCPPutString(sktHTTP, (BYTE*)profile.DeviceName);
	return;
}

void HTTPPrint_LOCATION()
{
	TCPPutString(sktHTTP, (BYTE*)profile.Location);
	return;
}

void HTTPPrint_SERVERIP()
{
	TCPPutString(sktHTTP, (BYTE*)profile.ServerIP);
	return;
}

void HTTPPrint_SERVERPORT()
{
	TCPPutString(sktHTTP, (BYTE*)profile.ServerPort);
	return;
}

void HTTPPrint_SERVERURL()
{
	TCPPutString(sktHTTP, (BYTE*)profile.ServerURL);
	return;
}

void HTTPPrint_IPADDR()
{
	TCPPutString(sktHTTP, (BYTE*)profile.IPAddress);
	return;
}

void HTTPPrint_SUBNET()
{
	TCPPutString(sktHTTP, (BYTE*)profile.SubnetMask);
	return;
}

void HTTPPrint_GATEWAY()
{
	TCPPutString(sktHTTP, (BYTE*)profile.Gateway);
	return;
}

void HTTPPrint_DNS1()
{
	TCPPutString(sktHTTP, (BYTE*)profile.DNS1);
	return;
}

void HTTPPrint_DNS2()
{
	TCPPutString(sktHTTP, (BYTE*)profile.DNS2);
	return;
}

/*void HTTPPrint_SSID()
{
	TCPPutString(sktHTTP, (BYTE*)profile.SSID);
	return;
}*/

void HTTPPrint_PIR()
{
	TCPPut(sktHTTP, IOGet(p7) ? '1' : '0');
	return;
}

void HTTPPrint_TEMP()
{
	char tmp[10];
	sprintf(tmp, "%lf", DS1820Read());
	TCPPutString(sktHTTP, (BYTE*)tmp);
	return;
}

void HTTPPrint_LIGHT()
{
	char tmp[10];
	sprintf(tmp, "%f", (double)APDSRead());
	TCPPutString(sktHTTP, (BYTE*)tmp);
	return;
}

#ifdef HTTP_USE_POST
/****************************************************************************
  FUNCTION	HTTP_IO_RESULT HTTPExecutePost(void)
	
  This function processes every POST request from the pages. 
*****************************************************************************/
HTTP_IO_RESULT HTTPExecutePost(void)
{

	// Resolve which function to use and pass along
	BYTE filename[20];
	int len;
	// Load the file name
	// Make sure BYTE filename[] above is large enough for your longest name
	MPFSGetFilename(curHTTP.file, filename, sizeof(filename));
	
	if(!memcmppgm2ram(filename, "protect/device.cgi",18)) 
	{
		profile.AppEnable = 0;
		profile.SNTPEnable = 0;
		while(curHTTP.byteCount)
		{
			// Check for a complete variable
			len = TCPFind(sktHTTP, '&', 0, FALSE);
			if(len == 0xffff)
			{
				// Check if is the last post, otherwise continue in the loop
				if( TCPIsGetReady(sktHTTP) == curHTTP.byteCount)
					len = curHTTP.byteCount - 1;
				else 
				{	
					return HTTP_IO_NEED_DATA; // No last post, we need more data
				}
			}
			if(len > HTTP_MAX_DATA_LEN - 2)
			{
				// Make sure we don't overflow
				curHTTP.byteCount -= TCPGetArray(sktHTTP, (BYTE*)String_post, len+1);
				continue;
			}

			len = TCPGetArray(sktHTTP,curHTTP.data, len+1);

			curHTTP.byteCount -= len;
			curHTTP.data[len] = '\0';
			HTTPURLDecode(curHTTP.data);
			
			if(memcmppgm2ram(curHTTP.data,(ROM void*)"SPERIOD", 7) == 0)
			{
				memcpy(String_post,(void*)&curHTTP.data[8], len-8);
				String_post[len-8]='\0';
				profile.SamplingPeriod = atoi(String_post);
			}
			else if(memcmppgm2ram(curHTTP.data,(ROM void*)"PPERIOD", 7) == 0)
			{
				memcpy(String_post,(void*)&curHTTP.data[8], len-8);
				String_post[len-8]='\0';
				profile.PublishPeriod = atoi(String_post);
			}
			else if(memcmppgm2ram(curHTTP.data,(ROM void*)"APIKEY", 6) == 0)
			{
				memcpy(String_post,(void*)&curHTTP.data[7], len-7);
				String_post[len-7]='\0';
				strcpy(profile.ApiKey, String_post);
			}
			else if(memcmppgm2ram(curHTTP.data,(ROM void*)"DEVICENAME", 10) == 0)
			{
				memcpy(String_post,(void*)&curHTTP.data[11], len-11);
				String_post[len-11]='\0';
				strcpy(profile.DeviceName, String_post);
			}
			else if(memcmppgm2ram(curHTTP.data,(ROM void*)"LOCATION", 8) == 0)
			{
				memcpy(String_post,(void*)&curHTTP.data[9], len-9);
				String_post[len-9]='\0';
				strcpy(profile.Location, String_post);
			}
			else if(memcmppgm2ram(curHTTP.data,(ROM void*)"SERVERIP", 8) == 0)
			{
				memcpy(String_post,(void*)&curHTTP.data[9], len-9);
				String_post[len-9]='\0';
				strcpy(profile.ServerIP, String_post);
			}
			else if(memcmppgm2ram(curHTTP.data,(ROM void*)"SERVERPORT", 10) == 0)
			{
				memcpy(String_post,(void*)&curHTTP.data[11], len-11);
				String_post[len-11]='\0';
				strcpy(profile.ServerPort, String_post);
			}
			else if(memcmppgm2ram(curHTTP.data,(ROM void*)"SERVERURL", 9) == 0)
			{
				memcpy(String_post,(void*)&curHTTP.data[10], len-10);
				String_post[len-10]='\0';
				strcpy(profile.ServerURL, String_post);
			}
			else if(memcmppgm2ram(curHTTP.data,(ROM void*)"SNTP", 4) == 0)
			{
				memcpy(String_post,(void*)&curHTTP.data[5], len-5);
				String_post[len-5]='\0';
				profile.SNTPEnable=1;
			}
			else if(memcmppgm2ram(curHTTP.data,(ROM void*)"ENABLEAPP", 9) == 0)
			{
				memcpy(String_post,(void*)&curHTTP.data[10], len-10);
				String_post[len-10]='\0';
				profile.AppEnable=1;
			}
		}
		ProfileSave();
	}
	
	if(!memcmppgm2ram(filename, "protect/network.cgi",19))
	{
		while(curHTTP.byteCount)
		{
			// Check for a complete variable
			len = TCPFind(sktHTTP, '&', 0, FALSE);
			if(len == 0xffff)
			{
				// Check if is the last post, otherwise continue in the loop
				if( TCPIsGetReady(sktHTTP) == curHTTP.byteCount)
					len = curHTTP.byteCount - 1;
				else 
				{	
					return HTTP_IO_NEED_DATA; // No last post, we need more data
				}
			}
			if(len > HTTP_MAX_DATA_LEN - 2)
			{
				// Make sure we don't overflow
				curHTTP.byteCount -= TCPGetArray(sktHTTP, (BYTE*)String_post, len+1);
				continue;
			}

			len = TCPGetArray(sktHTTP,curHTTP.data, len+1);

			curHTTP.byteCount -= len;
			curHTTP.data[len] = '\0';
			HTTPURLDecode(curHTTP.data);
			
			if(memcmppgm2ram(curHTTP.data,(ROM void*)"DHCP", 4) == 0)
			{
				memcpy(String_post,(void*)&curHTTP.data[5], len-5);	
				String_post[len-5] = '\0';
				if(String_post[0] == '1')
				    profile.DHCPEnabled = 1;
				else
					profile.DHCPEnabled = 0;
			}
			else if(memcmppgm2ram(curHTTP.data,(ROM void*)"IP", 2) == 0)
			{
				memcpy(String_post,(void*)&curHTTP.data[3], len-3);	
				String_post[len-3] = '\0';
				strcpy(profile.IPAddress, String_post);
			}
			else if(memcmppgm2ram(curHTTP.data,(ROM void*)"MASK", 4) == 0)
			{
				memcpy(String_post,(void*)&curHTTP.data[5], len-5);	
				String_post[len-5] = '\0';
				strcpy(profile.SubnetMask, String_post);
			}
			else if(memcmppgm2ram(curHTTP.data,(ROM void*)"GATEWAY", 7) == 0)
			{
				memcpy(String_post,(void*)&curHTTP.data[8], len-8);	
				String_post[len-8] = '\0';
				strcpy(profile.Gateway, String_post);
			}
			else if(memcmppgm2ram(curHTTP.data,(ROM void*)"DNS1", 4) == 0)
			{
				memcpy(String_post,(void*)&curHTTP.data[5], len-5);	
				String_post[len-5] = '\0';
				strcpy(profile.DNS1, String_post);
			}
			else if(memcmppgm2ram(curHTTP.data,(ROM void*)"DNS2", 4) == 0)
			{
				memcpy(String_post,(void*)&curHTTP.data[5], len-5);	
				String_post[len-5] = '\0';
				strcpy(profile.DNS2, String_post);
			}
		}
		NETSave();
		ProfileSave();
	}
	
	/*if(!memcmppgm2ram(filename, "protect/wireless.cgi",20))
	{
		while(curHTTP.byteCount)
		{
			// Check for a complete variable
			len = TCPFind(sktHTTP, '&', 0, FALSE);
			if(len == 0xffff)
			{
				// Check if is the last post, otherwise continue in the loop
				if( TCPIsGetReady(sktHTTP) == curHTTP.byteCount)
					len = curHTTP.byteCount - 1;
				else 
				{	
					return HTTP_IO_NEED_DATA; // No last post, we need more data
				}
			}
			if(len > HTTP_MAX_DATA_LEN - 2)
			{
				// Make sure we don't overflow
				curHTTP.byteCount -= TCPGetArray(sktHTTP, (BYTE*)String_post, len+1);
				continue;
			}

			len = TCPGetArray(sktHTTP,curHTTP.data, len+1);

			curHTTP.byteCount -= len;
			curHTTP.data[len] = '\0';
			HTTPURLDecode(curHTTP.data);
			
			if(memcmppgm2ram(curHTTP.data,(ROM void*)"NETTYPE", 7) == 0)
			{
				memcpy(String_post,(void*)&curHTTP.data[8], len-8);	
				String_post[len-8] = '\0';
				profile.NetType = String_post[0] == '0' ? NETTYPE_ADHOC : NETTYPE_INFRA;
			}
			else if(memcmppgm2ram(curHTTP.data,(ROM void*)"SSID", 4) == 0)
			{
				memcpy(String_post,(void*)&curHTTP.data[5], len-5);	
				String_post[len-5] = '\0';
				strcpy(profile.SSID, String_post);
			}
			else if(memcmppgm2ram(curHTTP.data,(ROM void*)"TYPE", 4) == 0)
			{
				memcpy(String_post,(void*)&curHTTP.data[5], len-5);	
				String_post[len-5] = '\0';
				if(strcmp(String_post, "OPEN")==0)
				{
					profile.SecurityType = WF_SECURITY_OPEN;
				}
				else if(strcmp(String_post, "WEP40")==0)
				{
					profile.SecurityType = WF_SECURITY_WEP_40;
				}
				else if(strcmp(String_post, "WEP104")==0)
				{
					profile.SecurityType = WF_SECURITY_WEP_104;
				}
				else if(strcmp(String_post, "WPA2")==0)
				{
					profile.SecurityType = WF_SECURITY_WPA2_WITH_PASS_PHRASE;
				}
				else if(strcmp(String_post, "WPA")==0)
				{
					profile.SecurityType = WF_SECURITY_WPA_WITH_PASS_PHRASE;
				}
			}
			else if(memcmppgm2ram(curHTTP.data,(ROM void*)"KEY", 3) == 0)
			{
				memcpy(String_post,(void*)&curHTTP.data[4], len-4);	
				String_post[len-4] = '\0';
				strcpy(profile.SecurityKey, String_post);
			}
		}
		WifiSave();
		ProfileSave();
	}*/
	return HTTP_IO_DONE;
}
#endif

#endif
