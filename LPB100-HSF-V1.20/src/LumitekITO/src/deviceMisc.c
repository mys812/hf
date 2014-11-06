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
#include "../inc/asyncMessage.h"
#include "../inc/messageDispose.h"
#include "../inc/localSocketUdp.h"
#include "../inc/deviceGpio.h"
#include "../inc/serverSocketTcp.h"
#include "../inc/socketSendList.h"
#include "../inc/deviceTime.h"



//calibrate time interval
#define MAX_CALIBRATE_TIME_INTERVAL			20000	//3600*2*1000
#define MAX_FAILD_CALIBRATE_TIME_INTERVAL	10000	//5*60*1000
#define FROM_1900_TO_1970_SEC				2208988800




static hftimer_handle_t getUtcTimer = NULL;
static hftimer_handle_t getHeartBeatTimer = NULL;




static void USER_FUNC setRtcTime(time_t time)
{
	struct timeval	iots_tm;

	
	iots_tm.tv_sec = (long)time;	
	iots_tm.tv_usec = 0;
	settimeofday(&iots_tm, NULL);
}


void USER_FUNC sendGetUtcTimeMsg(void)
{
	insertLocalMsgToList(MSG_LOCAL_EVENT, NULL, 0, MSG_CMD_LOCAL_GET_UTC_TIME);
}


static void USER_FUNC getUtcTimeCallback( hftimer_handle_t htimer )
{
	sendGetUtcTimeMsg();
}



static void USER_FUNC createGetUtcTimer(void)
{
	getUtcTimer = hftimer_create("Get_UTC_Time",10000, false, GET_UTC_TIMER_ID, getUtcTimeCallback, 0);
	if(getUtcTimer == NULL)
	{
		lumi_error("creatGetUtcTimer Faild\n");
		return;
	}
	hftimer_start(getUtcTimer);
}



void USER_FUNC getUtcTimeByMessage(void)
{
	BOOL getSucc = FALSE;
	U32 utcTime;
	U32 timerPeriod = MAX_FAILD_CALIBRATE_TIME_INTERVAL;


	if(getUtcTimer == NULL)
	{
		createGetUtcTimer();
	}
	else
	{
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
			timerPeriod = MAX_CALIBRATE_TIME_INTERVAL;
		}
		hftimer_change_period(getUtcTimer, timerPeriod);
		hftimer_start(getUtcTimer);
	}
}



static void USER_FUNC heartBeatTimerCallback( hftimer_handle_t htimer )
{
	lumi_debug("heartBeatTimerCallback \n");
	insertLocalMsgToList(MSG_LOCAL_EVENT, NULL, 0, MSG_CMD_HEART_BEAT);
	hftimer_change_period(htimer, 300000); //30S
	hftimer_start(htimer);
}



void USER_FUNC changeHeartBeatTimerPeriod(U16 interval)
{
	S32 period;

	period = interval*1000; //S to ms
	hftimer_change_period(getHeartBeatTimer, period); //30S
	hftimer_start(getHeartBeatTimer);
}


void USER_FUNC createHeartBeatTimer(void)
{
	if(getHeartBeatTimer == NULL)
	{
		getHeartBeatTimer = hftimer_create("HeartBeat Timer",1000, false, HEARTBENT_TIMER_ID, heartBeatTimerCallback, 0);
		hftimer_start(getHeartBeatTimer);
	}
	else
	{
		changeHeartBeatTimerPeriod(1);
	}
}


void USER_FUNC closeNtpMode(void)
{
	char *words[3]={NULL};
	char rsp[32]={0};
	char nrpMode[8]={0};
	

	hfat_send_cmd("AT+NTPEN\r\n",sizeof("AT+NTPEN\r\n"),rsp,32);
	if(hfat_get_words(rsp,words, 2)>0)
	{
		if((rsp[0]=='+')&&(rsp[1]=='o')&&(rsp[2]=='k'))
		{
			strcpy(nrpMode,words[1]);
			lumi_debug("AT+NTPEN===>%s\n", nrpMode);
			if(strncmp(nrpMode, "off", 3) != 0)
			{
				hfat_send_cmd("AT+NTPEN=off\r\n",sizeof("AT+NTPEN=off\r\n"),rsp,32);
				if(((rsp[0]=='+')&&(rsp[1]=='o')&&(rsp[2]=='k')))
				{
					hfsys_reset();
				}
			}
		}
	}
}


#endif

