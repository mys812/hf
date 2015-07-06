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
#include "../inc/lumTimeData.h"
#ifdef RN8209C_SUPPORT
#include "../inc/rn8209c.h"
#endif
#ifdef LUM_FACTORY_TEST_SUPPORT
#include "../inc/lumFactoryTest.h"
#endif


#ifdef BUZZER_RING_SUPPORT
static BUZZER_STATUS g_buzzer_status = BUZZER_CLOSE;
static BUZZER_RING_INFO buzzerRingInfo;
#endif

#ifdef EXTRA_SWITCH_SUPPORT
#if 0
static BOOL extraSwitchIsHigh;
#endif
#ifdef TWO_SWITCH_SUPPORT
static BOOL extraSwitch2IsHigh;
#endif
#endif //EXTRA_SWITCH_SUPPORT

#ifdef LIGHT_CHENGE_SUPPORT
static U8 g_lightLevel = 0;
#endif

#ifdef CHANGE_BRIGHTNESS_SUPPORT
static LIGHT_LET_INFO g_ledInfo;
static hftimer_handle_t g_lightLedStatusTimer = NULL;
#endif



#ifdef LIGHT_CHENGE_SUPPORT
static hftimer_handle_t lightHWTimerHandle = NULL;
static LIGHT_DIM_STATUS g_lightDimStatus = GET_AC_FREQ;
static U8 g_acFrequrence = 0;
static U32 g_hwTimerperiod = 0;  //设置成全局变量为了节约硬件timer callback计算时间


extern unsigned int GpioGetReg(unsigned char RegIndex);;
extern void GpioSetRegOneBit(unsigned char	RegIndex, unsigned int GpioIndex);
extern void GpioClrRegOneBit(unsigned char	RegIndex, unsigned int GpioIndex);
#endif


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


#if !defined(LIGHT_CHENGE_SUPPORT) && !defined(CHANGE_BRIGHTNESS_SUPPORT)
static S32 USER_FUNC lum_getSwitchFlag(SWITCH_PIN_FLAG switchFlag)
{
	S32 fid;

	if(switchFlag == SWITCH_PIN_1)
	{
		fid = HFGPIO_F_SWITCH;
	}
#ifdef TWO_SWITCH_SUPPORT
	else if(switchFlag == SWITCH_PIN_2)
	{
		fid = HFGPIO_F_SWITCH_2;
	}
#endif
	else
	{
		fid = HFGPIO_F_SWITCH;
		lumi_error("lum_getSwitchFlag switchFlag=%d\n", switchFlag);
	}
	return fid;
}
#endif

SWITCH_STATUS USER_FUNC getSwitchStatus(SWITCH_PIN_FLAG switchFlag)
{
#ifdef LIGHT_CHENGE_SUPPORT
	if(g_lightLevel > 0)
	{
		return SWITCH_OPEN;
	}
	return SWITCH_CLOSE;
#elif defined(CHANGE_BRIGHTNESS_SUPPORT)
	if(g_ledInfo.ledOpen != 0)
	{
		return SWITCH_OPEN;
	}
	return SWITCH_CLOSE;

#else
	S32 fid;

	fid = lum_getSwitchFlag(switchFlag);
	if(hfgpio_fpin_is_high(fid))
	{
		return SWITCH_OPEN;
	}
	return SWITCH_CLOSE;
#endif
}


void USER_FUNC setSwitchStatus(SWITCH_STATUS action, SWITCH_PIN_FLAG switchFlag)
{
	SWITCH_STATUS switchStatus = getSwitchStatus(switchFlag);
#if !defined(LIGHT_CHENGE_SUPPORT) && !defined(CHANGE_BRIGHTNESS_SUPPORT)
	S32 fid;


	fid = lum_getSwitchFlag(switchFlag);	
#endif

	if(SWITCH_OPEN == action)
	{
#ifdef LIGHT_CHENGE_SUPPORT
		lum_lightChangeLevel(MAX_LIGHT_LEVEL);
		if(g_lightDimStatus != GET_AC_FREQ)
		{
			hfgpio_fenable_interrupt(HFGPIO_F_ZERO_DETECTER);
		}
#elif defined(CHANGE_BRIGHTNESS_SUPPORT)
		lum_setLedLightStatus(LIGHT_LED_OPEN);
#else
		hfgpio_fset_out_high(fid);
#endif
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
#ifdef LIGHT_CHENGE_SUPPORT
		lum_lightChangeLevel(0);
		if(g_lightDimStatus != GET_AC_FREQ)
		{
			hfgpio_fdisable_interrupt(HFGPIO_F_ZERO_DETECTER);
		}
#elif defined(CHANGE_BRIGHTNESS_SUPPORT)
		lum_setLedLightStatus(LIGHT_LED_CLOSE);
#else
		hfgpio_fset_out_low(fid);
#endif
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
		GPIO_CHANGE_REPORT data;


		data.action = action;
		data.pinFlag = switchFlag;

		if(!lum_bEnterFactoryTest())
		{
			insertLocalMsgToList(MSG_LOCAL_EVENT, (U8*)(&data), sizeof(GPIO_CHANGE_REPORT), MSG_CMD_REPORT_GPIO_CHANGE);
		}
	}
}


void USER_FUNC changeSwitchStatus(SWITCH_PIN_FLAG switchFlag)
{
	if(getSwitchStatus(switchFlag) == SWITCH_OPEN)
	{
		setSwitchStatus(SWITCH_CLOSE, switchFlag);
	}
	else
	{
		setSwitchStatus(SWITCH_OPEN, switchFlag);
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
		hfgpio_fset_out_low(HFGPIO_F_BUZZER);
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
	hfgpio_pwm_disable(HFGPIO_F_BUZZER);
	hfgpio_fset_out_low(HFGPIO_F_BUZZER);
}


static void USER_FUNC initBuzzerRingInfo(const BUZZER_RING_DATA* pRindData)
{
	buzzerRingInfo.ringPeriodIndex = 0;
	buzzerRingInfo.curTimes = 0;
	buzzerRingInfo.startTime = lum_getSystemTime();
	buzzerRingInfo.pRingData = pRindData;
	buzzerRingInfo.ringStop = FALSE;
}


S32 USER_FUNC getBuzzerRingPeriod(const BUZZER_RING_DATA* initRingData)
{
	S32 period;
	U32 curTime = lum_getSystemTime();

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
#if 0
static void USER_FUNC extraSwitchIrq(U32 arg1,U32 arg2);

static S32 USER_FUNC lum_getExtraSwitchFlag(SWITCH_PIN_FLAG switchFlag)
{
	S32 fid;

	if(switchFlag == SWITCH_PIN_1)
	{
		fid = HFGPIO_F_EXTRA_SWITCH;
	}
#ifdef TWO_SWITCH_SUPPORT
	else if(switchFlag == SWITCH_PIN_2)
	{
		fid = HFGPIO_F_EXTRA_SWITCH_2;
	}
#endif
	else
	{
		fid = HFGPIO_F_EXTRA_SWITCH;
		lumi_error("lum_getSwitchFlag switchFlag=%d\n", switchFlag);
	}
	return fid;
}


static BOOL USER_FUNC getExtraSwitchStatus(SWITCH_PIN_FLAG switchFlag)
{
	S32 fid;


	fid = lum_getExtraSwitchFlag(switchFlag);
	if(hfgpio_fpin_is_high(fid))
	{
		return TRUE;
	}
	return FALSE;
}


static void USER_FUNC lum_extraKeyTimerCallback( hftimer_handle_t htimer )
{
	S32 curPinStatus;

	curPinStatus = getExtraSwitchStatus(SWITCH_PIN_1);
	
	if(curPinStatus == extraSwitchIsHigh)
	{
		U32 irqFlag = HFPIO_IT_HIGH_LEVEL;

		
#ifdef LUM_FACTORY_TEST_SUPPORT
		lum_addFactoryKeyPressTimes(FALSE, TRUE, FALSE);
#endif
		changeSwitchStatus(SWITCH_PIN_1);

		if(hfgpio_fpin_is_high(HFGPIO_F_EXTRA_SWITCH))
		{
			irqFlag = HFPIO_IT_LOW_LEVEL;
		}
		hfgpio_configure_fpin_interrupt(HFGPIO_F_EXTRA_SWITCH, (HFM_IO_TYPE_INPUT | irqFlag | HFPIO_PULLUP), extraSwitchIrq, 1);
	}
}


static void USER_FUNC lum_startExtraKeyTimer(void)
{
	static hftimer_handle_t irqExtrakeyTimer = NULL;


	if(irqExtrakeyTimer == NULL)
	{
		irqExtrakeyTimer = hftimer_create("Device_Key_TIMER_debounce", 30, false, EXTERA_KEY_DEBOUNCE_TIMER_ID, lum_extraKeyTimerCallback, 0);
	}
	hftimer_change_period(irqExtrakeyTimer, 30);
}


static void USER_FUNC extraSwitchIrq(U32 arg1,U32 arg2)
{
	extraSwitchIsHigh = getExtraSwitchStatus(SWITCH_PIN_1);
	lum_startExtraKeyTimer();
}
#endif

#ifdef TWO_SWITCH_SUPPORT
static void USER_FUNC extraSwitchIrq2(U32 arg1,U32 arg2);

static void USER_FUNC lum_extraKey2TimerCallback( hftimer_handle_t htimer )
{
	S32 curPinStatus;

	curPinStatus = getExtraSwitchStatus(SWITCH_PIN_2);
	
	if(curPinStatus == extraSwitch2IsHigh)
	{
		U32 irqFlag = HFPIO_IT_HIGH_LEVEL;

		
#ifdef LUM_FACTORY_TEST_SUPPORT
		lum_addFactoryKeyPressTimes(FALSE, FALSE, TRUE);
#endif
		changeSwitchStatus(SWITCH_PIN_2);

	
		if(hfgpio_fpin_is_high(HFGPIO_F_EXTRA_SWITCH_2))
		{
			irqFlag = HFPIO_IT_LOW_LEVEL;
		}
		hfgpio_configure_fpin_interrupt(HFGPIO_F_EXTRA_SWITCH_2, (HFM_IO_TYPE_INPUT | irqFlag | HFPIO_PULLUP), extraSwitchIrq2, 1);
	}

}


static void USER_FUNC lum_startExtraKey2Timer(void)
{
	static hftimer_handle_t irqExtrakey2Timer = NULL;


	if(irqExtrakey2Timer == NULL)
	{
		irqExtrakey2Timer = hftimer_create("Device_Key_TIMER_debounce", 30, false, EXTERA_KEY2_DEBOUNCE_TIMER_ID, lum_extraKey2TimerCallback, 0);
	}
	hftimer_change_period(irqExtrakey2Timer, 30);
}


static void USER_FUNC extraSwitchIrq2(U32 arg1,U32 arg2)
{
	extraSwitch2IsHigh = getExtraSwitchStatus(SWITCH_PIN_2);
	lum_startExtraKey2Timer();
}

#endif

static void USER_FUNC registerExtraSwitchInterrupt(void)
{
#if 0
	U32 irqFlag = HFPIO_IT_HIGH_LEVEL;


	if(hfgpio_fpin_is_high(HFGPIO_F_EXTRA_SWITCH))
	{
		irqFlag = HFPIO_IT_LOW_LEVEL;
	}
	//hfgpio_configure_fpin(HFGPIO_F_EXTRA_SWITCH, HFM_IO_TYPE_INPUT);
	if(hfgpio_configure_fpin_interrupt(HFGPIO_F_EXTRA_SWITCH, (HFM_IO_TYPE_INPUT | irqFlag | HFPIO_PULLUP), extraSwitchIrq, 1)!= HF_SUCCESS)
	{
		lumi_debug("configure HFGPIO_F_EXTRA_SWITCH fail\n");
		return;
	}
#endif
#ifdef TWO_SWITCH_SUPPORT
	if(hfgpio_fpin_is_high(HFGPIO_F_EXTRA_SWITCH_2))
	{
		irqFlag = HFPIO_IT_LOW_LEVEL;
	}
	//hfgpio_configure_fpin(HFGPIO_F_EXTRA_SWITCH_2, HFM_IO_TYPE_INPUT);
	if(hfgpio_configure_fpin_interrupt(HFGPIO_F_EXTRA_SWITCH_2, (HFM_IO_TYPE_INPUT | irqFlag | HFPIO_PULLUP), extraSwitchIrq2, 1)!= HF_SUCCESS)
	{
		lumi_debug("configure HFGPIO_F_EXTRA_SWITCH2 fail\n");
		return;
	}
#endif
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
		if(checkResetType() != RESET_FOR_SMARTLINK /* && !lum_bEnterFactoryTest() */)
		{
			lum_deviceFactoryReset();
		}
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
				deviceKeyTimer = hftimer_create("Device_Key_TIMER", 3000, false, DEVICE_KEY_TIMER_ID, deviceKeyTimerCallback, 0);
			}
			hftimer_change_period(deviceKeyTimer, 3000);
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
#ifdef RN8209_CALIBRATE_SELF
				if(lum_getKeyEnableStatus())
#endif
				{
					changeSwitchStatus(SWITCH_PIN_1);
#ifdef LUM_FACTORY_TEST_SUPPORT
					lum_addFactoryKeyPressTimes(TRUE, FALSE, FALSE);
#endif
				}
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

	irqFlag = HFM_IO_TYPE_INPUT | HFPIO_IT_EDGE | HFPIO_PULLUP;

	if(hfgpio_configure_fpin_interrupt(HFGPIO_F_KEY, irqFlag , deviceKeyPressIrq, 1)!= HF_SUCCESS)
	{
		lumi_debug("configure HFGPIO_F_KEY fail\n");
		return;
	}
}


#ifdef LUM_FACTORY_TEST_SUPPORT
void USER_FUNC lum_DisableKeyInterrupt(void)
{
	hfgpio_fdisable_interrupt(HFGPIO_F_KEY);
}


void USER_FUNC lum_EnableKeyInterrupt(void)
{
	hfgpio_fenable_interrupt(HFGPIO_F_KEY);
}
#endif //LUM_FACTORY_TEST_SUPPORT

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


#ifdef RN8209C_SUPPORT
static void USER_FUNC lum_rn8209cCfIrq(U32 arg1,U32 arg2)
{
	lum_rn8209cAddEnergyData();
}


void USER_FUNC lum_rn8209cInitCfPin(void)
{
	if(hfgpio_configure_fpin_interrupt(HFGPIO_F_CF, HFM_IO_TYPE_INPUT | HFPIO_IT_RISE_EDGE | HFPIO_PULLDOWN, lum_rn8209cCfIrq, 1)!= HF_SUCCESS)
	{
		lumi_debug("configure HFGPIO_F_CF fail\n");
		return;
	}
}
#endif


#ifdef LIGHT_CHENGE_SUPPORT
void USER_FUNC lum_lightChangeLevel(U8 level)
{
	if(level != g_lightLevel)
	{
		g_lightLevel = level;

		if(g_lightLevel > 0)
		{
			hfgpio_fset_out_low(HFGPIO_F_LED1);
		}
		else
		{
			hfgpio_fset_out_high(HFGPIO_F_LED1);
		}

		if(g_lightLevel > 1)
		{
			hfgpio_fset_out_low(HFGPIO_F_LED2);
		}
		else
		{
			hfgpio_fset_out_high(HFGPIO_F_LED2);
		}

		if(g_lightLevel > 2)
		{
			hfgpio_fset_out_low(HFGPIO_F_LED3);
		}
		else
		{
			hfgpio_fset_out_high(HFGPIO_F_LED3);
		}

		if(g_lightLevel > 3)
		{
			hfgpio_fset_out_low(HFGPIO_F_LED4);
		}
		else
		{
			hfgpio_fset_out_high(HFGPIO_F_LED4);
		}

		if(g_lightLevel > 4)
		{
			hfgpio_fset_out_low(HFGPIO_F_LED5);
		}
		else
		{
			hfgpio_fset_out_high(HFGPIO_F_LED5);
		}

		if(g_lightLevel > 5)
		{
			hfgpio_fset_out_low(HFGPIO_F_LED6);
		}
		else
		{
			hfgpio_fset_out_high(HFGPIO_F_LED6);
		}

		if(g_lightLevel > 6)
		{
			hfgpio_fset_out_low(HFGPIO_F_LED7);
		}
		else
		{
			hfgpio_fset_out_high(HFGPIO_F_LED7);
		}
		g_hwTimerperiod = LIGHT_DIM_BASE_TIME + (MAX_LIGHT_LEVEL - g_lightLevel)*LIGHT_DIM_LEVEL_GAP;
	}
}


static void USER_FUNC lum_lightLedInit(void)
{
	hfgpio_fset_out_high(HFGPIO_F_LED1);
	hfgpio_fset_out_high(HFGPIO_F_LED2);
	hfgpio_fset_out_high(HFGPIO_F_LED3);
	hfgpio_fset_out_high(HFGPIO_F_LED4);
	hfgpio_fset_out_high(HFGPIO_F_LED5);
	hfgpio_fset_out_high(HFGPIO_F_LED6);
	hfgpio_fset_out_high(HFGPIO_F_LED7);
}


static void USER_FUNC lum_lightKeyUpTimerCallback( hftimer_handle_t htimer )
{
	U8 tmpLevel;


	if(hfgpio_fpin_is_high(HFGPIO_F_KEY_UP))
	{
		return;
	}
	if(g_lightLevel < MAX_LIGHT_LEVEL)
	{
		tmpLevel = g_lightLevel + 1;
		lum_lightChangeLevel(tmpLevel);
	}
}


static void USER_FUNC lum_lightKeyUpIrq(U32 arg1,U32 arg2)
{
	static hftimer_handle_t lightKeyupTimerHandle = NULL;


	if(g_lightDimStatus == GET_AC_FREQ)  //获取市电频率期间禁止按键
	{
		return;
	}
	if(g_lightLevel == 0) //关闭时上键禁止开灯
	{
		return;
	}
	if(lightKeyupTimerHandle == NULL)
	{
		lightKeyupTimerHandle = hftimer_create("lightKeyUp", LIGHT_KEY_DEBOUNCE, false, LIGHT_KEYUP_TIMER_ID, lum_lightKeyUpTimerCallback, 0);
	}
	hftimer_change_period(lightKeyupTimerHandle, LIGHT_KEY_DEBOUNCE);
}


static void USER_FUNC lum_lightKeyDownTimerCallback( hftimer_handle_t htimer )
{
	U8 tmpLevel;

	if(hfgpio_fpin_is_high(HFGPIO_F_KEY_DOWN))
	{
		return;
	}
	if(g_lightLevel > 1) //最低有1级亮度
	{
		tmpLevel = g_lightLevel - 1;
		lum_lightChangeLevel(tmpLevel);
	}

}


static void USER_FUNC lum_lightKeyDownIrq(U32 arg1,U32 arg2)
{
	static hftimer_handle_t lightKeydownTimerHandle = NULL;

	if(g_lightDimStatus == GET_AC_FREQ)  //获取市电频率期间禁止按键
	{
		return;
	}
	if(lightKeydownTimerHandle == NULL)
	{
		lightKeydownTimerHandle = hftimer_create("lightKeyDown", LIGHT_KEY_DEBOUNCE, false, LIGHT_KEYDOWN_TIMER_ID, lum_lightKeyDownTimerCallback, 0);
	}
	hftimer_change_period(lightKeydownTimerHandle, LIGHT_KEY_DEBOUNCE);

}


static void USER_FUNC lum_lightDimShutdown(void)
{
	hftimer_change_period(lightHWTimerHandle, LIGHT_DIM_SHUTDOWN_TIME);
	g_lightDimStatus = SHUT_DOWN_DIM;
}


static void USER_FUNC lum_lightHwTimerCallback( hftimer_handle_t htimer )
{
	if(g_lightDimStatus == ZERO_DETECT)
	{
		//hfgpio_fset_out_high(HFGPIO_F_DIM);
		//GpioSetRegOneBit(GPIO_B_OUT, (1<<20)); //O18_GPIO_BANK_B(20),  //lpb SPI_MOSI
		GpioSetRegOneBit(GPIO_B_OUT, 0x100000);
		lum_lightDimShutdown();
	}
	else if(g_lightDimStatus == SHUT_DOWN_DIM)
	{
		//hfgpio_fset_out_low(HFGPIO_F_DIM);
		//GpioClrRegOneBit(GPIO_B_OUT, (1<<20));	 //O18_GPIO_BANK_B(20),  //lpb SPI_MOSI
		GpioClrRegOneBit(GPIO_B_OUT, 0x100000);
	}
	else if(g_lightDimStatus == GET_AC_FREQ)
	{
		g_lightDimStatus = ZERO_DETECT;
		if(g_lightLevel == 0)
		{
			hfgpio_fdisable_interrupt(HFGPIO_F_ZERO_DETECTER);
		}
	}
}


static void USER_FUNC lum_lightHWTimerInit(BOOL bGetFreq)
{
	U32 period;


	if(lightHWTimerHandle == NULL)
	{
		lightHWTimerHandle = hftimer_create("lightDim", 110000, false, LIGHT_DIM_TIMER_ID, lum_lightHwTimerCallback, HFTIMER_FLAG_HARDWARE_TIMER);
	}

	if(bGetFreq) //获取市电频率
	{
		g_lightDimStatus = GET_AC_FREQ;
		g_acFrequrence = 0;
		period = 2000000;  //2S
		hftimer_change_period(lightHWTimerHandle, period);
	}
	else
	{
		if(g_lightLevel > 0)
		{
			//period = LIGHT_DIM_BASE_TIME + (MAX_LIGHT_LEVEL - g_lightLevel)*LIGHT_DIM_LEVEL_GAP;
			//hftimer_change_period(lightHWTimerHandle, period);
			hftimer_change_period(lightHWTimerHandle, g_hwTimerperiod);
		}
		g_lightDimStatus = ZERO_DETECT;
	}
}


static void USER_FUNC lum_ACZeroDetectIrq(U32 arg1,U32 arg2)
{
	if(g_lightDimStatus != GET_AC_FREQ)
	{
		//hfgpio_fset_out_low(HFGPIO_F_DIM);
		lum_lightHWTimerInit(FALSE);
	}
	else
	{
		g_acFrequrence++;
	}
}


void USER_FUNC lum_lightChangeIRQInit(void)
{
	if(hfgpio_configure_fpin_interrupt(HFGPIO_F_KEY_UP, HFM_IO_TYPE_INPUT | HFPIO_IT_FALL_EDGE | HFPIO_PULLUP, lum_lightKeyUpIrq, 1)!= HF_SUCCESS)
	{
		lumi_debug("configure KeyUp\n");
		return;
	}

	if(hfgpio_configure_fpin_interrupt(HFGPIO_F_KEY_DOWN, HFM_IO_TYPE_INPUT | HFPIO_IT_FALL_EDGE | HFPIO_PULLUP, lum_lightKeyDownIrq, 1)!= HF_SUCCESS)
	{
		lumi_debug("configure KeyDown\n");
		return;
	}

	if(hfgpio_configure_fpin_interrupt(HFGPIO_F_ZERO_DETECTER, HFM_IO_TYPE_INPUT | HFPIO_IT_EDGE | HFPIO_PULLUP, lum_ACZeroDetectIrq, 1)!= HF_SUCCESS)
	{
		lumi_debug("configure ZeroDetect\n");
		return;
	}
}
#endif


#ifdef CHANGE_BRIGHTNESS_SUPPORT
static void USER_FUNC lum_startLightLedStatusTimer(void);


static void USER_FUNC lum_setLightLedStatus(U8 brightLevel, U8 tempLevel)
{
	U8 whiteLedLevel;
	U8 yellowLevel;

	
#ifdef COLOR_TEMPERATURE_SUPPORT
	whiteLedLevel = (brightLevel*tempLevel)/100;
	yellowLevel = (brightLevel*(100-tempLevel))/100;
#else
	whiteLedLevel = brightLevel;
#endif

	if(whiteLedLevel == 0)
	{
		hfgpio_pwm_disable(HFGPIO_F_LED_BRIGHTNESS);
		hfgpio_fset_out_low(HFGPIO_F_LED_BRIGHTNESS);
	}
	else
	{
		hfgpio_pwm_enable(HFGPIO_F_LED_BRIGHTNESS, 400, whiteLedLevel);
	}
#ifdef COLOR_TEMPERATURE_SUPPORT
	if(yellowLevel == 0)
	{
		hfgpio_pwm_disable(HFGPIO_F_COLOR_TEMP);
		hfgpio_fset_out_low(HFGPIO_F_COLOR_TEMP);
	}
	else
	{
		hfgpio_pwm_enable(HFGPIO_F_COLOR_TEMP, 400, yellowLevel);
	}
#endif

	
}


static void USER_FUNC lum_lightLedStatusTimerCallback( hftimer_handle_t htimer )
{
	if(g_ledInfo.ledOpen == 0)
	{
		lum_setLightLedStatus(SMARTLINK_BRIGHT_LEVEL, SMARTLINK_BRIGHT_LEVEL);
		g_ledInfo.ledOpen = 1;
	}
	else
	{
		lum_setLightLedStatus(1, 1);
		g_ledInfo.ledOpen = 0;
	}
	lum_startLightLedStatusTimer();
}


static void USER_FUNC lum_startLightLedStatusTimer(void)
{
	S32 period;


	if(g_lightLedStatusTimer == NULL)
	{
		g_ledInfo.ledOpen = 0;
		g_lightLedStatusTimer = hftimer_create("ReadEnergyDataTestTimer", 5000, true, LIGHT_LED_STATUS_TIMER_ID, lum_lightLedStatusTimerCallback, 0);
	}
	if(g_ledInfo.ledStatus == LIGHT_NOT_UNCONNECT)
	{
		period = 2000; //2s
	}
	else
	{
		period = 150; //150ms
	}
	hftimer_change_period(g_lightLedStatusTimer, period);
}


static void USER_FUNC lum_stopLightLedStatusTimer(void)
{
	if(g_lightLedStatusTimer != NULL)
	{
		hftimer_stop(g_lightLedStatusTimer);
		hftimer_delete(g_lightLedStatusTimer);
		g_lightLedStatusTimer = NULL;
	}
}


void USER_FUNC lum_setLedLightStatus(LIGHT_STATUS_INDICATION letStatus)
{
	U8 tempLevel;
	U8 brightLevel;


	
	g_ledInfo.ledStatus = letStatus;
	if(letStatus == LIGHT_LED_OPEN || letStatus == LIGHT_LED_CLOSE)
	{
		lum_stopLightLedStatusTimer();
	}
	if(letStatus == LIGHT_LED_OPEN)
	{
		brightLevel = lum_getBrightnessLevel();
#ifdef COLOR_TEMPERATURE_SUPPORT
		tempLevel = lum_getTemperatureLevel();
#endif
		lum_setLightLedStatus(brightLevel, tempLevel);
		g_ledInfo.ledOpen = 1;
	}
	else if(letStatus == LIGHT_LED_CLOSE)
	{
		lum_setLightLedStatus(0, 0);
		g_ledInfo.ledOpen = 0;	
	}
	else if(letStatus == LIGHT_NOT_UNCONNECT ||  letStatus == LIGHT_SMARTLINK)
	{
		lum_startLightLedStatusTimer();
	}
}


#ifdef LUM_LIGHT_BRIGHT_TEST

//static U8 brightLevel = 0;
//static U8 tempLevel = 0;

static void USER_FUNC lum_wifiLightBrightKeyTimerCallback( hftimer_handle_t htimer )
{
	U8 brightLevel;
	U8 tempLevel = 0;

	
	if(hfgpio_fpin_is_high(HFGPIO_F_BRIGHT_CHANGE))
	{
		return;
	}

	brightLevel = lum_getBrightnessLevel();
#ifdef COLOR_TEMPERATURE_SUPPORT
	tempLevel = lum_getTemperatureLevel();
#endif
	brightLevel += 5;
	if(brightLevel > 100)
	{
		brightLevel = 0;
	}
	lum_setBrightnessLevel(brightLevel);
	lum_setLightLedStatus(brightLevel, tempLevel);
}


static void USER_FUNC lum_wifiLightBrightKeyIrq(U32 arg1,U32 arg2)
{
	static hftimer_handle_t lightBrightTimerHandle = NULL;

	if(lightBrightTimerHandle == NULL)
	{
		lightBrightTimerHandle = hftimer_create("lightKeyBright", 30, false, LIGHT_LED_BRIGHT_TIMER_ID, lum_wifiLightBrightKeyTimerCallback, 0);
	}
	hftimer_change_period(lightBrightTimerHandle, 30);

}



#ifdef COLOR_TEMPERATURE_SUPPORT

static void USER_FUNC lum_wifiLightTempKeyTimerCallback( hftimer_handle_t htimer )
{
	U8 brightLevel;
	U8 tempLevel = 0;


	if(hfgpio_fpin_is_high(HFGPIO_F_TEMP_CHANGE))
	{
		return;
	}

	brightLevel = lum_getBrightnessLevel();
	tempLevel = lum_getTemperatureLevel();
	tempLevel += 20;
	if(tempLevel > 100)
	{
		tempLevel = 0;
	}
	lum_setTemperatureLevel(tempLevel);
	lum_setLightLedStatus(brightLevel, tempLevel);
}


static void USER_FUNC lum_wifiLightTempKeyIrq(U32 arg1,U32 arg2)
{
	static hftimer_handle_t lightTempTimerHandle = NULL;

	if(lightTempTimerHandle == NULL)
	{
		lightTempTimerHandle = hftimer_create("lightKeyTemp", 30, false, LIGHT_LED_TEMPERATURE_TIMER_ID, lum_wifiLightTempKeyTimerCallback, 0);
	}
	hftimer_change_period(lightTempTimerHandle, 30);

}
#endif
#endif

static void USER_FUNC lum_wifiLightLedInit(void)
{
	g_ledInfo.ledOpen = 0;
	lum_setLedLightStatus(LIGHT_NOT_UNCONNECT);

#ifdef LUM_LIGHT_BRIGHT_TEST
	if(hfgpio_configure_fpin_interrupt(HFGPIO_F_BRIGHT_CHANGE, HFM_IO_TYPE_INPUT | HFPIO_IT_FALL_EDGE | HFPIO_PULLUP, lum_wifiLightBrightKeyIrq, 1)!= HF_SUCCESS)
	{
		lumi_debug("configure KeyDown\n");
		return;
	}
#ifdef COLOR_TEMPERATURE_SUPPORT
	if(hfgpio_configure_fpin_interrupt(HFGPIO_F_TEMP_CHANGE, HFM_IO_TYPE_INPUT | HFPIO_IT_FALL_EDGE | HFPIO_PULLUP, lum_wifiLightTempKeyIrq, 1)!= HF_SUCCESS)
	{
		lumi_debug("configure ZeroDetect\n");
		return;
	}
#endif
#endif
}

#endif


void USER_FUNC initDevicePin(void)
{
#ifdef BUZZER_RING_SUPPORT
	initBuzzerStatus();
#endif
#ifdef EXTRA_SWITCH_SUPPORT
	registerExtraSwitchInterrupt();
#endif
#ifdef DEVICE_KEY_SUPPORT
	if(checkResetType() != RESET_FOR_UPGRADE)
	{
		initKeyGpio();
#ifdef LIGHT_CHENGE_SUPPORT
		lum_lightLedInit();
		lum_lightChangeIRQInit();
		lum_lightHWTimerInit(TRUE);
#endif
	}
#endif
#ifdef DEVICE_WIFI_LED_SUPPORT
	setWifiLedStatus(WIFI_LED_AP_DISCONNECT);
#endif
	setSwitchStatus(SWITCH_CLOSE, SWITCH_PIN_1);
#ifdef TWO_SWITCH_SUPPORT
	setSwitchStatus(SWITCH_CLOSE, SWITCH_PIN_2);
#endif
#ifdef CHANGE_BRIGHTNESS_SUPPORT
	lum_wifiLightLedInit();
#endif
}

#endif

