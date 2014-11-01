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
#include "../inc/asyncMessage.h"
#include "../inc/messageDispose.h"
#include "../inc/deviceGpio.h"


void USER_FUNC deviceClientEventThread(void)
{
	time_t curTime;


	hfthread_enable_softwatchdog(NULL, 30); //Start watchDog
    while(1)
    {
		curTime = time(NULL);

        //u_printf(" deviceClientEventThread \n");
        hfthread_reset_softwatchdog(NULL); //tick watchDog

		
        if(checkHeartBeatTime(curTime))
    	{
    		startSendHeartBeat();
    	}
        msleep(5000);
    }
}

#endif
