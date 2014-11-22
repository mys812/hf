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
#include "md5.h"


#include "../inc/itoCommon.h"
#include "../inc/deviceUpgrade.h"



void USER_FUNC setUpgradeType(S8* url)
{
	setSoftwareUpgradeUrl(url);
	msleep(300);
	hfsys_reset();
}


static void USER_FUNC deviceEnterSwUpgrade(void)
{
	SW_UPGRADE_DATA* pUpgradeData;
	S8 rspBuf[RSP_BUFFER_LEN];
	S8 sendCmd[120];
	S8 cmdLen;


	
	pUpgradeData = getSoftwareUpgradeData();
	clearSoftwareUpgradeFlag();
	
	cmdLen = strlen(pUpgradeData->urlData);
	if(cmdLen > 100)
	{
		lumi_debug("URL too long cmdLen=%d\n", cmdLen);
		return;
	}
	
	memset(sendCmd, 0, sizeof(sendCmd));
	memset(rspBuf, 0, RSP_BUFFER_LEN);
	sprintf(sendCmd, "AT+UPURL=%s\r\n", pUpgradeData->urlData);
	cmdLen = strlen(sendCmd);
	lumi_debug("sendCmd = %s\n", sendCmd);
	hfat_send_cmd(sendCmd, cmdLen, rspBuf, RSP_BUFFER_LEN);
	lumi_debug("rsp = %s\n", rspBuf);


	memset(sendCmd, 0, sizeof(sendCmd));
	memset(rspBuf, 0, RSP_BUFFER_LEN);
	strcpy(sendCmd, "AT+UPFILE=lumi_config.txt\r\n");
	cmdLen = strlen(sendCmd);
	lumi_debug("sendCmd = %s\n", sendCmd);
	hfat_send_cmd(sendCmd, cmdLen, rspBuf, RSP_BUFFER_LEN);
	lumi_debug("rsp = %s\n", rspBuf);

	
	memset(sendCmd, 0, sizeof(sendCmd));
	memset(rspBuf, 0, RSP_BUFFER_LEN);
	strcpy(sendCmd, "AT+UPST\r\n");
	cmdLen = strlen(sendCmd);
	lumi_debug("sendCmd = %s\n", sendCmd);
	hfat_send_cmd(sendCmd, cmdLen, rspBuf, RSP_BUFFER_LEN);
	lumi_debug("rsp = %s\n", rspBuf);
	msleep(100);

}



static BOOL checkNetworkConnect(void)
{
	S32 pingRet;
	BOOL ret = FALSE;
	
	pingRet = ping(TCP_SERVER_IP);
	lumi_debug("pingRet=%d\n", pingRet);
	if(pingRet == 0)
	{
		ret = TRUE;
	}
	return ret;
}


static void USER_FUNC deviceUpgradeThread(void)
{
	while(1)
	{
		if(checkNetworkConnect())
		{
			deviceEnterSwUpgrade();
			hfsys_reset();
			hfthread_destroy(NULL);
		}
		
		msleep(1000);
	}
}



void USER_FUNC enterUpgradeThread(void)
{
	if(hfthread_create((PHFTHREAD_START_ROUTINE)deviceUpgradeThread, "IOT_Upgrade_C", 512, NULL, HFTHREAD_PRIORITIES_LOW,NULL,NULL)!= HF_SUCCESS)
	{
		lumi_error("Create IOT_Upgrade_C thread failed!\n");
	}
	lumi_debug("go into enterUpgradeThread\n");
}


#endif

