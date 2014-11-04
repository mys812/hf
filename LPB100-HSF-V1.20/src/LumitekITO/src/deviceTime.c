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
#include "../inc/deviceTime.h"


void USER_FUNC test2(void)
{
    char *wday[] = {"星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"};
    time_t timep;
    struct tm *p_tm;
    timep = time(NULL);
    p_tm = localtime(&timep); /*获取本地时区时间*/
    lumi_debug("%d-%d-%d %s %d:%d:%d\n", (p_tm->tm_year+1900), 
		(p_tm->tm_mon+1),
		p_tm->tm_mday,
		wday[p_tm->tm_wday],
		p_tm->tm_hour,
		p_tm->tm_min,
		p_tm->tm_sec); 
}


#endif

