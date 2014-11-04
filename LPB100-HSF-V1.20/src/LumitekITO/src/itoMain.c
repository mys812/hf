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
#include "../inc/localClientEvent.h"
#include "../inc/localSocketUdp.h"
#include "../inc/serverSocketTcp.h"
#include "../inc/asyncMessage.h"
#include "../inc/serverSocketTcp.h"
#include "../inc/itoCommon.h"
#include "../inc/deviceMisc.h"






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
		lumi_debug("wifi sta disconnected!!\n");
		break;

	case HFE_DHCP_OK:
	{
		U32 *p_ip;
		p_ip = (U32*)param;
		setDeviceConnectInfo(DHPC_OK_BIT, TRUE);
		sendGetUtcTimeMsg();
		lumi_debug("dhcp ok %08X!\n",*p_ip);
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



void USER_FUNC lumitekITOMain(void)
{

	if(!checkSmartlinkStatus())
	{
		itoParaInit();

		if(hfsys_register_system_event((hfsys_event_callback_t)systemEventCallback)!= HF_SUCCESS)
		{
			lumi_debug("register system event fail\n");
		}

		if(hfthread_create((PHFTHREAD_START_ROUTINE)deviceClientEventThread, "IOT_TD_C", CLIENT_EVENT_THREAD_DEPTH,
		                   NULL, HFTHREAD_PRIORITIES_LOW,NULL,NULL)!= HF_SUCCESS)
		{
			HF_Debug(DEBUG_ERROR, "Create IOT_TD_C thread failed!\n");
		}

		if(hfthread_create((PHFTHREAD_START_ROUTINE)deviceLocalUdpThread, "IOT_TD_L",LOCAL_UDP_THREAD_DEPTH,
		                   NULL, HFTHREAD_PRIORITIES_MID,NULL,NULL)!= HF_SUCCESS)
		{
			HF_Debug(DEBUG_ERROR, "Create IOT_TD_L thread failed!\n");
		}

		if(hfthread_create((PHFTHREAD_START_ROUTINE)deviceServerTcpThread, "IOT_TD_S", SERVER_TCP_THREAD_DEPTH,
		                   NULL, HFTHREAD_PRIORITIES_MID,NULL,NULL)!= HF_SUCCESS)
		{
			HF_Debug(DEBUG_ERROR, "Create IOT_TD_S thread failed!\n");
		}
		if(hfthread_create((PHFTHREAD_START_ROUTINE)deviceMessageThread, "IOT_TD_M", MESSAGE_THREAD_DEPTH,
		                   NULL, HFTHREAD_PRIORITIES_LOW,NULL,NULL)!= HF_SUCCESS)
		{
			HF_Debug(DEBUG_ERROR, "Create IOT_TD_M thread failed!\n");
		}
	}

}

#endif

