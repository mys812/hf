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




static hftimer_handle_t g_alarmTimer[MAX_ALARM_COUNT];
static hftimer_handle_t g_absenceTimer[MAX_ABSENCE_COUNT];
static hftimer_handle_t g_countDownTimer[MAX_COUNTDOWN_COUNT];




static void USER_FUNC TimerDataInit(void)
{
	U8 i;

	for(i=0; i<MAX_ALARM_COUNT; i++)
	{
		g_alarmTimer[i] = NULL;
	}
	
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
static S32 USER_FUNC checkTimeBefor(U8 hour, U8 minute, TIME_DATA_INFO* pCurTime)
{
	S16 curMinute;
	S16 checkMinute;
	S16 tmpMinute;
	S32 ret = -1;

	curMinute = pCurTime->hour*60 + pCurTime->minute;
	checkMinute = hour*60 + minute;
	if(checkMinute >= curMinute)
	{
		tmpMinute = checkMinute - curMinute;
	}
	else
	{
		tmpMinute = checkMinute + 24*60 - curMinute;
	}
	if(tmpMinute >= 0 && tmpMinute <= START_TIMER_INTERVAL)
	{
		ret = tmpMinute;
	}
	//lumi_debug("cur time: %d:%d  check time %d:%d  ret=%d\n", pCurTime->hour, pCurTime->minute, hour, minute, ret);
	return ret;
}


static BOOL USER_FUNC compareWeekData(U8 alarmWeek, U8 curWeek)
{
	U8 tem;
	BOOL ret = FALSE;

	tem = ((alarmWeek&0x3F)<<1)|((alarmWeek&0x40)>>6);

	//if((tem&curWeek) > 0)
	if((tem & (1<<curWeek)) != 0)
	{
		ret = TRUE;
	}
	//lumi_debug("alarmWeek=0x%x, tem=0x%x ret=%d\n", alarmWeek, tem, ret);
	return ret;
}


static void USER_FUNC checkInactiveAlarm(U8 index)
{
	ALARM_DATA_INFO* pAlarmInfo;
	ALARM_DATA_INFO tmpAlarmInfo;
	U8 tmp;

	
	pAlarmInfo = getAlarmData(index);
	tmp = *((U8*)(&pAlarmInfo->repeatData));
	if((tmp&0x7F) == 0)
	{
		pAlarmInfo->repeatData.bActive= (U8)EVENT_INCATIVE;
		memcpy(&tmpAlarmInfo, pAlarmInfo, sizeof(ALARM_DATA_INFO));
		setAlarmData(&tmpAlarmInfo, index);
	}
}


static void USER_FUNC alarmTimerCallback( hftimer_handle_t htimer )
{
	U32 timerId;
	U8 index;
	ALARM_DATA_INFO* pAlarmInfo;
	U8 sendData[3] = {0};

	timerId = hftimer_get_timer_id(htimer);
	lumi_debug("%s, index=%d\n", __FUNCTION__, (timerId-ALARM_TIMER_ID_BEGIN));
	if(timerId >= ALARM_TIMER_ID_BEGIN)
	{
		index = (U8)(timerId - ALARM_TIMER_ID_BEGIN);
		hftimer_delete(htimer);
		g_alarmTimer[index] = NULL;

		pAlarmInfo = getAlarmData(index);

		sendData[0] = pAlarmInfo->action;
		sendData[1] = index;
		
		insertLocalMsgToList(MSG_LOCAL_EVENT, sendData, 2, MSG_CMD_LOCAL_ALARM_EVENT);
		checkInactiveAlarm(index);
	}
}


static void USER_FUNC compareAlarmTime(U8 index, TIME_DATA_INFO* pCurTime)
{
	ALARM_DATA_INFO* pAlarmInfo;
	S32 timeInterval;
	U8 tmp;

	pAlarmInfo = getAlarmData(index);
	tmp = *((U8*)(&pAlarmInfo->repeatData));
	//lumi_debug("index=%d alarm %d:%d-0x%x  pCurTime=%d:%d-%d g_alarmTimer[%d]=%d\n", index,  pAlarmInfo->hourData, pAlarmInfo->minuteData, tmp,
	//	pCurTime->hour, pCurTime->minute, pCurTime->week, index, (g_alarmTimer[index]!=NULL));
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
	timeInterval = checkTimeBefor(pAlarmInfo->hourData, pAlarmInfo->minuteData, pCurTime);
	lumi_debug("timeInterval=%d, g_alarmTimer[%d]=%d\n", timeInterval, index, (g_alarmTimer[index]!=NULL));
	if(timeInterval > 0)
	{
		timeInterval *= 60000; //n*60*1000 (minute to ms)
		if(g_alarmTimer[index] == NULL)
		{
			g_alarmTimer[index] = hftimer_create("Alarm Timer",timeInterval, false, (ALARM_TIMER_ID_BEGIN + index), alarmTimerCallback, 0);
			//hftimer_start(g_alarmTimer[index]);
			hftimer_change_period(g_alarmTimer[index], timeInterval);
		}
	}
}


static void USER_FUNC checkAlarmEvent(void)
{
	TIME_DATA_INFO timeInfo;
	U8 i;


	getLocalTime(&timeInfo);
	for(i=0; i<MAX_ALARM_COUNT; i++)
	{
		compareAlarmTime(i, &timeInfo);
	}
}


void USER_FUNC deviceAlarmArrived(U8 action, U8 alarmIndex)
{
	SWITCH_ACTION switchAction;

	switchAction = (action == 1)?SWITCH_OPEN:SWITCH_CLOSE;
	setSwitchStatus(switchAction);
	insertLocalMsgToList(MSG_LOCAL_EVENT, &alarmIndex, 1, MSG_CMD_REPORT_ALARM_CHANGE);
}




//Asbence 

static BOOL USER_FUNC checkTimeBetween(ASBENCE_DATA_INFO* pAbenceInfo, TIME_DATA_INFO* pCurTime)
{
	U8 endHour;
	U8 curHour;
	S32 startMinute;
	S32 endMinute;
	S32 curMinute;

	endHour = pAbenceInfo->endHour;
	curHour = pCurTime->hour;
	if(pAbenceInfo->startHour > pAbenceInfo->endHour)
	{
		endHour += 24;
		if(pCurTime->hour <=  pAbenceInfo->endHour)
		{
			curHour += 24;
		}
	}
	startMinute = pAbenceInfo->startHour*60 + pAbenceInfo->startMinute;
	endMinute = endHour*60 + pAbenceInfo->endMinute;
	curMinute = curHour*60 + pCurTime->minute;

	if(curMinute >= startMinute && startMinute < endMinute)
	{
		return TRUE;
	}
	return FALSE;
}


static void USER_FUNC absenceTimerCallback( hftimer_handle_t htimer )
{
	U32 timerId;
	U8 index;

	timerId = hftimer_get_timer_id(htimer);
	lumi_debug("%s, index=%d\n", __FUNCTION__, (timerId-ABSENCE_TIMER_ID_BEGIN));
	if(timerId >= ABSENCE_TIMER_ID_BEGIN)
	{
		index = (U8)(timerId - ABSENCE_TIMER_ID_BEGIN);
		insertLocalMsgToList(MSG_LOCAL_EVENT, &index, 1, MSG_CMD_LOCAL_ABSENCE_EVENT);
	}
}



static void USER_FUNC compareAbsenceTime(U8 index, TIME_DATA_INFO* pCurTime)
{
	ASBENCE_DATA_INFO* pAbenceInfo;
	S32 timeInterval;
	U8 tmp;
	BOOL createTimer = FALSE;


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

	timeInterval = checkTimeBefor(pAbenceInfo->startHour, pAbenceInfo->startMinute, pCurTime);
	lumi_debug("timeInterval=%d, g_absenceTimer[%d]=%d\n", timeInterval, index, (g_absenceTimer[index]!=NULL));
	if(timeInterval > 0)
	{
		timeInterval *= 60000; //n*60*1000 (minute to ms)
		if(g_absenceTimer[index] == NULL)
		{
			createTimer = TRUE;
		}
	}
	else if(g_absenceTimer[index] == NULL)
	{
		if(checkTimeBetween(pAbenceInfo, pCurTime))
		{
			timeInterval = 1000;
			createTimer = TRUE;
		}
	}

	if(createTimer)
	{
		g_absenceTimer[index] = hftimer_create("Absence Timer",timeInterval, false, (ABSENCE_TIMER_ID_BEGIN + index), absenceTimerCallback, 0);
		//hftimer_start(g_absenceTimer[index]);
		hftimer_change_period(g_absenceTimer[index], timeInterval);
	}
}



static void USER_FUNC checkAbsenceEvent(void)
{
	TIME_DATA_INFO timeInfo;
	U8 i;


	getLocalTime(&timeInfo);
	for(i=0; i<MAX_ABSENCE_COUNT; i++)
	{
		compareAbsenceTime(i, &timeInfo);
	}
}


static S32 USER_FUNC getAbsenceTimerPeriod(U8 index, SWITCH_ACTION* action)
{
	U8 endHour;
	U8 curHour;
	S32 startMinute;
	S32 endMinute;
	S32 curMinute;
	S32 timerPeriod;
	BOOL bFirstTime = FALSE;
	ASBENCE_DATA_INFO* pAbenceInfo;	
	TIME_DATA_INFO curTime;


	getLocalTime(&curTime);
	pAbenceInfo =  getAbsenceData(index);
	endHour = pAbenceInfo->endHour;
	curHour = curTime.hour;
	if(pAbenceInfo->startHour > pAbenceInfo->endHour)
	{
		endHour += 24;
		if(curTime.hour <=  pAbenceInfo->endHour)
		{
			curHour += 24;
		}
	}
	startMinute = pAbenceInfo->startHour*60 + pAbenceInfo->startMinute;
	endMinute = endHour*60 + pAbenceInfo->endMinute;
	curMinute = curHour*60 + curTime.minute;


	if((curMinute - startMinute) < START_TIMER_INTERVAL)
	{
		bFirstTime = TRUE;
	}
	if(bFirstTime || !getSwitchStatus())
	{
		*action = SWITCH_OPEN;
	}
	else
	{
		*action = SWITCH_CLOSE;
	}
	if(pAbenceInfo->timeData != 0)
	{
		timerPeriod = pAbenceInfo->timeData;
	}
	else
	{
		if(*action == SWITCH_OPEN)
		{
			timerPeriod = getRandomNumber(MIN_ABSENCE_OPEN_INTERVAL, MAX_ABSENCE_OPEN_INTERVAL);
		}
		else
		{
			timerPeriod = getRandomNumber(MIN_ABSENCE_CLOSE_INTERVAL, MAX_ABSENCE_CLOSE_INTERVAL);
		}	
	}
	if(curMinute >= endMinute)
	{
		timerPeriod = -1;
		*action = SWITCH_CLOSE;
	}
	else
	{
		if((curMinute + timerPeriod) > endMinute)
		{
			timerPeriod = endMinute - curMinute;
		}
		//timerPeriod *= 60000;
		timerPeriod *= 1000;
	}
	lumi_debug("Ramdom num=%d curMinute=%d endMinute=%d index=%d\n", timerPeriod, curMinute, endMinute, index);
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


void USER_FUNC deviceAbsenceArrived(U8 index)
{
	S32 timerPeriod;
	SWITCH_ACTION action;


	timerPeriod = getAbsenceTimerPeriod(index, &action);

	if(timerPeriod < 0)
	{
		hftimer_delete(g_absenceTimer[index]);
		g_absenceTimer[index] = NULL;
		checkInactiveAbsence(index);
	}
	else
	{
		hftimer_change_period(g_absenceTimer[index], timerPeriod);
		//hftimer_start(g_absenceTimer[index]);
	}
	setSwitchStatus(action);
}




// CountDown
static void USER_FUNC countDownTimerCallback( hftimer_handle_t htimer )
{
	U32 timerId;
	U8 index;
	COUNTDOWN_DATA_INFO* pCountDownInfo;
	COUNTDOWN_DATA_INFO tmpCountDownInfo;

	timerId = hftimer_get_timer_id(htimer);
	lumi_debug("%s, index=%d\n", __FUNCTION__, (timerId-COUNTDOWN_TIMER_ID_BEGIN));
	if(timerId >= COUNTDOWN_TIMER_ID_BEGIN)
	{
		index = (U8)(timerId - COUNTDOWN_TIMER_ID_BEGIN);
		hftimer_delete(htimer);
		g_countDownTimer[index] = NULL;
		
		pCountDownInfo = getCountDownData(index);
		insertLocalMsgToList(MSG_LOCAL_EVENT, &pCountDownInfo->action, 1, MSG_CMD_LOCAL_COUNTDOWN_EVENT);
		pCountDownInfo->flag.bActive = EVENT_INCATIVE;

		memcpy(&tmpCountDownInfo, pCountDownInfo, sizeof(COUNTDOWN_DATA_INFO));
		setCountDownData(&tmpCountDownInfo, index);
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
	if(timeInterval > 0 && timeInterval <= START_TIMER_INTERVAL*60)
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


static void USER_FUNC checkCountDownEvent(void)
{
	time_t curTime;
	U8 i;


	curTime = time(NULL);
	for(i=0; i<MAX_ABSENCE_COUNT; i++)
	{
		compareCountDownTime(i, curTime);
	}
}



void USER_FUNC deviceCountDownArrived(U8 action)
{
	setSwitchStatus((SWITCH_ACTION)action);
}


void USER_FUNC checkAlarmTimerAfterChange(U8 index)
{
	if(g_alarmTimer[index] != NULL)
	{
		hftimer_delete(g_alarmTimer[index]);
		g_alarmTimer[index] = NULL;
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



void USER_FUNC deviceTimeThread(void)
{
	TimerDataInit();
	hfthread_enable_softwatchdog(NULL, (MAX_TIME_THREAD_SLEEP + 30)); //Start watchDog
    while(1)
    {
        //u_printf(" deviceClientEventThread \n");
        hfthread_reset_softwatchdog(NULL); //tick watchDog

		checkAlarmEvent();
		checkAbsenceEvent();
		checkCountDownEvent();
        msleep(MAX_TIME_THREAD_SLEEP*1000); //2minute = 2*60*1000
    }
}



#endif

