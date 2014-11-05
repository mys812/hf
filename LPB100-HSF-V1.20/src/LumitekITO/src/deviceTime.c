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


#if 0
int tm_sec; /* 秒–取值区间为[0,59] */
int tm_min; /* 分 - 取值区间为[0,59] */
int tm_hour; /* 时 - 取值区间为[0,23] */
int tm_mday; /* 一个月中的日期 - 取值区间为[1,31] */
int tm_mon; /* 月份（从一月开始，0代表一月） - 取值区间为[0,11] */
int tm_year; /* 年份，其值从1900开始 */
int tm_wday; /* 星期–取值区间为[0,6]，其中0代表星期天，1代表星期一，以此类推 */
int tm_yday; /* 从每年的1月1日开始的天数–取值区间为[0,365]，其中0代表1月1日，1代表1月2日，以此类推 */
int tm_isdst; /* 夏令时标识符，实行夏令时的时候，tm_isdst为正。不实行夏令时的进候，tm_isdst为0；不了解情况时，tm_isdst()为负。*/
long int tm_gmtoff; /*指定了日期变更线东面时区中UTC东部时区正秒数或UTC西部时区的负秒数*/
const char *tm_zone; /*当前时区的名字(与环境变量TZ有关)*/

#endif
static void USER_FUNC getLocalTime(TIME_DATA_INFO *pTimeInfo)
{
	time_t curTime;
	struct tm *p_tm;


	curTime = time(NULL);
	p_tm = localtime(&curTime);

	memset(pTimeInfo, 0, sizeof(TIME_DATA_INFO));
	if(p_tm != NULL)
	{
		pTimeInfo->year = p_tm->tm_year + 1900;
		pTimeInfo->month = p_tm->tm_mon;
		pTimeInfo->day = p_tm->tm_mday;
		pTimeInfo->week = p_tm->tm_wday;
		pTimeInfo->hour = p_tm->tm_hour;
		pTimeInfo->minute = p_tm->tm_min;
		pTimeInfo->second = p_tm->tm_sec;
		pTimeInfo->dayCount = p_tm->tm_yday;
		lumi_debug("Cur Time= %d-%d-%d (%d) %d:%d:%d  (days=%d)\n",
			pTimeInfo->year,
			pTimeInfo->month,
			pTimeInfo->day,
			pTimeInfo->week,
			pTimeInfo->hour,
			pTimeInfo->minute,
			pTimeInfo->second,
			pTimeInfo->dayCount);
	}
}


static BOOL compareWeekData(U8 alarmWeek, U8 curWeek)
{
	U8 tem;
	BOOl ret = FALSE;

	tem = ((alarmWeek&0x3F)>1)|((alarmWeek&0x4F)>>6);

	if((tem&curWeek) > 0)
	{
		ret = TRUE;
	}
	lumi_debug("alarmWeek=0x%x, tem=0x%x ret=%d\n", alarmWeek, tem, ret);
	return ret;
}


static BOOL USER_FUNC compareAlarmTime(ALARM_DATA_INFO* pAlarmInfo, TIME_DATA_INFO* pCurTime)
{
	if(pAlarmInfo->repeatData.bActive == EVENT_INCATIVE)
	{
		return FALSE;
	}
	if((pAlarmInfo->repeatData&0x7F) > 0)
	{
		if(!compareWeekData(pAlarmInfo->repeatData, pCurTime->week)
		{
			return FALSE;
		}
	}
	if(pAlarmInfo->hourData != pCurTime->hour)
	{
		return FALSE;
	}
	if(pAlarmInfo->minuteData != pCurTime->minute)
	{
		return FALSE;
	}
	return TRUE;
}


static BOOL USER_FUNC checkAlarmEvent(TIME_DATA_INFO* pTimeInfo, SWITCH_ACTION* action)
{
	ALARM_DATA_INFO* pAlarmInfo;
	TIME_DATA_INFO timeInfo;
	U8 i;
	BOOL ret = FALSE;


	getLocalTime(&timeInfo);
	for(i=0; i<MAX_ALARM_COUNT; i++)
	{
		pAlarmInfo = getAlarmData(i);
		if(compareAlarmTime(pAlarmInfo, &timeInfo))
		{
			ret = TRUE;
			*action = pAlarmInfo->action;
			break;
		}
	}
	return TRUE;
}

#endif

