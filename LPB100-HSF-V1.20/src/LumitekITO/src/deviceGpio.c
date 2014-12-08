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
#include "../inc/deviceMisc.h"
#include "../inc/asyncMessage.h"
#include "../inc/deviceGpio.h"


static BUZZER_STATUS g_buzzer_status = BUZZER_CLOSE;
static BUZZER_RING_INFO buzzerRingInfo;


#ifdef EXTRA_SWITCH_SUPPORT
static BOOL extraSwitchIsHigh;
#endif //EXTRA_SWITCH_SUPPORT




SWITCH_STATUS USER_FUNC getSwitchStatus(void)
{
	if(hfgpio_fpin_is_high(HFGPIO_F_SWITCH))
	{
		return SWITCH_OPEN;
	}
	return SWITCH_CLOSE;
}


void USER_FUNC setSwitchStatus(SWITCH_STATUS action)
{
	SWITCH_STATUS switchStatus = getSwitchStatus();

	
	if(SWITCH_OPEN == action)
	{
		hfgpio_fset_out_high(HFGPIO_F_SWITCH);
	}
	else
	{
		hfgpio_fset_out_low(HFGPIO_F_SWITCH);
	}
	if(action != switchStatus)
	{
		U8 data = (action == SWITCH_OPEN)?1:0;
		
		insertLocalMsgToList(MSG_LOCAL_EVENT, &data, 1, MSG_CMD_REPORT_GPIO_CHANGE);
	}
}


void USER_FUNC changeSwitchStatus(void)
{
	if(getSwitchStatus() == SWITCH_OPEN)
	{
		setSwitchStatus(SWITCH_CLOSE);
	}
	else
	{
		setSwitchStatus(SWITCH_OPEN);
	}
}


static void USER_FUNC initBuzzerStatus(void)
{
	hfgpio_fset_out_low(HFGPIO_F_BUZZER);
	g_buzzer_status = BUZZER_CLOSE;
}


void USER_FUNC setBuzzerStatus(BUZZER_STATUS buzzerStatus)
{
	if(buzzerStatus == BUZZER_OPEN)
	{
		hfgpio_pwm_enable(HFGPIO_F_BUZZER, 1500, 50);
		g_buzzer_status = BUZZER_OPEN;
		//lumi_debug("buzzer open\n");
	}
	else
	{
		hfgpio_pwm_disable(HFGPIO_F_BUZZER);
		//hfgpio_fset_out_low(HFGPIO_F_BUZZER);
		g_buzzer_status = BUZZER_CLOSE;
		//lumi_debug("buzzer close\n");
	}
}


void USER_FUNC switchBuzzerStatus(void)
{
	if(g_buzzer_status == BUZZER_OPEN)
	{
		setBuzzerStatus(BUZZER_CLOSE);
	}
	else
	{
		setBuzzerStatus(BUZZER_OPEN);
	}
}


void USER_FUNC buzzerRingNotice(S32 period, S32 ringTims)
{
	S32  i;

	for(i=0; i<ringTims; i++)
	{
		setBuzzerStatus(BUZZER_OPEN);
		msleep(period);
		setBuzzerStatus(BUZZER_CLOSE);
		msleep(period);
	}
	hfgpio_fset_out_low(HFGPIO_F_BUZZER);
}


static void USER_FUNC initBuzzerRingInfo(const BUZZER_RING_DATA* pRindData)
{
	buzzerRingInfo.ringPeriodIndex = 0;
	buzzerRingInfo.curTimes = 0;
	buzzerRingInfo.startTime = time(NULL);
	buzzerRingInfo.pRingData = pRindData;
	buzzerRingInfo.ringStop = FALSE;
}


S32 USER_FUNC getBuzzerRingPeriod(const BUZZER_RING_DATA* initRingData)
{
	S32 period;
	time_t curTime = time(NULL);

	if(initRingData != NULL)
	{
		initBuzzerRingInfo(initRingData);
		setBuzzerStatus(BUZZER_CLOSE);
		hfgpio_fset_out_low(HFGPIO_F_BUZZER);
	}
	else if(buzzerRingInfo.ringStop)
	{
		buzzerRingInfo.ringStop = FALSE;
		buzzerRingInfo.startTime = curTime;
		buzzerRingInfo.ringPeriodIndex = 0;
	}

	if((curTime - buzzerRingInfo.startTime) > buzzerRingInfo.pRingData->ringPeriod)
	{
		buzzerRingInfo.curTimes++;
		if(buzzerRingInfo.curTimes >= buzzerRingInfo.pRingData->maxRingTimes)
		{
			period = -1;
			lumi_debug("stop buzzer\n");
		}
		else
		{
			period = buzzerRingInfo.pRingData->stopPeriod*1000;
			buzzerRingInfo.ringStop = TRUE;
			lumi_debug("now stop sometimes \n");
		}
		setBuzzerStatus(BUZZER_CLOSE);
		hfgpio_fset_out_low(HFGPIO_F_BUZZER);
	}
	else
	{
		if(buzzerRingInfo.pRingData->pRindPeriod[buzzerRingInfo.ringPeriodIndex] == 0)
		{
			buzzerRingInfo.ringPeriodIndex = 0;
		}
		period = (S32)buzzerRingInfo.pRingData->pRindPeriod[buzzerRingInfo.ringPeriodIndex];
		buzzerRingInfo.ringPeriodIndex++;
	}
	return period;
}



#ifdef EXTRA_SWITCH_SUPPORT

static BOOL USER_FUNC getExtraSwitchStatus(void)
{
	if(hfgpio_fpin_is_high(HFGPIO_F_EXTRA_SWITCH))
	{
		return TRUE;
	}
	return FALSE;
}


static void USER_FUNC extraSwitchIrq(U32 arg1,U32 arg2)
{
	S32 curPinStatus;

	curPinStatus = getExtraSwitchStatus();
	
	if(curPinStatus != extraSwitchIsHigh)
	{
		changeSwitchStatus();
		extraSwitchIsHigh = curPinStatus;
	}
}


static void USER_FUNC registerExtraSwitchInterrupt(void)
{
	//hfgpio_configure_fpin(HFGPIO_F_EXTRA_SWITCH, HFPIO_PULLUP | HFM_IO_TYPE_INPUT);
	hfgpio_configure_fpin(HFGPIO_F_EXTRA_SWITCH, HFM_IO_TYPE_INPUT);
	if(hfgpio_configure_fpin_interrupt(HFGPIO_F_EXTRA_SWITCH, HFPIO_IT_EDGE, extraSwitchIrq, 1)!= HF_SUCCESS)
	{
		lumi_debug("configure HFGPIO_F_EXTRA_SWITCH fail\n");
		return;
	}
	extraSwitchIsHigh = getExtraSwitchStatus();
}

#endif //EXTRA_SWITCH_SUPPORT


void USER_FUNC initDevicePin(BOOL initBeforNormal)
{
	if(initBeforNormal)
	{
		initBuzzerStatus();
	}
	else
	{
#ifdef EXTRA_SWITCH_SUPPORT
		registerExtraSwitchInterrupt();
#endif
	}
}

#endif

