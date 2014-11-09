/*
******************************
*Company:Lumitek
*Data:2014-11-09
*Author:Meiyusong
******************************
*/

#include "../inc/lumitekConfig.h"

#ifdef CONFIG_LUMITEK_DEVICE
#include <hsf.h>
#include <string.h>
#include <stdio.h>

#include "../inc/itoCommon.h"


static BOOL g_has_get_ip;




void USER_FUNC setUpgradeType(S8* url)
{
	setSoftwareUpgradeUrl(url);
	msleep(300);
	hfsys_reset();
}



static void USER_FUNC deviceEnterSwUpgrade(void)
{
	SW_UPGRADE_DATA* pUpgradeData;
	S8 rsp[64]={0};
	S8 sendCmd[120];
	S8 cmdlen;


	
	pUpgradeData = getSoftwareUpgradeData();
	clearSoftwareUpgradeFlag();
	
	memset(sendCmd, 0, sizeof(sendCmd));
	sprintf(sendCmd, "AT+UPURL=%s,", pUpgradeData->urlData);
	cmdlen = strlen(sendCmd);

	lumi_debug("sendCmd = %s\n", sendCmd);
	hfat_send_cmd(sendCmd, cmdlen, rsp, 64);
	lumi_debug("rsp = %s\n", rsp);
	msleep(1000);
	if(((rsp[0]=='+')&&(rsp[1]=='o')&&(rsp[2]=='k')))
	{
		hfsys_reset();
	}
}



static int updgradeEventCallback( uint32_t event_id,void * param)
{
	if(HFE_DHCP_OK == event_id)
	{
		g_has_get_ip = TRUE;
		lumi_debug("upgrade has got IP\n");
	}
	return 0;
}


static void USER_FUNC deviceUpgradeThread(void)
{
	while(1)
	{
		if(g_has_get_ip)
		{
			deviceEnterSwUpgrade();
		}
		
		msleep(100);
	}
}



void USER_FUNC enterUpgradeThread(void)
{
	g_has_get_ip = FALSE;
	if(hfsys_register_system_event((hfsys_event_callback_t)updgradeEventCallback)!= HF_SUCCESS)
	{
		lumi_debug("register upgrade event fail\n");
	}

	if(hfthread_create((PHFTHREAD_START_ROUTINE)deviceUpgradeThread, "IOT_Upgrade_C", 512, NULL, HFTHREAD_PRIORITIES_LOW,NULL,NULL)!= HF_SUCCESS)
	{
		lumi_error("Create IOT_Upgrade_C thread failed!\n");
	}
	lumi_debug("go into enterUpgradeThread\n");
}


#endif

