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
static BUZZER_STATUS g_buzzer_status = BUZZER_CLOSE;
#endif

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
			if(getLightStatus() == LIGHT_OPEN)
			{
				setLightStatus(LIGHT_CLOSE);
			}
			else
			{
				setLightStatus(LIGHT_OPEN);
			}
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


SWITCH_STATUS USER_FUNC getSwitchStatus(void)
{
	if(hfgpio_fpin_is_high(HFGPIO_F_SWITCH))
	{
		return SWITCH_CLOSE;
	}
	return SWITCH_OPEN;
}


void USER_FUNC setSwitchStatus(SWITCH_STATUS action)
{
	SWITCH_STATUS switchStatus = getSwitchStatus();;

	
	if(SWITCH_OPEN == action)
	{
		hfgpio_fset_out_low(HFGPIO_F_SWITCH);
	}
	else
	{
		hfgpio_fset_out_high(HFGPIO_F_SWITCH);
	}
	if(action != switchStatus)
	{
		U8 data = (action == SWITCH_OPEN)?1:0;
		
		insertLocalMsgToList(MSG_LOCAL_EVENT, &data, 1, MSG_CMD_REPORT_GPIO_CHANGE);
	}
}



void USER_FUNC setLightStatus(LIGHT_STATUS lightStatus)
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


LIGHT_STATUS USER_FUNC getLightStatus(void)
{
	if(hfgpio_fpin_is_high(HFGPIO_F_LIGHT))
	{
		return LIGHT_CLOSE;
	}
	return LIGHT_OPEN;
}


#ifdef DEEVICE_LUMITEK_P1
void USER_FUNC setBuzzerStatus(BUZZER_STATUS buzzerStatus)
{
	if(buzzerStatus == BUZZER_OPEN)
	{
		hfgpio_pwm_enable(HFGPIO_F_BUZZER,6000,50);
		g_buzzer_status = BUZZER_OPEN;
	}
	else
	{
		hfgpio_pwm_disable(HFGPIO_F_BUZZER);
		hfgpio_fset_out_low(HFGPIO_F_BUZZER);
		g_buzzer_status = BUZZER_CLOSE;
	}
}


BUZZER_STATUS USER_FUNC getBuzzerStatus(void)
{
	return g_buzzer_status;
}
#endif

#endif

