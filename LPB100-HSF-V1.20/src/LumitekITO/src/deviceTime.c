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
#include "../inc/asyncMessage.h"
#include "../inc/messageDispose.h"
#include "../inc/deviceGpio.h"



static hftimer_handle_t g_absenceTimer[MAX_ABSENCE_COUNT];
static hftimer_handle_t g_countDownTimer[MAX_COUNTDOWN_COUNT];



static void USER_FUNC TimerDataInit(void)
{
	U8 i;

	for(i=0; i<MAX_ABSENCE_COUNT; i++)
	{
		g_absenceTimer[i] = NULL;
	}
	
	for(i=0; i<MAX_COUNTDOWN_COUNT; i++)
	{
		g_countDownTimer[i] = NULL;
	}
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
#if 0
		lumi_debug("Cur Time= %d-%d-%d (%d) %d:%d:%d  (days=%d)\n",
			pTimeInfo->year,
			pTimeInfo->month,
			pTimeInfo->day,
			pTimeInfo->week,
			pTimeInfo->hour,
			pTimeInfo->minute,
			pTimeInfo->second,
			pTimeInfo->dayCount);
#endif
	}
}


// Alarm

static BOOL USER_FUNC compareWeekData(U8 alarmWeek, U8 curWeek)
{
	U8 tem;
	BOOL ret = FALSE;

	tem = ((alarmWeek&0x3F)<<1)|((alarmWeek&0x40)>>6);


	if((tem & (1<<curWeek)) != 0)
	{
		ret = TRUE;
	}
	return ret;
}


static void USER_FUNC compareAlarmTime(U8 index, TIME_DATA_INFO* pCurTime, U16 curMinute)
{
	ALARM_DATA_INFO* pAlarmInfo;
	U8 tmp;
	U16 checkMinute;

	pAlarmInfo = getAlarmData(index);
	tmp = *((U8*)(&pAlarmInfo->repeatData));
	if(pAlarmInfo->repeatData.bActive == EVENT_INCATIVE)
	{
		return;
	}
	if((tmp&0x7F) > 0)
	{
		if(!compareWeekData(tmp, pCurTime->week))
		{
			return;
		}
	}

	checkMinute = pAlarmInfo->hourData*60 + pAlarmInfo->minuteData;

	if(curMinute == checkMinute) // Alarm arrived
	{
		SWITCH_STATUS switchAction;

		switchAction = (pAlarmInfo->action == 1)?SWITCH_OPEN:SWITCH_CLOSE;
		setSwitchStatus(switchAction);
		insertLocalMsgToList(MSG_LOCAL_EVENT, &index, 1, MSG_CMD_REPORT_ALARM_CHANGE);
		lumi_debug("alarm come index=%d, action=%d\n", index, pAlarmInfo->action);
		
		if((tmp&0x7F) == 0)
		{
			ALARM_DATA_INFO tmpAlarmInfo;
			
			pAlarmInfo->repeatData.bActive= (U8)EVENT_INCATIVE;
			memcpy(&tmpAlarmInfo, pAlarmInfo, sizeof(ALARM_DATA_INFO));
			setAlarmData(&tmpAlarmInfo, index);
		}
	}
}


static void USER_FUNC checkAlarmEvent(TIME_DATA_INFO* pCurTime, U16 curMinute)
{
	U8 i;


	for(i=0; i<MAX_ALARM_COUNT; i++)
	{
		compareAlarmTime(i, pCurTime, curMinute);
	}
}



//Asbence 

static S32 USER_FUNC getAbsenceTimerPeriod(U8 index)
{
	U8 endHour;
	U8 curHour;
	S32 endMinute;
	S32 curMinute;
	S32 timerPeriod;
	SWITCH_STATUS switchStatus;
	ASBENCE_DATA_INFO* pAbenceInfo; 
	TIME_DATA_INFO curTime;


	getLocalTime(&curTime);
	pAbenceInfo =  getAbsenceData(index);
	endHour = pAbenceInfo->endHour;
	curHour = curTime.hour;
	if(pAbenceInfo->startHour > pAbenceInfo->endHour)
	{
		endHour += 24;
		if(curTime.hour <=	pAbenceInfo->endHour)
		{
			curHour += 24;
		}
	}
	endMinute = endHour*60 + pAbenceInfo->endMinute;
	curMinute = curHour*60 + curTime.minute;


	if(curMinute < endMinute)
	{
		switchStatus = getSwitchStatus();
		if(switchStatus == SWITCH_OPEN)
		{
			timerPeriod = getRandomNumber(MIN_ABSENCE_OPEN_INTERVAL, MAX_ABSENCE_OPEN_INTERVAL);
		}
		else
		{
			timerPeriod = getRandomNumber(MIN_ABSENCE_CLOSE_INTERVAL, MAX_ABSENCE_CLOSE_INTERVAL);
		}
		if((curMinute + timerPeriod) > endMinute)
		{
			timerPeriod = endMinute - curMinute;
		}
	}
	else
	{
		timerPeriod = -1;
	}
	timerPeriod *= 60000; //minute -->ms
	//timerPeriod *= 1000; //minute -->ms
	lumi_debug("absence interval=%d index=%d\n", timerPeriod, index);
	return timerPeriod;
	
}


static void USER_FUNC checkInactiveAbsence(U8 index)
{
	ASBENCE_DATA_INFO* pAbenceInfo;
	ASBENCE_DATA_INFO tmpAbenceInfo;
	U8 tmp;

	
	pAbenceInfo = getAbsenceData(index);
	tmp = *((U8*)(&pAbenceInfo->repeatData));
	if((tmp&0x7F) == 0)
	{
		pAbenceInfo->repeatData.bActive = (U8)EVENT_INCATIVE;
		memcpy(&tmpAbenceInfo, pAbenceInfo, sizeof(ASBENCE_DATA_INFO));
		setAbsenceData(&tmpAbenceInfo, index);
	}
}


static void USER_FUNC absenceTimerCallback( hftimer_handle_t htimer )
{
	U32 timerId;
	U8 index;
	S32 timerPeriod;

	timerId = hftimer_get_timer_id(htimer);
	lumi_debug("%s, index=%d\n", __FUNCTION__, (timerId-ABSENCE_TIMER_ID_BEGIN));
	if(timerId >= ABSENCE_TIMER_ID_BEGIN)
	{
		index = (U8)(timerId - ABSENCE_TIMER_ID_BEGIN);
		timerPeriod = getAbsenceTimerPeriod(index);

		if(timerPeriod > 0)
		{
			changeSwitchStatus();
			lumi_debug("absebce come index=%d\n", index);
			hftimer_change_period(g_absenceTimer[index], timerPeriod);
		}
		else
		{
			setSwitchStatus(SWITCH_CLOSE);
			hftimer_delete(g_absenceTimer[index]);
			g_absenceTimer[index] = NULL;
			checkInactiveAbsence(index);
		}
	}
}


static void USER_FUNC compareAbsenceTime(U8 index, TIME_DATA_INFO* pCurTime, U16 curMinute)
{
	ASBENCE_DATA_INFO* pAbenceInfo;
	S32 timeInterval;
	U8 tmp;
	U16 checkMunite;


	pAbenceInfo = getAbsenceData(index);
	tmp = *((U8*)(&pAbenceInfo->repeatData));
	if(pAbenceInfo->repeatData.bActive == EVENT_INCATIVE)
	{
		return;
	}
	if((tmp&0x7F) > 0)
	{
		if(!compareWeekData(tmp, pCurTime->week))
		{
			return;
		}
	}
	
	checkMunite = pAbenceInfo->startHour*60 + pAbenceInfo->startMinute;
	if(curMinute >= checkMunite && g_absenceTimer[index] == NULL)
	{
		setSwitchStatus(SWITCH_OPEN);
		timeInterval = getAbsenceTimerPeriod(index);

		g_absenceTimer[index] = hftimer_create("Absence Timer",timeInterval, false, (ABSENCE_TIMER_ID_BEGIN + index), absenceTimerCallback, 0);
		hftimer_change_period(g_absenceTimer[index], timeInterval);
	}
}



static void USER_FUNC checkAbsenceEvent(TIME_DATA_INFO* pCurTime, U16 curMinute)
{
	U8 i;


	for(i=0; i<MAX_ABSENCE_COUNT; i++)
	{
		compareAbsenceTime(i, pCurTime, curMinute);
	}
}




// CountDown
static void USER_FUNC inactiveCountDown(U8 index)
{
	COUNTDOWN_DATA_INFO* pCountDownInfo;
	COUNTDOWN_DATA_INFO tmpCountDownInfo;

	pCountDownInfo = getCountDownData(index);
	memcpy(&tmpCountDownInfo, pCountDownInfo, sizeof(COUNTDOWN_DATA_INFO));
	tmpCountDownInfo.flag.bActive = EVENT_INCATIVE;
	setCountDownData(&tmpCountDownInfo, index);
}


static void USER_FUNC countDownTimerCallback( hftimer_handle_t htimer )
{
	U32 timerId;
	U8 index;
	COUNTDOWN_DATA_INFO* pCountDownInfo;
	SWITCH_STATUS switchAction;
	

	timerId = hftimer_get_timer_id(htimer);
	lumi_debug("%s, index=%d\n", __FUNCTION__, (timerId-COUNTDOWN_TIMER_ID_BEGIN));
	if(timerId >= COUNTDOWN_TIMER_ID_BEGIN)
	{
		index = (U8)(timerId - COUNTDOWN_TIMER_ID_BEGIN);
		hftimer_delete(htimer);
		g_countDownTimer[index] = NULL;
		
		pCountDownInfo = getCountDownData(index);
		switchAction = (pCountDownInfo->action == 1)?SWITCH_OPEN:SWITCH_CLOSE;
		setSwitchStatus(switchAction);
		inactiveCountDown(index);
	}
}



static void USER_FUNC compareCountDownTime(U8 index, time_t curTime)
{
	COUNTDOWN_DATA_INFO* pCountDownInfo;
	S32 timeInterval;


	pCountDownInfo = getCountDownData(index);
	if(pCountDownInfo->flag.bActive == EVENT_INCATIVE)
	{
		return;
	}

	timeInterval = pCountDownInfo->count - curTime;

	lumi_debug("timeInterval=%d count=%d, curTime= %d index=%d\n", timeInterval, pCountDownInfo->count, curTime, index);
	if(timeInterval > 0)
	{
		if(timeInterval <= START_TIMER_INTERVAL*60)
		{
			timeInterval *= 1000; //n*1000 (minute to ms)
			if(g_countDownTimer[index] == NULL)
			{
				g_countDownTimer[index] = hftimer_create("CountDown Timer",timeInterval, false, (COUNTDOWN_TIMER_ID_BEGIN + index), countDownTimerCallback, 0);
				//hftimer_start(g_countDownTimer[index]);
				hftimer_change_period(g_countDownTimer[index], timeInterval);
			}
		}
	}
	else
	{
		inactiveCountDown(index);
	}
}


static void USER_FUNC checkCountDownEvent(void)
{
	U8 i;
	time_t curTime;


	curTime = time(NULL);
	for(i=0; i<MAX_ABSENCE_COUNT; i++)
	{
		compareCountDownTime(i, curTime);
	}
}


void USER_FUNC checkAbsenceTimerAfterChange(U8 index)
{
	if(g_absenceTimer[index] != NULL)
	{
		hftimer_delete(g_absenceTimer[index]);
		g_absenceTimer[index] = NULL;
	}
}


void USER_FUNC checkCountDownTimerAfterChange(U8 index)
{
	if(g_countDownTimer[index] != NULL)
	{
		hftimer_delete(g_countDownTimer[index]);
		g_countDownTimer[index] = NULL;
	}
}


static void USER_FUNC checkTimeThread(TIME_DATA_INFO* pCurTime)
{
	U16 curMinute;


	if(pCurTime->year < 2014)
	{
		lumi_debug("Time not got\n");
		return;
	}
	curMinute = pCurTime->hour*60 + pCurTime->minute;

	checkAlarmEvent(pCurTime, curMinute);
	checkAbsenceEvent(pCurTime, curMinute);
	checkCountDownEvent();
}


#if 0
void USER_FUNC deviceTimeThread(void *arg)
{
	TIME_DATA_INFO timeInfo;
	U32 sleepTime;
	

	TimerDataInit();
	hfthread_enable_softwatchdog(NULL, 80); //Start watchDog
    while(1)
    {
        //u_printf(" deviceClientEventThread \n");
        hfthread_reset_softwatchdog(NULL); //tick watchDog

		checkTimeThread(&timeInfo);
		
		getLocalTime(&timeInfo);
		sleepTime = 60 - timeInfo.second + 10; //每分钟的第20秒开始执行
		lumi_debug("curMinute=%d second=%d sleepTime=%d\n", timeInfo.minute, timeInfo.second, sleepTime);
		sleepTime *= 1000; //s-->ms
        msleep(sleepTime);
    }
}

#else
static S32 USER_FUNC getCheckTimerPeriod(TIME_DATA_INFO* pCurTime)
{
	S32 period;
	
	period = (70 - pCurTime->second)*1000; //60 - timeInfo.second + 10;  每分钟的第10秒开始执行
	return period;
}

static void USER_FUNC timeCheckTimerCallback( hftimer_handle_t htimer )
{
	TIME_DATA_INFO timeInfo;
	S32 timerPeriod;

	getLocalTime(&timeInfo);
	checkTimeThread(&timeInfo);
	timerPeriod = getCheckTimerPeriod(&timeInfo); 
	hftimer_change_period(htimer, timerPeriod);
	lumi_debug("check Thread start minute=%d, second=%d sleepTime=%d\n", timeInfo.minute, timeInfo.second, timerPeriod);
}


void USER_FUNC initTimeCheck(void)
{
	hftimer_handle_t checkTimer;
	TIME_DATA_INFO timeInfo;
	S32 timerPeriod;


	TimerDataInit();
	getLocalTime(&timeInfo);
	//checkTimeThread(&timeInfo);
	timerPeriod = getCheckTimerPeriod(&timeInfo); 
	checkTimer = hftimer_create("Check Timer",timerPeriod, false, CHECK_TIME_TIMER_ID, timeCheckTimerCallback, 0);
	hftimer_change_period(checkTimer, timerPeriod);
}
#endif

#endif

