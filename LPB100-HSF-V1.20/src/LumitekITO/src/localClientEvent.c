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


void USER_FUNC deviceClientEventThread(void)
{
    while(1)
    {

        //u_printf(" deviceClientEventThread \n");
        msleep(100);
    }
}

#endif
