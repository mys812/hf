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
#include "../inc/localSocketUdp.h"
#include "../inc/serverSocketTcp.h"
#include "../inc/asyncMessage.h"
#include "../inc/serverSocketTcp.h"
#include "../inc/itoCommon.h"
#include "../inc/deviceMisc.h"
#include "../inc/deviceUpgrade.h"
#include "../inc/deviceGpio.h"





#define CLIENT_EVENT_THREAD_DEPTH		256
#define LOCAL_UDP_THREAD_DEPTH			256
#define SERVER_TCP_THREAD_DEPTH			256
#define MESSAGE_THREAD_DEPTH			420



static int systemEventCallback( uint32_t event_id,void * param)
{
	switch(event_id)
	{
	case HFE_WIFI_STA_CONNECTED:
		setDeviceConnectInfo(STA_CONN_BIT, TRUE);
		lumi_debug("wifi sta connected!!\n");
		break;

	case HFE_WIFI_STA_DISCONNECTED:
		setDeviceConnectInfo(STA_CONN_BIT, FALSE);
		setDeviceConnectInfo(DHPC_OK_BIT, FALSE);
		setDeviceConnectInfo(SERVER_CONN_BIT, FALSE);
		setDeviceConnectInfo(NEED_RECONN_BIT, TRUE);
		checkNeedEnterSmartLink();
		lumi_debug("wifi sta disconnected!!\n");
		break;

	case HFE_DHCP_OK:
	{
#ifdef LUMITEK_DEBUG
		U32 *p_ip;
		p_ip = (U32*)param;
		lumi_debug("dhcp ok %08X!\n",*p_ip);
#endif
		setDeviceConnectInfo(DHPC_OK_BIT, TRUE);
		sendGetUtcTimeMsg();
		cancelCheckSmartLinkTimer();
	}
	break;

	case HFE_SMTLK_OK:
		lumi_debug("smtlk ok!\n");
		//return -1;
		break;

	case HFE_CONFIG_RELOAD:
		lumi_debug("reload!\n");
		break;

	default:
		break;

	}
	return 0;
}



static DEVICE_RESET_TYPE USER_FUNC checkResetType(void)
{
	S32	resetReason;
	SW_UPGRADE_DATA* pUpgradeData;
	DEVICE_RESET_TYPE resetType = RESET_FOR_NORMAL;


	globalConfigDataInit();
	resetReason = hfsys_get_reset_reason();
	pUpgradeData = getSoftwareUpgradeData();
	
	if(resetReason&HFSYS_RESET_REASON_SMARTLINK_START)
	{
		resetType = RESET_FOR_SMARTLINK;
	}
	else if(pUpgradeData->upgradeFlag == SOFTWARE_UPGRADE_FLAG)
	{
		resetType = RESET_FOR_UPGRADE;
	}
	lumi_debug("resetType=%d\n", resetType);
	return resetType;
}




void USER_FUNC lumitekITOMain(void)
{
	DEVICE_RESET_TYPE resetType;

	resetType = checkResetType();

	if(resetType == RESET_FOR_SMARTLINK)
	{
		deviceEnterSmartLink();
	}
	else if(resetType == RESET_FOR_UPGRADE)
	{
		keyGpioInit();
		enterUpgradeThread();
	}
	else
	{
		itoParaInit();

		if(hfsys_register_system_event((hfsys_event_callback_t)systemEventCallback)!= HF_SUCCESS)
		{
			lumi_debug("register system event fail\n");
		}

		if(hfthread_create((PHFTHREAD_START_ROUTINE)deviceTimeThread, "IOT_TD_C", CLIENT_EVENT_THREAD_DEPTH,
		                   NULL, HFTHREAD_PRIORITIES_LOW,NULL,NULL)!= HF_SUCCESS)
		{
			lumi_error("Create IOT_TD_C thread failed!\n");
		}

		if(hfthread_create((PHFTHREAD_START_ROUTINE)deviceLocalUdpThread, "IOT_TD_L",LOCAL_UDP_THREAD_DEPTH,
		                   NULL, HFTHREAD_PRIORITIES_MID,NULL,NULL)!= HF_SUCCESS)
		{
			lumi_error("Create IOT_TD_L thread failed!\n");
		}

		if(hfthread_create((PHFTHREAD_START_ROUTINE)deviceServerTcpThread, "IOT_TD_S", SERVER_TCP_THREAD_DEPTH,
		                   NULL, HFTHREAD_PRIORITIES_MID,NULL,NULL)!= HF_SUCCESS)
		{
			lumi_error("Create IOT_TD_S thread failed!\n");
		}
		if(hfthread_create((PHFTHREAD_START_ROUTINE)deviceMessageThread, "IOT_TD_M", MESSAGE_THREAD_DEPTH,
		                   NULL, HFTHREAD_PRIORITIES_LOW,NULL,NULL)!= HF_SUCCESS)
		{
			lumi_error("Create IOT_TD_M thread failed!\n");
		}
	}

}

#endif

