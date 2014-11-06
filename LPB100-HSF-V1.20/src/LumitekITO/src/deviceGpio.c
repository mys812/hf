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



void USER_FUNC deviceEnterSmartLink(void)
{
	char rsp[64]= {0};

	hfat_send_cmd("AT+SMTLK\r\n",sizeof("AT+SMTLK\r\n"),rsp,64);
	u_printf("Device go into SmartLink status\n");

}



static void USER_FUNC smartLinkKeyIrq(U32 arg1,U32 arg2)
{
	static time_t g_key_pressdown_time;
	time_t now = time(NULL);

	if(hfgpio_fpin_is_high(HFGPIO_F_SMARTLINK)) //key up
	{
		if((now - g_key_pressdown_time) >= 3)
		{
			deviceEnterSmartLink();
		}
		else
		{
			if(hfgpio_fpin_is_high(HFGPIO_F_LIGHT))
			{
				hfgpio_fset_out_low(HFGPIO_F_LIGHT);
			}
			else
			{
				hfgpio_fset_out_high(HFGPIO_F_LIGHT);
			}
		}
	}
	else //key down
	{
		g_key_pressdown_time = now;
	}
}



void USER_FUNC smartlinkTimerCallback( hftimer_handle_t htimer )
{

	if(hftimer_get_timer_id(htimer)==SMARTLINK_TIMER_ID)
	{
		if(hfgpio_fpin_is_high(HFGPIO_F_LIGHT))
		{
			hfgpio_fset_out_low(HFGPIO_F_LIGHT);
		}
		else
		{
			hfgpio_fset_out_high(HFGPIO_F_LIGHT);
		}
	}
}



void USER_FUNC keyGpioInit(void)
{
	if(hfgpio_configure_fpin_interrupt(HFGPIO_F_SMARTLINK, HFPIO_IT_EDGE, smartLinkKeyIrq, 1)!= HF_SUCCESS)
	{
		u_printf("configure HFGPIO_F_SMARTLINK fail\n");
		return;
	}
}


void USER_FUNC setSwitchStatus(SWITCH_ACTION action)
{
	if(SWITCH_OPEN == action)
	{
		hfgpio_fset_out_low(HFGPIO_F_SWITCH);
	}
	else
	{
		hfgpio_fset_out_high(HFGPIO_F_SWITCH);
	}
}


SWITCH_ACTION USER_FUNC getSwitchStatus(void)
{
	if(hfgpio_fpin_is_high(HFGPIO_F_SWITCH))
	{
		return SWITCH_CLOSE;
	}
	return SWITCH_OPEN;
}


#endif
