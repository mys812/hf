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
#include "../inc/deviceGpio.h"
#ifdef RN8209C_SUPPORT
#include "../inc/asyncMessage.h"
#include "../inc/rn8209c.h"
#endif
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
#ifdef RN8209C_SUPPORT
		&& g_factoryTestData.calibrate_succ
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



#ifdef RN8209C_SUPPORT
static MeatureEnergyData g_calibrateData;
static RN6209_CALI_STATUS g_calibrateStatus;


static void USER_FUNC lum_sendCaliCmd(void)
{
	insertLocalMsgToList(MSG_LOCAL_EVENT, NULL, 0, MSG_CMD_GET_CALIBRATE_DATA);
}


static void USER_FUNC lum_rn8209CaliTimerCallback( hftimer_handle_t htimer )
{
	lum_sendCaliCmd();
}

static void USER_FUNC lum_startRn8209CaliTimer(U32 timerGap)
{
	static hftimer_handle_t g_rn8209CaliTimer = NULL;

	
	if(g_rn8209CaliTimer == NULL)
	{
		g_rn8209CaliTimer = hftimer_create("Rn8209_Cali", timerGap, false, RN8209_CALI_TIMER_ID, lum_rn8209CaliTimerCallback, 0);
	}
	hftimer_change_period(g_rn8209CaliTimer, timerGap);
}


static void USER_FUNC lum_rn8209CaliInit(void)
{
	memset(&g_calibrateData, 0, sizeof(MeatureEnergyData));
	g_calibrateStatus = CALI_CLOSED;
	lum_startRn8209CaliTimer(RN8209C_CALI_GET_BASE_DATA_GAP);
}


void lum_checkCaliData(U8* caliData)
{
	static BOOL checkAgain = FALSE;
	MeatureEnergyData* pEnergyDataInfo;


	pEnergyDataInfo = (MeatureEnergyData*)caliData;
	if(g_calibrateStatus == CALI_CLOSED)
	{
		memcpy(&g_calibrateData, pEnergyDataInfo, sizeof(MeatureEnergyData));
		g_calibrateStatus = CALI_FIRST;
		setSwitchStatus(SWITCH_OPEN, SWITCH_PIN_1);
		lum_startRn8209CaliTimer(RN8209C_CALI_GET_BASE_DATA_GAP);
	}
	else if(g_calibrateStatus == CALI_FIRST)
	{
		g_calibrateStatus = CALI_CHECK;
		lum_rn8209cCalcCaliKdata(pEnergyDataInfo);
		
		setSwitchStatus(SWITCH_CLOSE, SWITCH_PIN_1);
		hfgpio_fenable_interrupt(HFGPIO_F_KEY);
		checkAgain = FALSE;
		lum_startRn8209CaliTimer(RN8209C_CALI_GET_BASE_DATA_GAP);
	}
	else if(g_calibrateStatus == CALI_CHECK)
	{
		MeatureEnergyData tmpCaliData;


		lum_rn8209cGetIVPData(&tmpCaliData);
		if(!checkAgain)
		{
			if(pEnergyDataInfo->powerP > (U32)(tmpCaliData.powerP *1.5) || pEnergyDataInfo->powerP < (U32)(tmpCaliData.powerP *0.7))
			{
				checkAgain = TRUE;
			}
		}
		else
		{
			lum_rn8209cGetIVPData(&tmpCaliData);
			if(pEnergyDataInfo->powerP >= (U32)(tmpCaliData.powerP*0.96) &&  pEnergyDataInfo->powerP <= (U32)(tmpCaliData.powerP*1.04))
			{
				checkAgain = TRUE;
				g_calibrateStatus = CALI_SUCC;
				lum_saveKData();
				g_factoryTestData.calibrate_succ = TRUE;
				setSwitchStatus(SWITCH_CLOSE, SWITCH_PIN_1);
			}
		}
		if(g_calibrateStatus != CALI_SUCC)
		{
			lum_startRn8209CaliTimer(RN8209C_CALI_GET_BASE_DATA_GAP);
		}
	}
}

#endif


void USER_FUNC lum_enterFactoryTestThread(void *arg)
{
	g_factoryTestData.bInFactoryTest = TRUE;
	lum_setDefaultApData();
	lum_showEnterFactoryTest();
#ifdef RN8209C_SUPPORT
	hfgpio_fdisable_interrupt(HFGPIO_F_KEY);
	setSwitchStatus(SWITCH_CLOSE, SWITCH_PIN_1);
	lum_rn8209CaliInit();
#endif

	while(1)
	{

		if(lum_checkTestSucc())
		{
			lum_showFactoryTestSucc();
			lum_setFactoryTestFlag(FALSE);
			lum_setFactorySmartlink(TRUE);
			while(1)
			{
				msleep(6000000); // one hour 60*1000*1000
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
	S32 up_result;


	up_result = hfupdate_auto_upgrade(0);
	if(up_result < 0)
	{
		return FALSE;
	}
	else
	{
		memset(&g_factoryTestData, 0, sizeof(FACORY_TEST_DATA));
		ret = lum_getFactoryTestFlag();
	}
	return ret;
}


BOOL USER_FUNC lum_bEnterFactoryTest(void)
{
	return g_factoryTestData.bInFactoryTest;
}

#endif /* LUM_FACTORY_TEST_SUPPORT */
#endif


