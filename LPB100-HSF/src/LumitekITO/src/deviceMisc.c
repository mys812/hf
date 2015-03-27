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
#include "../inc/deviceMisc.h"



#define SSID_TO_SMARTLINK			"TO_SMARTLINK"



#ifdef DEVICE_NO_KEY
static hftimer_handle_t checkSmarkLinkTimer = NULL;
#endif
#ifdef BUZZER_RING_SUPPORT
static hftimer_handle_t buzzerRingTimer = NULL;
#endif
#ifdef DEVICE_WIFI_LED_SUPPORT
static hftimer_handle_t wifiLedTimer = NULL;
#endif



static void USER_FUNC heartBeatTimerCallback( hftimer_handle_t htimer )
{
	//lumi_debug("heartBeatTimerCallback \n");
	insertLocalMsgToList(MSG_LOCAL_EVENT, NULL, 0, MSG_CMD_HEART_BEAT);
	lum_createHeartBeatTimer(30);
	//hftimer_start(htimer);
}


void USER_FUNC lum_createHeartBeatTimer(U16 interval)
{
	static hftimer_handle_t getHeartBeatTimer = NULL;
	S32 period;


	period = interval*1000;

	if(getHeartBeatTimer == NULL)
	{
		getHeartBeatTimer = hftimer_create("HeartBeat Timer",period, false, HEARTBEAT_TIMER_ID, heartBeatTimerCallback, 0);
	}
	hftimer_change_period(getHeartBeatTimer, period);
}


void USER_FUNC lum_AfterConnectServer(void)
{
	
	lum_createHeartBeatTimer(1); //Start Server heartbeat 1 sencond after connect server
#ifdef RN8209C_SUPPORT
	lum_startReportEnergyUdataTimer(RESEND_ENERGY_DATA_TIMER_GAP);
#endif

}


void USER_FUNC closeNtpMode(void)
{
	char *words[3]={NULL};
	char rsp[64]={0};
	BOOL bAdjust;
	

	memset(rsp, 0, sizeof(rsp));
	hfat_send_cmd("AT+NTPEN\r\n",sizeof("AT+NTPEN\r\n"),rsp,32);
	if(hfat_get_words(rsp,words, 2)>0)
	{
		if((rsp[0]=='+')&&(rsp[1]=='o')&&(rsp[2]=='k'))
		{
			lumi_debug("AT+NTPEN===>%s\n", words[1]);
			if(strncmp(words[1], "off", 3) != 0)
			{
				hfat_send_cmd("AT+NTPEN=off\r\n",sizeof("AT+NTPEN=off\r\n"),rsp,32);
				if(((rsp[0]=='+')&&(rsp[1]=='o')&&(rsp[2]=='k')))
				{
					hfsys_reset();
				}
			}
		}
	}
	
	bAdjust = hftimer_is_adjust();
	lumi_debug("bAdjust = %d\n", bAdjust);
}


BOOL USER_FUNC bRuningStaMode(void)
{
	char *words[3]={NULL};
	char rsp[64]={0};
	BOOL ret = FALSE;
	

	memset(rsp, 0, sizeof(rsp));
	hfat_send_cmd("AT+WMODE\r\n",sizeof("AT+WMODE\r\n"),rsp,32);
	if(hfat_get_words(rsp,words, 2)>0)
	{
		if((rsp[0]=='+')&&(rsp[1]=='o')&&(rsp[2]=='k'))
		{
			lumi_debug("AT+WMODE===>%s\n", words[1]);
			if(strncmp(words[1], "STA", 3) == 0)
			{
				ret = TRUE;
			}
		}
	}
	return ret;
}


//wifi led support
#ifdef DEVICE_WIFI_LED_SUPPORT
static U16 g_wifiLedPeriod;

static void USER_FUNC wifiLedTimerCallback( hftimer_handle_t htimer )
{
	changeWifiLedStatus(FALSE);
	hftimer_change_period(wifiLedTimer, g_wifiLedPeriod);
}

void USER_FUNC setWifiLedStatus(WIFI_LED_STATUS ledStatus)
{
	if(wifiLedTimer == NULL)
	{
		wifiLedTimer = hftimer_create("Wifi LED Timer", 2000, false, WIFI_LED_TIMER_ID, wifiLedTimerCallback, 0);
	}
	if(ledStatus == WIFILED_CLOSE)
	{
		hftimer_stop(wifiLedTimer);
		changeWifiLedStatus(TRUE);
	}
	else if(ledStatus == WIFI_LED_AP_DISCONNECT)
	{
		g_wifiLedPeriod = 1000;
		wifiLedTimerCallback(wifiLedTimer);
	}
	else if(ledStatus == WIFI_LED_TCP_DISCONNECT)
	{
		g_wifiLedPeriod = 400;
		wifiLedTimerCallback(wifiLedTimer);
	}
	else if(ledStatus == WIFI_LED_SMARTLINK)
	{
		g_wifiLedPeriod = 150;
		wifiLedTimerCallback(wifiLedTimer);
	}
}
#endif
//About SmarkLink


#ifdef BUZZER_RING_SUPPORT
const U16 buzzerRingPeriod[] = {250, 250, 0}; //open-->close-->***-->close
const BUZZER_RING_DATA buzzerRingData = {3, 30, 5, buzzerRingPeriod};


static void USER_FUNC deleteBuzzerRingTimer(void)
{
	if(buzzerRingTimer != NULL)
	{
		hftimer_stop(buzzerRingTimer);
		hftimer_delete(buzzerRingTimer);
		buzzerRingTimer = NULL;
	}
}


static void USER_FUNC smartlinkTimerCallback( hftimer_handle_t htimer )
{
	S32 preiod;

	if(buzzerRingTimer == NULL)
	{
		return; //添加任务调度保护
	}
	preiod = getBuzzerRingPeriod(NULL);
	if(preiod > 0)
	{
		if(preiod != buzzerRingData.stopPeriod*1000)
		{
			switchBuzzerStatus();
		}
		hftimer_change_period(htimer, preiod);
	}
	else
	{
		deleteBuzzerRingTimer();
	}
}
#endif



static int systemEventCallbackSmarkLink( uint32_t event_id,void * param)
{
	//need confirm, because forbid delay & sleep within system callback
	if(event_id == HFE_SMTLK_OK)
	{
#ifdef BUZZER_RING_SUPPORT
		deleteBuzzerRingTimer();
		setBuzzerStatus(BUZZER_CLOSE);
		lumi_debug("close buzzer by HFE_SMTLK_OK\n");
#endif

#ifdef DEVICE_WIFI_LED_SUPPORT
		setWifiLedStatus(WIFI_LED_AP_DISCONNECT);
#endif

	}
	return 0;
}


void USER_FUNC deviceEnterSmartLink(void)
{
#ifdef BUZZER_RING_SUPPORT
	S32 period = 300;

	period = getBuzzerRingPeriod(&buzzerRingData);
	switchBuzzerStatus();
	if(buzzerRingTimer == NULL)
	{
		buzzerRingTimer  = hftimer_create("SMARTLINK_TIMER", period, false, SMARTLINK_TIMER_ID, smartlinkTimerCallback, 0);
	}
	//hftimer_start(smartlinkTimer);
	hftimer_change_period(buzzerRingTimer, period);
#endif

#ifdef DEVICE_WIFI_LED_SUPPORT
	setWifiLedStatus(WIFI_LED_SMARTLINK);
#endif
	
	if(hfsys_register_system_event((hfsys_event_callback_t)systemEventCallbackSmarkLink)!= HF_SUCCESS)
	{
		lumi_debug("register system event fail\n");
	}
}


void USER_FUNC sendSmartLinkCmd(void)
{
	char rsp[64]= {0};

	hfat_send_cmd("AT+SMTLK\r\n",sizeof("AT+SMTLK\r\n"),rsp,64);

}



#ifdef DEVICE_NO_KEY
static void USER_FUNC checkSmartLinkTimerCallback( hftimer_handle_t htimer )
{
	lumi_debug("checkSmartLinkTimerCallback \n");
	hftimer_delete(htimer);
	checkSmarkLinkTimer = NULL;
	sendSmartLinkCmd();
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


void USER_FUNC clearDeviceSSIDForSmartLink(void)
{
	char rsp[64]= {0};
	S8 sendCmd[40];


	memset(sendCmd, 0, sizeof(sendCmd));
	sprintf(sendCmd, "AT+WSSSID=%s\r\n", SSID_TO_SMARTLINK);
	hfat_send_cmd(sendCmd, strlen(sendCmd),rsp,64);
	msleep(100);
}


static BOOL USER_FUNC getDeviceSSID(S8* ssidData)
{
	char *words[3]={NULL};
	char rsp[64]={0};
	U8 copyLen;
	

	hfat_send_cmd("AT+WSSSID\r\n",sizeof("AT+WSSSID\r\n"),rsp,32);
	if(hfat_get_words(rsp,words, 2)>0)
	{
		if((rsp[0]=='+')&&(rsp[1]=='o')&&(rsp[2]=='k'))
		{
			copyLen = strlen(words[1]);
			copyLen = (copyLen>18)?18:copyLen;
			
			memcpy(ssidData,words[1], copyLen);
			lumi_debug("AT+WSSSID===>%s\n", ssidData);
			if(strlen(ssidData) > 0)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}



void USER_FUNC checkNeedEnterSmartLink(void)
{
	BOOL hasSSID;
	S8 ssidData[20];


	memset(ssidData, 0, sizeof(ssidData));
	
	hasSSID = getDeviceSSID(ssidData);	
	if(!hasSSID || strcmp(ssidData, SSID_TO_SMARTLINK) == 0)
	{
		sendSmartLinkCmd();
	}
#ifdef DEVICE_NO_KEY
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
#endif	
}


#ifdef LUM_FACTORY_TEST_SUPPORT

#ifdef BUZZER_RING_SUPPORT
static void USER_FUNC lum_buzzerTimerCallback( hftimer_handle_t htimer )
{
	switchBuzzerStatus();
}
#endif


void USER_FUNC lum_showFactoryTestSucc(void)
{
#ifdef BUZZER_RING_SUPPORT
	if(buzzerRingTimer == NULL)
	{
		buzzerRingTimer  = hftimer_create("SMARTLINK_TIMER", 250, true, SMARTLINK_TIMER_ID, lum_buzzerTimerCallback, 0);
	}
	hftimer_change_period(buzzerRingTimer, 250);
#endif

#ifdef DEVICE_WIFI_LED_SUPPORT
	setWifiLedStatus(WIFI_LED_SMARTLINK);
#endif
}


void USER_FUNC lum_showEnterFactoryTest(void)
{
#ifdef BUZZER_RING_SUPPORT
	if(buzzerRingTimer == NULL)
	{
		buzzerRingTimer  = hftimer_create("SMARTLINK_TIMER", 250, true, SMARTLINK_TIMER_ID, lum_buzzerTimerCallback, 0);
	}
	hftimer_change_period(buzzerRingTimer, 1000);
#endif

#ifdef DEVICE_WIFI_LED_SUPPORT
	setWifiLedStatus(WIFI_LED_AP_DISCONNECT);
#endif
}


void USER_FUNC lum_showFactoryTestApConnect(void)
{
#ifdef BUZZER_RING_SUPPORT
	hftimer_stop(buzzerRingTimer);
	setBuzzerStatus(BUZZER_CLOSE);
#endif

#ifdef DEVICE_WIFI_LED_SUPPORT
	setWifiLedStatus(WIFILED_CLOSE);
#endif
}



#endif //LUM_FACTORY_TEST_SUPPORT
#endif

