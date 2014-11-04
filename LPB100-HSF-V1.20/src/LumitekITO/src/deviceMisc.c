/*
******************************
*Company:Lumitek
*Data:2014-10-07
*Author:Meiyusong
******************************
*/


#include "../inc/lumitekConfig.h"

#ifdef CONFIG_LUMITEK_DEVICE
#include <hsf.h>
#include <string.h>
#include <stdio.h>


#include "../inc/itoCommon.h"
#include "../inc/deviceTime.h"


//calibrate time interval
#define MAX_CALIBRATE_TIME_INTERVAL			20000	//3600*2*1000
#define MAX_FAILD_CALIBRATE_TIME_INTERVAL	10000	//5*60*1000
#define FROM_1900_TO_1970_SEC				2208988800


#define TIMER_GETUTC						2

static hftimer_handle_t getUtcTimer = NULL;



static void USER_FUNC setRtcTime(time_t time)
{
	struct timeval	iots_tm;

	
	iots_tm.tv_sec = (long)time;	
	iots_tm.tv_usec = 0;
	settimeofday(&iots_tm, NULL);
}


static void USER_FUNC getUtcTimeCallback( hftimer_handle_t htimer )
{
	BOOL getSucc = FALSE;
	U32 utcTime;
	U32 timerPeriod;


	
	if(getUtcTimeFromNetwork(&utcTime))
	{
		if(utcTime > FROM_1900_TO_1970_SEC)
		{
			utcTime -= FROM_1900_TO_1970_SEC;
			setRtcTime(utcTime);
			getSucc = TRUE;
		}
	}
	if(getSucc)
	{
		timerPeriod = MAX_CALIBRATE_TIME_INTERVAL
	}
	else
	{
		timerPeriod = MAX_FAILD_CALIBRATE_TIME_INTERVAL
	}
	hftimer_change_period(htimer, timerPeriod);
	hftimer_start(htimer);
}



void USER_FUNC creatGetUtcTimer(void)
{
	getUtcTimer = hftimer_create("Get_UTC_Time",10000, false, TIMER_GETUTC, getUtcTimeCallback, 0);
	if(getUtcTimer == NULL)
	{
		lumi_error("creatGetUtcTimer Faild\n");
		return;
	}
	hftimer_start(getUtcTimer);
}





BOOL USER_FUNC closeNtpMode(void)
{
	char *words[3]={NULL};
	char rsp[32]={0};
	char nrpMode[8]={0};
	BOOL ret = FALSE;
	

	hfat_send_cmd("AT+NTPEN\r\n",sizeof("AT+NTPEN\r\n"),rsp,32);
	if(hfat_get_words(rsp,words, 2)>0)
	{
		if((rsp[0]=='+')&&(rsp[1]=='o')&&(rsp[2]=='k'))
		{
			strcpy(nrpMode,words[1]);		
			if(strncmp(nrpMode, "off") != 0)
			{
				hfat_send_cmd("AT+NTPEN=off\r\n",sizeof("AT+NTPEN=off\r\n"),rsp,32);
				if(((rsp[0]=='+')&&(rsp[1]=='o')&&(rsp[2]=='k')))
				{
					ret = TRUE;
				}
			}
		}
	}
	return ret;
}


#endif

