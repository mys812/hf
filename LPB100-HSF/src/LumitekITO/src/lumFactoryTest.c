/*
******************************
*Company:Lumlink
*Data:2015-03-25
*Author:Meiyusong
******************************
*/

#include "../inc/lumitekConfig.h"

#ifdef CONFIG_LUMITEK_DEVICE

#ifdef LUM_FACTORY_TEST_SUPPORT

#include <hsf.h>
#include <string.h>
#include <stdio.h>

#include "../inc/itoCommon.h"
#include "../inc/deviceMisc.h"
#include "../inc/lumFactoryTest.h"


static FACORY_TEST_DATA g_factoryTestData;


static void USER_FUNC lum_setDefaultApData(void)
{
	
	char *words[3]={NULL};
	char rsp[64]={0};
	S8 sendData[50];
	

	memset(rsp, 0, sizeof(rsp));
	memset(sendData, 0, sizeof(sendData));

	hfat_send_cmd("AT+WSSSID\r\n", sizeof("AT+WSSSID\r\n"), rsp, 32);
	if(hfat_get_words(rsp,words, 2)>0)
	{
		if((rsp[0]=='+')&&(rsp[1]=='o')&&(rsp[2]=='k'))
		{
			lumi_debug("AT+WSSSID===>%s\n", words[1]);
			if(strncmp(words[1], FACTORY_TEST_SSID, strlen(FACTORY_TEST_SSID)) == 0)
			{
				return;
			}
		}
	}	
	
	sprintf(sendData, "AT+WSSSID=%s\r\n", FACTORY_TEST_SSID);	
	hfat_send_cmd(sendData, strlen(sendData), rsp, 32);
	if(((rsp[0]=='+')&&(rsp[1]=='o')&&(rsp[2]=='k')))
	{
		memset(rsp, 0, sizeof(rsp));
		memset(sendData, 0, sizeof(sendData));

		sprintf(sendData, "AT+WSKEY=WPA2PSK,AES,%s\r\n", FACTORY_TEST_PASSWORD);
		hfat_send_cmd(sendData, strlen(sendData), rsp, 32);
		if(((rsp[0]=='+')&&(rsp[1]=='o')&&(rsp[2]=='k')))
		{
			hfsys_reset();
		}
	}
}


void USER_FUNC lum_factoryTestDhcpSucc(void)
{
	lum_showFactoryTestApConnect();
	g_factoryTestData.wifiConnect = TRUE;
}


static BOOL USER_FUNC lum_checkTestSucc(void)
{
	if(g_factoryTestData.bInFactoryTest
		&& g_factoryTestData.wifiConnect
#ifdef DEVICE_KEY_SUPPORT
		&& g_factoryTestData.keyPressTimes >= MAX_KEY_PRESS_TIMES
#endif
#ifdef EXTRA_SWITCH_SUPPORT
		&& g_factoryTestData.exteaKeyPressTimes >= MAX_KEY_PRESS_TIMES
#endif
#ifdef TWO_SWITCH_SUPPORT
		&& g_factoryTestData.extraKey2PressTimes >= MAX_KEY_PRESS_TIMES
#endif
	)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


static BOOL USER_FUNC lum_getFactoryTestFlag(void)
{
	U32 flag;


	hfuflash_read(FACTORY_TEST_DATA_OFFSET, (S8*)(&flag), 4);
	if(flag == FACTORY_TEST_FLAG)
	{
		return FALSE;
	}
	return TRUE;
}


void USER_FUNC lum_setFactoryTestFlag(BOOL bClear)
{
	U32 flag;


	if(bClear)
	{
		flag = 0;
	}
	else
	{
		flag = FACTORY_TEST_FLAG;
	}
	hfuflash_erase_page(FACTORY_TEST_DATA_OFFSET, (FACTORY_TEST_DATA_TOTAL_SIZE/HFFLASH_PAGE_SIZE));
	hfuflash_write(FACTORY_TEST_DATA_OFFSET, (S8*)(&flag), 4);
	if(bClear)
	{
		msleep(200);
		hfsys_reset();
	}
}


void USER_FUNC lum_enterFactoryTestThread(void *arg)
{
	g_factoryTestData.bInFactoryTest = TRUE;
	lum_setDefaultApData();
	lum_showEnterFactoryTest();


	while(1)
	{

		if(lum_checkTestSucc())
		{
			lum_showFactoryTestSucc();
			lum_setFactoryTestFlag(FALSE);
			while(1)
			{
				msleep(6000000);
			}
		}
		else
		{
			msleep(100);
		}
	}
}


void USER_FUNC lum_addFactoryKeyPressTimes(BOOL key, BOOL extraKey, BOOL extraKey2)
{
	if(!g_factoryTestData.bInFactoryTest)
	{
		return;
	}
	else
	{
#ifdef DEVICE_KEY_SUPPORT
		if(key)
		{
			g_factoryTestData.keyPressTimes++;
			if(g_factoryTestData.keyPressTimes >= 200)
			{
				g_factoryTestData.keyPressTimes = MAX_KEY_PRESS_TIMES;
			}
		}
#endif

#ifdef EXTRA_SWITCH_SUPPORT
		if(extraKey)
		{
			g_factoryTestData.exteaKeyPressTimes++;
			if(g_factoryTestData.exteaKeyPressTimes >= 200)
			{
				g_factoryTestData.exteaKeyPressTimes = MAX_KEY_PRESS_TIMES;
			}
		}
#endif

#ifdef TWO_SWITCH_SUPPORT
		if(extraKey2)
		{
			g_factoryTestData.extraKey2PressTimes++;
			if(g_factoryTestData.extraKey2PressTimes >= 200)
			{
				g_factoryTestData.extraKey2PressTimes = MAX_KEY_PRESS_TIMES;
			}
		}
#endif
	}
}


BOOL USER_FUNC lum_checkNeedFactoryTest(void)
{
	BOOL ret;


	memset(&g_factoryTestData, 0, sizeof(FACORY_TEST_DATA));
	ret = lum_getFactoryTestFlag();
	return ret;
}


BOOL USER_FUNC lum_bEnterFactoryTest(void)
{
	return g_factoryTestData.bInFactoryTest;
}

#endif /* LUM_FACTORY_TEST_SUPPORT */
#endif


