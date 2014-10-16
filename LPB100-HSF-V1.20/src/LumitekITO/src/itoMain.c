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
#include "../inc/localClientEvent.h"
#include "../inc/localSocketUdp.h"
#include "../inc/serverSocketTcp.h"
#include "../inc/asyncMessage.h"
#include "../inc/serverSocketTcp.h"
#include "../inc/deviceGpio.h"






#define CLIENT_EVENT_THREAD_DEPTH		256
#define LOCAL_UDP_THREAD_DEPTH			256
#define SERVER_TCP_THREAD_DEPTH			256
#define MESSAGE_THREAD_DEPTH			420




static BOOL USER_FUNC checkSmartlinkStatus(void)
{
    S32	start_reason = hfsys_get_reset_reason();
    BOOL ret = FALSE;


    if(start_reason&HFSYS_RESET_REASON_SMARTLINK_START)
    {
        hftimer_handle_t smartlinkTimer;
        if((smartlinkTimer = hftimer_create("SMARTLINK_TIMER", 300, true, SMARTLINK_TIMER_ID, smartlinkTimerCallback, 0)) == NULL)
        {

            u_printf("create smartlinkTimer fail\n");
        }
        else
        {
            hftimer_start(smartlinkTimer);
            u_printf("meiyusong===> go into SmartLink time = %d\n", time(NULL));
        }
        ret = TRUE;
    }

    return ret;
}



void USER_FUNC lumitekITOMain(void)
{
    u_printf("meiyusong===> start time = %d\n", time(NULL));
    if(!checkSmartlinkStatus())
    {
        itoParaInit();

        if(hfthread_create((PHFTHREAD_START_ROUTINE)deviceClientEventThread, "IOT_TD_C", CLIENT_EVENT_THREAD_DEPTH,
                           NULL, HFTHREAD_PRIORITIES_LOW,NULL,NULL)!= HF_SUCCESS)
        {
            HF_Debug(DEBUG_ERROR, "Create IOT_TD_C thread failed!\n");
        }

        if(hfthread_create((PHFTHREAD_START_ROUTINE)deviceLocalUdpThread, "IOT_TD_L",LOCAL_UDP_THREAD_DEPTH,
                           NULL, HFTHREAD_PRIORITIES_MID,NULL,NULL)!= HF_SUCCESS)
        {
            HF_Debug(DEBUG_ERROR, "Create IOT_TD_L thread failed!\n");
        }

        if(hfthread_create((PHFTHREAD_START_ROUTINE)deviceServerTcpThread, "IOT_TD_S", SERVER_TCP_THREAD_DEPTH,
                           NULL, HFTHREAD_PRIORITIES_MID,NULL,NULL)!= HF_SUCCESS)
        {
            HF_Debug(DEBUG_ERROR, "Create IOT_TD_S thread failed!\n");
        }
        if(hfthread_create((PHFTHREAD_START_ROUTINE)deviceMessageThread, "IOT_TD_M", MESSAGE_THREAD_DEPTH,
                           NULL, HFTHREAD_PRIORITIES_LOW,NULL,NULL)!= HF_SUCCESS)
        {
            HF_Debug(DEBUG_ERROR, "Create IOT_TD_M thread failed!\n");
        }
    }

}

#endif

