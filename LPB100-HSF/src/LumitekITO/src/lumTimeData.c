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
#include "../inc/serverSocketTcp.h"
#include "../inc/lumTimeData.h"


static TIME_DATE_INFO g_timeDateInfo;




static const U32 LUM_SEC_PER_YR[2] = { 31536000, 31622400 };
static const U32 LUM_SEC_PER_MT[2][12] =
{
	{ 2678400, 2419200, 2678400, 2592000, 2678400, 2592000, 2678400, 2678400, 2592000, 2678400, 2592000, 2678400 },
	{ 2678400, 2505600, 2678400, 2592000, 2678400, 2592000, 2678400, 2678400, 2592000, 2678400, 2592000, 2678400 },
};
static const U8 LUM_WEEK_DAY_INFO[] =  {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
#define LUM_SEC_PER_DY		86400
#define LUM_SEC_PER_HR		3600



static inline U8 lum_bLeapYear(U16 yearData)
{
	if (!(yearData%100))
	{
		return (yearData%400 == 0) ? 1 : 0;
	}
	else
	{
		return (yearData%4 == 0) ? 1 : 0;
	}
}


static U8 USER_FUNC lum_getDayOfWeek(U8 month, U8 day, U16 year)
{
	/* Month should be a number 0 to 11, Day should be a number 1 to 31 */

	year -= month < 3;
	return (year + year/4 - year/100 + year/400 + LUM_WEEK_DAY_INFO[month-1] + day) % 7;
}


void USER_FUNC lum_gmtime(U32 second, TIME_DATA_INFO* timeInfo)
{
	U32 tmpSecond;
	U8 bLeepYear;


	tmpSecond = second;
	memset(timeInfo, 0, sizeof(TIME_DATA_INFO));

	timeInfo->year = 1970;
	while(1)
	{
		if(tmpSecond < LUM_SEC_PER_YR[lum_bLeapYear(timeInfo->year)])
		{
			break;
		}
		tmpSecond -= LUM_SEC_PER_YR[lum_bLeapYear(timeInfo->year)];
		timeInfo->year++;
	}

	bLeepYear = lum_bLeapYear(timeInfo->year);

	while(1)
	{
		if(tmpSecond < LUM_SEC_PER_MT[bLeepYear][timeInfo->month])
		{
			break;
		}
		tmpSecond -= LUM_SEC_PER_MT[bLeepYear][timeInfo->month];
		timeInfo->month++;
	}

	timeInfo->day = tmpSecond / LUM_SEC_PER_DY;
	timeInfo->day++;
	tmpSecond = tmpSecond % LUM_SEC_PER_DY;

	timeInfo->hour = tmpSecond / LUM_SEC_PER_HR;
	tmpSecond = tmpSecond % LUM_SEC_PER_HR;

	timeInfo->minute = tmpSecond / 60;
	timeInfo->second = tmpSecond % 60;

	timeInfo->week = lum_getDayOfWeek((timeInfo->month + 1), timeInfo->day, timeInfo->year);
}


static void USER_FUNC getUtcTimerCallback( hftimer_handle_t htimer )
{
	insertLocalMsgToList(MSG_LOCAL_EVENT, NULL, 0, MSG_CMD_LOCAL_GET_UTC_TIME);
}


static void USER_FUNC createGetUtcTimer(S32 period)
{
	static hftimer_handle_t getUtcTimer = NULL;

	
	if(getUtcTimer == NULL)
	{
		getUtcTimer = hftimer_create("Get_UTC_Time",period, false, GET_UTC_TIMER_ID, getUtcTimerCallback, 0);
	}
	hftimer_change_period(getUtcTimer, period);
}


static void USER_FUNC lum_syncNetworkTime(U32 networkTime)
{
	g_timeDateInfo.lastUTCTime = networkTime;
	g_timeDateInfo.lastSystemTime = hfsys_get_time();
}


void USER_FUNC getUtcTimeByMessage(void)
{
	BOOL getSucc = FALSE;
	U32 utcTime;
	U32 timerPeriod;



	if(getUtcTimeFromNetwork(&utcTime))
	{
		if(utcTime > DIFF_SEC_1900_1970)
		{				
			utcTime -= DIFF_SEC_1900_1970;
			lumi_debug("============>utcTime=%d, curtime=%d\n", utcTime, lum_getSystemTime());
			lum_syncNetworkTime(utcTime);
			getSucc = TRUE;
		}
	}
	if(getSucc)
	{
		timerPeriod = MAX_CALIBRATE_TIME_INTERVAL;
	}
	else
	{
		 timerPeriod = MAX_FAILD_CALIBRATE_TIME_INTERVAL;
	}
	createGetUtcTimer(timerPeriod);
}


void USER_FUNC lum_initSystemTime(void)
{
	g_timeDateInfo.lastSystemTime = hfsys_get_time();
	g_timeDateInfo.lastUTCTime = SEC_2014_01_01_00_00_00;
	createGetUtcTimer(MAX_FAILD_CALIBRATE_TIME_INTERVAL);
}


U32 USER_FUNC lum_getSystemTime(void)
{
	U32 curTimeMs;
	U32 totalSecond;
	U32 totalUs;


	curTimeMs = hfsys_get_time();
	if(curTimeMs < g_timeDateInfo.lastSystemTime)
	{
		totalUs = 0xFFFFFFFF - g_timeDateInfo.lastSystemTime + curTimeMs;
		totalSecond = totalUs/1000;  //ms-->S
		g_timeDateInfo.lastUTCTime += totalSecond;

		g_timeDateInfo.lastSystemTime = curTimeMs;
		createGetUtcTimer(MAX_FAILD_CALIBRATE_TIME_INTERVAL);
		return g_timeDateInfo.lastUTCTime;
	}
	else
	{
		totalUs = curTimeMs - g_timeDateInfo.lastSystemTime;
		totalSecond = totalUs/1000;  //ms-->S
		return g_timeDateInfo.lastUTCTime + totalSecond;
	}
}



void USER_FUNC lum_getGmtime(TIME_DATA_INFO* timeInfo)
{
	U32 curSecond;


	curSecond = lum_getSystemTime();
	lum_gmtime(curSecond, timeInfo);
}


void USER_FUNC lum_getStringTime(S8* timeData, BOOL needDay, BOOL chinaDate)
{
	U32 curTime;
	TIME_DATA_INFO timeInfo;


	curTime = lum_getSystemTime();
	lum_gmtime(curTime, &timeInfo);

	if(chinaDate)
	{
		timeInfo.hour = (timeInfo.hour + 8)%24;
	}

	if(needDay)
	{
		sprintf(timeData, "%04d-%02d-%02d %02d:%02d:%02d <%d>", timeInfo.year, timeInfo.month+1,
			timeInfo.day, timeInfo.hour, timeInfo.minute, timeInfo.second, timeInfo.week);
	}
	else
	{
		sprintf(timeData, "%02d:%02d:%02d", timeInfo.hour, timeInfo.minute, timeInfo.second);
	}
}


#endif

