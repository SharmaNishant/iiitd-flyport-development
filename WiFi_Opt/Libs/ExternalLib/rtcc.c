/* **************************************************************************																					
 *                                OpenPicus                 www.openpicus.com
 *                                                            italian concept
 * 
 *            openSource wireless Platform for sensors and Internet of Things	
 * **************************************************************************
 *  FileName:        rtcc.c
 *  Dependencies:    Microchip configs files
 *  Module:          FlyPort WI-FI
 *  Compiler:        Microchip C30 v3.12 or higher
 *
 *  Author               Rev.    Date              Comment
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  Stefano Saccucci     1.0     06/13/2011		   First release  (core team)
 *  
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
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

#include "rtcc.h"

static WORD buffer = 0;
BOOL alarmflag = FALSE;
BOOL alarmflag_opt = FALSE;
int arpt;

  /*-------------------------------------------------------------------------------------
  |	Function: 		void __attribute((interrupt,auto_psv)) _RTCCInterrupt()		        |
  | Description: 	The function executed anytime the alarm runs. Put here the code 	|
  |					you want to be executed.											|
  | Returns:		-														   			|
  | Parameters:		-													                |
  |																						|
  | IMPORTANT: 		It's not possible to call any TCP function from here, in case		|
  |					set a flag to check in the main program.							|
  -------------------------------------------------------------------------------------*/

void __attribute((interrupt,auto_psv)) _RTCCInterrupt()
{
	//	AUTOMATIC ALARM HANDLING CODE: DON'T TOUCH IT!
	while (RCFGCALbits.RTCSYNC == 1);
	arpt = ALCFGRPTbits.ARPT;
	if (arpt)
		RTCCRunAlarm(1);
	IFS3bits.RTCIF = 0;
	//	END OF ALARM HANDLIG CODE
	
	alarmflag = TRUE;  //pre-configured flag to use in the main routine with "extern BOOL alarmflag"
	alarmflag_opt = TRUE;
	
	//	ALARM CUSTOM CODE: PUT HERE YOUR CUSTOM CODE TO EXECUTE DURING THE ALARM,
	//	OR CATCH THE alarmflag VAR IN THE FLYPORT TASK
	
}

  /*-------------------------------------------------------------------------------------
  |	Function: 		RTCCWrite(t_RTCC* rtcc)		 		   	            		        |
  | Description: 	This function writes date and time in RTCC registers and starts it. |
  | Returns:		-														   			|
  | Parameters:		t_RTCC* rtcc										                |
  -------------------------------------------------------------------------------------*/

void RTCCWrite(t_RTCC* rtcc)
{
	__builtin_write_OSCCONL(OSCCON | 0x02); 
	asm volatile("disi #5");
	__builtin_write_RTCWEN();
	
	ALCFGRPTbits.ALRMEN = 0;	
	RCFGCALbits.RTCEN = 0;
	
	RCFGCALbits.RTCPTR = 3;
	
	RTCVAL = DtoB(rtcc->year);
	RTCVAL = BtoW(DtoB(rtcc->month),DtoB(rtcc->day));
	RTCVAL = BtoW(DtoB(rtcc->dweek),DtoB(rtcc->hour));
	RTCVAL = BtoW(DtoB(rtcc->min),DtoB(rtcc->sec));
	
	RCFGCALbits.RTCEN = 1;
	
	RCFGCALbits.RTCWREN = 0;
}
  
  /*-------------------------------------------------------------------------------------
  |	Function: 		RTCCRead(t_RTCC* rtcc)						     		   			|
  | Description: 	This function reads the RTC registers and saves the values  		|
  |					in the t_RTCC type struct.											|
  | Returns:		-														   			|
  | Parameters:		t_RTCC* rtcc														|
  -------------------------------------------------------------------------------------*/

void RTCCRead(t_RTCC* rtcc)
{	
	RCFGCALbits.RTCPTR = 3;
	
	rtcc->year = BtoD(RTCVAL);
	
	buffer = RTCVAL;
	rtcc->month = BtoD(buffer>>8);
	rtcc->day = BtoD(buffer);

	buffer = RTCVAL;
	rtcc->dweek = BtoD(buffer>>8);
	rtcc->hour = BtoD(buffer);

	buffer = RTCVAL;
	rtcc->min = BtoD(buffer>>8);
	rtcc->sec = BtoD(buffer);
}

  /*----------------------------------------------------------------------------------------
  |	Function: 		RTCCSetAlarm(t_RTCC* rtcc, int repeats, BYTE mask)					   |
  | Description: 	With this function you can configure the RTCC Alarm.	   			   |
  | Returns:		-														   			   |
  | Parameters:		t_RTCC* rtcc		 												   |
  |																						   |
  |				 	int repeats - number of repeats	or								   	   |  
  |				 	  	      	  -	REPEAT_NO											   |
  |				 	  	      	  -	REPEAT_INFINITE										   | 
  |																						   |
  |					BYTE mask - use these definitions:									   |
  |							    - EVERY_HALF_SEC								   		   |
  |							    - EVERY_SEC									   			   |
  |							    - EVERY_TEN_SEC										  	   |
  |							    - EVERY_MIN												   |
  |							    - EVERY_TEN_MIN											   |
  |							    - EVERY_HOUR											   |
  |							    - EVERY_DAY												   |
  |							    - EVERY_WEEK											   |
  |							    - EVERY_MONTH											   |
  |							    - EVERY_YEAR											   |
  ----------------------------------------------------------------------------------------*/

void RTCCSetAlarm(t_RTCC* rtcc, int repeats, BYTE mask)
{	
	BYTE chime, crepeat;
	if (repeats == 0)
	{
		chime = 0;
		crepeat = 0;
	}
	else if (repeats > 255)
	{
		chime = 1;
		crepeat = 255;
	}
	else 
	{
		chime = 0;
		crepeat = (BYTE)(repeats - 1);
	}
	IEC3bits.RTCIE = 0;
	
	//	Synch operations and critical section to avoid context switch
	vTaskSuspendAll();
	while (RCFGCALbits.RTCSYNC == 1);
	
	//	Enabling writing on RTCC registers
	asm volatile("disi #5");
	__builtin_write_RTCWEN();

	
	ALCFGRPTbits.ALRMEN = 0;

	ALCFGRPTbits.ALRMPTR = 2;
	ALRMVAL = BtoW(DtoB(rtcc->month),DtoB(rtcc->day));
	ALRMVAL = BtoW(DtoB(rtcc->dweek),DtoB(rtcc->hour));
	ALRMVAL = BtoW(DtoB(rtcc->min),DtoB(rtcc->sec));
	
	
	ALCFGRPTbits.CHIME = chime; 
	ALCFGRPTbits.ARPT = crepeat;
	ALCFGRPTbits.AMASK = mask;
	
	RCFGCALbits.RTCWREN = 0;
	IPC15bits.RTCIP = 5; 
	IEC3bits.RTCIE = 1; 
	IFS3bits.RTCIF = 0;
	xTaskResumeAll();
}

  /*-------------------------------------------------------------------------------------
  |	Function: 		RTCCRunAlarm(BYTE run)                  		 		   			|
  | Description: 	This function activates or deactivates the alarm.   	   			|
  | Returns:		-														   			|
  | Parameters:		BYTE run - 0 Off										 			|
  |							   1 On														|
  -------------------------------------------------------------------------------------*/

void RTCCRunAlarm(BYTE run) 
{	
	if (run != ALCFGRPTbits.ALRMEN )
	{
		while (RCFGCALbits.RTCSYNC == 1);
		asm volatile("disi #5");
		__builtin_write_RTCWEN();
		ALCFGRPTbits.ALRMEN = run;
		RCFGCALbits.RTCWREN = 0;
	}
}
