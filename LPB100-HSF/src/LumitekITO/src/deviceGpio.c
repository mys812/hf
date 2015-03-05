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

#ifdef BUZZER_RING_SUPPORT
static BUZZER_STATUS g_buzzer_status = BUZZER_CLOSE;
static BUZZER_RING_INFO buzzerRingInfo;
#endif

#ifdef EXTRA_SWITCH_SUPPORT
static BOOL extraSwitchIsHigh;
#endif //EXTRA_SWITCH_SUPPORT




#ifdef SPECIAL_RELAY_SUPPORT
static hftimer_handle_t specialRelayTimer = NULL;


static void USER_FUNC specialRelayTimerCallback( hftimer_handle_t htimer )
{
	if(hfgpio_fpin_is_high(HFGPIO_F_RELAY_2))
	{
		hfgpio_fset_out_low(HFGPIO_F_RELAY_2);
	}
	if(hfgpio_fpin_is_high(HFGPIO_F_RELAY_1))
	{
		hfgpio_fset_out_low(HFGPIO_F_RELAY_1);
	}
}


static void USER_FUNC startSpecialRelayTimer(void)
{
	if(specialRelayTimer == NULL)
	{
		specialRelayTimer  = hftimer_create("Special_relay_TIMER", 200, false, SPECILA_RELAY_TIMER_ID, specialRelayTimerCallback, 0);
	}
	hftimer_change_period(specialRelayTimer, 200);
}
#endif

#ifdef DEEVICE_LUMITEK_P2
SWITCH_STATUS USER_FUNC getSwitchStatus(void)
{
	if(hfgpio_fpin_is_high(HFGPIO_F_NORMAL_RELAY))
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
		hfgpio_fset_out_high(HFGPIO_F_NORMAL_RELAY);
#ifdef SPECIAL_RELAY_SUPPORT
		hfgpio_fset_out_high(HFGPIO_F_RELAY_2);
		hfgpio_fset_out_low(HFGPIO_F_RELAY_1);
		startSpecialRelayTimer();
#endif
#ifdef DEVICE_RELAY_LED_SUPPORT
		hfgpio_fset_out_low(HFGPIO_F_RELAY_LED);
#endif
	}
	else
	{
		hfgpio_fset_out_low(HFGPIO_F_NORMAL_RELAY);
#ifdef SPECIAL_RELAY_SUPPORT
		hfgpio_fset_out_low(HFGPIO_F_RELAY_2);
		hfgpio_fset_out_high(HFGPIO_F_RELAY_1);
		startSpecialRelayTimer();
#endif
#ifdef DEVICE_RELAY_LED_SUPPORT
		hfgpio_fset_out_high(HFGPIO_F_RELAY_LED);
#endif
	}
	if(action != switchStatus)
	{
		U8 data = (action == SWITCH_OPEN)?1:0;
		
		insertLocalMsgToList(MSG_LOCAL_EVENT, &data, 1, MSG_CMD_REPORT_GPIO_CHANGE);
	}
}

#elif defined(DEEVICE_LUMITEK_P1) || defined(DEEVICE_LUMITEK_P3)
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
#else
	#error "No device defined !"
#endif

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


#ifdef BUZZER_RING_SUPPORT
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


void USER_FUNC buzzerRingNotice(S32 ringPeriod, S32 stopPeriod, S32 ringTims)
{
	S32  i;

	for(i=0; i<ringTims; i++)
	{
		setBuzzerStatus(BUZZER_OPEN);
		msleep(ringPeriod);
		setBuzzerStatus(BUZZER_CLOSE);
		msleep(stopPeriod);
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
#endif


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


#ifdef DEVICE_KEY_SUPPORT
static BOOL g_bLongPress = FALSE;
static hftimer_handle_t deviceKeyTimer = NULL;


static BOOL USER_FUNC getDevicekeyPressStatus(void)
{
	if(hfgpio_fpin_is_high(HFGPIO_F_KEY))
	{
		return FALSE;
	}
	return TRUE;
}


static void USER_FUNC deviceKeyTimerCallback( hftimer_handle_t htimer )
{
	if(getDevicekeyPressStatus())
	{
		g_bLongPress = TRUE;
		sendSmartLinkCmd();
	}
}



static void USER_FUNC irqDebounceTimerCallback( hftimer_handle_t htimer )
{
	BOOL bKeyPressed;
	static BOOL g_bKeyPressed = FALSE;


	bKeyPressed = getDevicekeyPressStatus();
	if(g_bKeyPressed != bKeyPressed)
	{
		if(bKeyPressed)
		{
			g_bLongPress = FALSE;
			if(deviceKeyTimer == NULL)
			{
				deviceKeyTimer = hftimer_create("Device_Key_TIMER", 5000, false, DEVICE_KEY_TIMER_ID, deviceKeyTimerCallback, 0);
			}
			hftimer_change_period(deviceKeyTimer, 5000);
		}
		else
		{
			if(g_bLongPress)
			{
				g_bLongPress = FALSE;
			}
			else
			{
				hftimer_stop(deviceKeyTimer);
				changeSwitchStatus();
			}
		}
		HF_Debug(DEBUG_LEVEL_USER, "========> deviceKeyPressIrq bKeyPressed=%d\n", bKeyPressed);
	}
	else
	{
		HF_Debug(DEBUG_LEVEL_USER, "IRQ dispatch g_bKeyPressed=%d bKeyPressed=%d\n", g_bKeyPressed, bKeyPressed);
	}
	hfgpio_fenable_interrupt(HFGPIO_F_KEY);
	g_bKeyPressed = bKeyPressed;
}


static void USER_FUNC startIrqDebounceTimer(void)
{
	static hftimer_handle_t irqDebounceTimer = NULL;


	hfgpio_fdisable_interrupt(HFGPIO_F_KEY);
	if(irqDebounceTimer == NULL)
	{
		irqDebounceTimer = hftimer_create("Device_Key_TIMER_debounce", 30, false, KEY_IRQ_DEBOUNCE_TIMER_ID, irqDebounceTimerCallback, 0);
	}
	hftimer_change_period(irqDebounceTimer, 30);
}


static void USER_FUNC deviceKeyPressIrq(U32 arg1,U32 arg2)
{
	startIrqDebounceTimer();
}


void USER_FUNC initKeyGpio(void)
{
	U32 irqFlag;

	irqFlag = HFM_IO_TYPE_INPUT | HFPIO_IT_EDGE;
#ifdef DEEVICE_LUMITEK_P3
	irqFlag |= HFPIO_PULLUP;
#endif
	if(hfgpio_configure_fpin_interrupt(HFGPIO_F_KEY, irqFlag , deviceKeyPressIrq, 1)!= HF_SUCCESS)
	{
		lumi_debug("configure HFGPIO_F_KEY fail\n");
		return;
	}
}
#endif


#ifdef DEVICE_WIFI_LED_SUPPORT
void USER_FUNC changeWifiLedStatus(BOOL needClose)
{
	if(needClose || hfgpio_fpin_is_high(HFGPIO_F_WIFI_LED) <= 0)
	{
		hfgpio_fset_out_high(HFGPIO_F_WIFI_LED);
	}
	else
	{
		hfgpio_fset_out_low(HFGPIO_F_WIFI_LED);
	}
}
#endif


void USER_FUNC initDevicePin(BOOL initBeforNormal)
{
	if(initBeforNormal)
	{
#ifdef BUZZER_RING_SUPPORT
		initBuzzerStatus();
#endif
	}
	else
	{
#ifdef EXTRA_SWITCH_SUPPORT
		registerExtraSwitchInterrupt();
#endif
#ifdef DEVICE_KEY_SUPPORT
		initKeyGpio();
#endif
#ifdef DEVICE_WIFI_LED_SUPPORT
		setWifiLedStatus(WIFI_LED_NO_CONNECT);
#endif
		setSwitchStatus(SWITCH_CLOSE);

	}
}

#endif

