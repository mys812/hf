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
#define MAX_CALIBRATE_TIME_INTERVAL			3600000U	//3600*2*1000
#define MAX_FAILD_CALIBRATE_TIME_INTERVAL	10000	//5*60*1000
#define FROM_1900_TO_1970_SEC				2208988800U




static hftimer_handle_t getUtcTimer = NULL;
static hftimer_handle_t getHeartBeatTimer = NULL;
static hftimer_handle_t checkSmarkLinkTimer = NULL;




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


static void USER_FUNC getUtcTimerCallback( hftimer_handle_t htimer )
{
	sendGetUtcTimeMsg();
}



static void USER_FUNC createGetUtcTimer(void)
{
	S32 period = 10000;
	
	getUtcTimer = hftimer_create("Get_UTC_Time",period, false, GET_UTC_TIMER_ID, getUtcTimerCallback, 0);
	if(getUtcTimer == NULL)
	{
		lumi_error("creatGetUtcTimer Faild\n");
		return;
	}
	//hftimer_start(getUtcTimer);
	hftimer_change_period(getUtcTimer, period);
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
		//hftimer_start(getUtcTimer);
	}
}



static void USER_FUNC heartBeatTimerCallback( hftimer_handle_t htimer )
{
	//lumi_debug("heartBeatTimerCallback \n");
	insertLocalMsgToList(MSG_LOCAL_EVENT, NULL, 0, MSG_CMD_HEART_BEAT);
	hftimer_change_period(htimer, 30000); //30S
	//hftimer_start(htimer);
}



void USER_FUNC changeHeartBeatTimerPeriod(U16 interval)
{
	S32 period;

	period = interval*1000; //S to ms
	hftimer_change_period(getHeartBeatTimer, period); //30S
	//hftimer_start(getHeartBeatTimer);
}


void USER_FUNC createHeartBeatTimer(void)
{
	if(getHeartBeatTimer == NULL)
	{
		S32 period = 1000;

		
		getHeartBeatTimer = hftimer_create("HeartBeat Timer",period, false, HEARTBEAT_TIMER_ID, heartBeatTimerCallback, 0);
		//hftimer_start(getHeartBeatTimer);
		hftimer_change_period(getHeartBeatTimer, period);
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



//About SmarkLink

static void USER_FUNC smartlinkTimerCallback( hftimer_handle_t htimer )
{
#ifdef LPB100_DEVLOPMENT_BOARD
	switchLightStatus();
#elif defined(DEEVICE_LUMITEK_P1)
	switchBuzzerStatus();
#else
	//do nothing
#endif
}



void USER_FUNC deviceEnterSmartLink(void)
{
	hftimer_handle_t smartlinkTimer;
	S32 period = 800;


	smartlinkTimer = hftimer_create("SMARTLINK_TIMER", period, true, SMARTLINK_TIMER_ID, smartlinkTimerCallback, 0);
	//hftimer_start(smartlinkTimer);
	hftimer_change_period(smartlinkTimer, period);
}


void USER_FUNC clearDeviceSSID(void)
{
	char rsp[64]= {0};

	hfat_send_cmd("AT+WSSSID=NULL\r\n",sizeof("AT+WSSSID=NULL\r\n"),rsp,64);
	msleep(100);

}



void USER_FUNC sendSmartLinkCmd(void)
{
	char rsp[64]= {0};

	hfat_send_cmd("AT+SMTLK\r\n",sizeof("AT+SMTLK\r\n"),rsp,64);

}



static void USER_FUNC checkSmartLinkTimerCallback( hftimer_handle_t htimer )
{
	lumi_debug("checkSmartLinkTimerCallback \n");
	hftimer_delete(htimer);
	checkSmarkLinkTimer = NULL;
	sendSmartLinkCmd();
}



static BOOL USER_FUNC checkWifiStaConfig(void)
{
	char *words[3]={NULL};
	char rsp[32]={0};
	char ssidStr[20]={0};
	

	memset(ssidStr, 0, sizeof(ssidStr));
	hfat_send_cmd("AT+WSSSID\r\n",sizeof("AT+WSSSID\r\n"),rsp,32);
	if(hfat_get_words(rsp,words, 2)>0)
	{
		if((rsp[0]=='+')&&(rsp[1]=='o')&&(rsp[2]=='k'))
		{
			memcpy(ssidStr,words[1], 18);
			lumi_debug("AT+WSSSID===>%s\n", ssidStr);
			if(strlen(ssidStr) > 0)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}



void USER_FUNC checkNeedEnterSmartLink(void)
{
	if(!checkWifiStaConfig())
	{
		sendSmartLinkCmd();
	}
	else
	{
		S32 period = 30000; //30S

		
		if(checkSmarkLinkTimer == NULL)
		{
			checkSmarkLinkTimer = hftimer_create("check SMARTLINK Timer", period, false, CHECK_SMARTLINK_TIMER_ID, checkSmartLinkTimerCallback, 0);
		}
		hftimer_change_period(checkSmarkLinkTimer, period);

		//hftimer_start(checkSmarkLinkTimer);
	}
}


void USER_FUNC cancelCheckSmartLinkTimer(void)
{
	if(checkSmarkLinkTimer != NULL)
	{
		//hftimer_stop(checkSmarkLinkTimer);
		hftimer_delete(checkSmarkLinkTimer);
		checkSmarkLinkTimer = NULL;
	}
}

#endif

