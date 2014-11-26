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


#ifdef DEEVICE_LUMITEK_P1
#define BUZZER_RING_PREIOD		60


static BUZZER_STATUS g_buzzer_status = BUZZER_CLOSE;
static BUZZER_RING_INFO buzzerRingInfo;
#endif


#ifdef LPB100_DEVLOPMENT_BOARD
static void USER_FUNC smartLinkKeyIrq(U32 arg1,U32 arg2)
{
	static time_t g_key_pressdown_time;
	time_t now = time(NULL);

	if(hfgpio_fpin_is_high(HFGPIO_F_SMARTLINK)) //key up
	{
		if((now - g_key_pressdown_time) >= 3)
		{
			sendSmartLinkCmd();
		}
		else
		{
			switchLightStatus();
		}
	}
	else //key down
	{
		g_key_pressdown_time = now;
	}
}


void USER_FUNC keyGpioInit(void)
{
	if(hfgpio_configure_fpin_interrupt(HFGPIO_F_SMARTLINK, HFPIO_IT_EDGE, smartLinkKeyIrq, 1)!= HF_SUCCESS)
	{
		lumi_debug("configure HFGPIO_F_SMARTLINK fail\n");
		return;
	}
}


static void USER_FUNC setLightStatus(LIGHT_STATUS lightStatus)
{
	if(lightStatus == LIGHT_OPEN)
	{
		hfgpio_fset_out_low(HFGPIO_F_LIGHT);
	}
	else
	{
		hfgpio_fset_out_high(HFGPIO_F_LIGHT);
	}
}


static LIGHT_STATUS USER_FUNC getLightStatus(void)
{
	if(hfgpio_fpin_is_high(HFGPIO_F_LIGHT))
	{
		return LIGHT_CLOSE;
	}
	return LIGHT_OPEN;
}


void USER_FUNC switchLightStatus(void)
{
	if(getLightStatus() == LIGHT_OPEN)
	{
		setLightStatus(LIGHT_CLOSE);
	}
	else
	{
		setLightStatus(LIGHT_OPEN);
	}
}

#endif

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



#ifdef DEEVICE_LUMITEK_P1

static void USER_FUNC initBuzzerStatus(void)
{
	hfgpio_fset_out_low(HFGPIO_F_BUZZER);
	g_buzzer_status = BUZZER_CLOSE;
}


static void USER_FUNC setBuzzerStatus(BUZZER_STATUS buzzerStatus)
{
	if(buzzerStatus == BUZZER_OPEN)
	{
		hfgpio_pwm_enable(HFGPIO_F_BUZZER,buzzerRingInfo.freq, 50);
		g_buzzer_status = BUZZER_OPEN;
	}
	else
	{
		hfgpio_pwm_disable(HFGPIO_F_BUZZER);
		hfgpio_fset_out_low(HFGPIO_F_BUZZER);
		g_buzzer_status = BUZZER_CLOSE;
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


void USER_FUNC initBuzzerRingInfo(S32 freq)
{
	buzzerRingInfo.startTime = time(NULL);
	buzzerRingInfo.freq = freq;
}


BOOL USER_FUNC checkNeedStopBuzzerRing(void)
{
	BOOL ret = FALSE;
	U32 period;
	
	time_t cutTime = time(NULL);


	period = cutTime - buzzerRingInfo.startTime;
	if(period > BUZZER_RING_PREIOD)
	{
		if(g_buzzer_status == BUZZER_OPEN)
		{
			setBuzzerStatus(BUZZER_CLOSE);
		}
		ret = TRUE;
	}
	lumi_debug("Ring time = %d\n", period);
	return ret;
}



#ifdef EXTRA_SWITCH_SUPPORT
static void USER_FUNC extraSwitchIrq(U32 arg1,U32 arg2)
{
	SWITCH_STATUS switchStatus = getSwitchStatus();

	if(switchStatus == SWITCH_CLOSE)
	{
		setSwitchStatus(SWITCH_OPEN);
	}
	else
	{
		setSwitchStatus(SWITCH_CLOSE);
	}
}


void USER_FUNC extraSwitchInit(void)
{
	if(hfgpio_configure_fpin_interrupt(HFGPIO_F_EXTRA_SWITCH, HFPIO_IT_EDGE, extraSwitchIrq, 1)!= HF_SUCCESS)
	{
		lumi_debug("configure HFGPIO_F_EXTRA_SWITCH fail\n");
		return;
	}
}

#endif //EXTRA_SWITCH_SUPPORT

#endif //DEEVICE_LUMITEK_P1


void USER_FUNC initDevicePin(BOOL initBeforNormal)
{
	if(initBeforNormal)
	{
#ifdef DEEVICE_LUMITEK_P1
		initBuzzerStatus();
#endif
	}
	else
	{
#ifdef EXTRA_SWITCH_SUPPORT
		extraSwitchInit();
#endif
#ifdef LPB100_DEVLOPMENT_BOARD
		keyGpioInit();
#endif
	}
}

#endif

