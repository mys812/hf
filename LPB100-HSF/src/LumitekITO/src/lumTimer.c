/*
******************************
*Company:Lumlink
*Data:2015-02-07
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
#include "../inc/lumTimeData.h"
#include "../inc/lumTimer.h"



static BOOL g_absenceRunning = FALSE;
static U16	g_nextAbsenceMinute = INVALUE_ABSENCE_MINUTE;
static U8 g_lastCheckMinute = 0xFF;


static BOOL USER_FUNC lum_compareWeekData(U8 compareWeek, U8 curWeek)
{
	U8 tem;
	BOOL ret = FALSE;

	if((compareWeek&0x7F) == 0) // not repeat
	{
		ret = TRUE;
	}
	else
	{
		tem = ((compareWeek&0x3F)<<1)|((compareWeek&0x40)>>6);
		if((tem & (1<<curWeek)) != 0)
		{
			ret = TRUE;
		}
	}
	return ret;
}


static BOOL USER_FUNC lum_bAbsenceRunNow(void)
{
	return g_absenceRunning;
}


static void USER_FUNC lum_checkInactiveAlarm(U8 index, ALARM_DATA_INFO* pAlarmInfo)
{
	ALARM_DATA_INFO tmpAlarmInfo;

	memcpy(&tmpAlarmInfo, pAlarmInfo, sizeof(ALARM_DATA_INFO));
	tmpAlarmInfo.repeatData.bActive = (U8)EVENT_INCATIVE;
	setAlarmData(&tmpAlarmInfo, index);
}


static void USER_FUNC lum_compareAlarm(U8 index, TIME_DATA_INFO* pCurTime, U16 curMinute)
{
	ALARM_DATA_INFO* pAlarmInfo;
	U8 compareWeek;
	U16 checkStartMinute = 0;
	U16 checkStopMinute = 0;


	pAlarmInfo = getAlarmData(index);
	if(pAlarmInfo->repeatData.bActive == EVENT_INCATIVE)
	{
		return;
	}

	checkStartMinute = pAlarmInfo->startHour*60 + pAlarmInfo->startMinute;
	checkStopMinute = pAlarmInfo->stopHour*60 + pAlarmInfo->stopMinute;

	//compareWeek = *(U8*)(&pAlarmInfo->repeatData));
	memcpy(&compareWeek, &pAlarmInfo->repeatData, sizeof(ALARM_REPEAT_DATA));	
	lumi_debug("Alarm index=%d startHour=%d startMinute=%d stopHour=%d stopMinute=%d repeatData=0x%x\n", index, pAlarmInfo->startHour,
		pAlarmInfo->startMinute, pAlarmInfo->stopHour, pAlarmInfo->stopMinute, compareWeek);
	if(checkStartMinute == curMinute)
	{
		if(lum_compareWeekData(compareWeek, pCurTime->week))
		{
			if(!lum_bAbsenceRunNow())
			{
				setSwitchStatus(SWITCH_OPEN);
			}
			if((compareWeek&0x7F) == 0)
			{
				//unActive alarm
			}
		}
	}
	if(checkStopMinute == curMinute)
	{
		if(checkStopMinute <= checkStartMinute)
		{
			compareWeek = ((compareWeek&0x40)>>6) | ((compareWeek&0x3F)<<1) | (compareWeek&0x80);
		}
		if(lum_compareWeekData(compareWeek, pCurTime->week))
		{
			if(!lum_bAbsenceRunNow())
			{
				setSwitchStatus(SWITCH_CLOSE);
			}
			if((compareWeek&0x7F) == 0)
			{
				//unActive alarm
				lum_checkInactiveAlarm(index, pAlarmInfo);
			}
		}
	}
}


static void USER_FUNC lum_checkAlarm(TIME_DATA_INFO* pCurTime, U16 curMinute)
{
	U8 i;

	for(i=0; i<MAX_ALARM_COUNT; i++)
	{
		lum_compareAlarm(i, pCurTime, curMinute);
	}
}


static void USER_FUNC lum_checkInactiveAbsence(U8 index, ASBENCE_DATA_INFO* pAbenceInfo)
{
	U8 compareWeek;
	ASBENCE_DATA_INFO abenceInfo;


	memcpy(&compareWeek, &pAbenceInfo->repeatData, sizeof(ALARM_REPEAT_DATA));
	if((compareWeek&0x7F) == 0) // not repeat
	{
		memcpy(&abenceInfo, pAbenceInfo, sizeof(ASBENCE_DATA_INFO));
		abenceInfo.repeatData.bActive = EVENT_INCATIVE;
		setAbsenceData(&abenceInfo, index);
	}
}


static ABSENXE_CHECK_STATUS USER_FUNC lum_compareAbsence(U8 index, TIME_DATA_INFO* pCurTime, U16 curMinute)
{
	U16 endMinute;
	U16 startMunite;
	U8 compareWeek;
	ASBENCE_DATA_INFO* pAbenceInfo;
	BOOL withinPeriod = FALSE;
	ABSENXE_CHECK_STATUS checkStatus = OUTOF_ABSENCE;;


	pAbenceInfo =  getAbsenceData(index);
	if(pAbenceInfo->repeatData.bActive == EVENT_INCATIVE)
	{
		return checkStatus;
	}

	memcpy(&compareWeek, &pAbenceInfo->repeatData, sizeof(ALARM_REPEAT_DATA));
	lumi_debug("Absence index=%d, startHour=%d startMinute=%d endHour=%d endMinute=%d repeatData=0x%x\n", index,
		pAbenceInfo->startHour, pAbenceInfo->startMinute, pAbenceInfo->endHour, pAbenceInfo->endMinute,compareWeek);
	startMunite = pAbenceInfo->startHour*60 + pAbenceInfo->startMinute;
	endMinute = pAbenceInfo->endHour*60 + pAbenceInfo->endMinute;


	if(curMinute >= startMunite) //today
	{
		if(lum_compareWeekData(compareWeek, pCurTime->week))
		{
			if(curMinute <= endMinute || startMunite >= endMinute)
			{
				withinPeriod = TRUE;
			}
		}
	}
	else if(startMunite >= endMinute)//next day
	{
		compareWeek = ((compareWeek&0x40)>>6) | ((compareWeek&0x3F)<<1) | (compareWeek&0x80); //week add 1
		if(lum_compareWeekData(compareWeek, pCurTime->week))
		{
			if(curMinute <= endMinute)
			{
				withinPeriod = TRUE;
			}
		}
	}
	if(withinPeriod)
	{
		if(startMunite == curMinute)
		{
			checkStatus = EQUAL_START;
		}
		else if(curMinute == endMinute)
		{
			checkStatus = EQUAL_END;
			lum_checkInactiveAbsence(index, pAbenceInfo);
		}
		else
		{
			checkStatus = WITHIN_ABSENCE;
		}
	}
	return checkStatus;
}


static void USER_FUNC lum_changeSwitchByAbsence(SWITCH_STATUS action, U16 curMinute, BOOL bStopAbsence)
{
	U16 randMinute;


	lumi_debug("Absence action=%d curMinute=%d bStopAbsence=%d\n", action, curMinute, bStopAbsence);
	setSwitchStatus(action);
	if(bStopAbsence)
	{
		g_nextAbsenceMinute = INVALUE_ABSENCE_MINUTE;
	}
	else
	{
		if(action == SWITCH_CLOSE)
		{
			randMinute = getRandomNumber(MIN_ABSENCE_CLOSE_INTERVAL, MAX_ABSENCE_CLOSE_INTERVAL);
		}
		else
		{
			randMinute = getRandomNumber(MIN_ABSENCE_OPEN_INTERVAL, MAX_ABSENCE_OPEN_INTERVAL);
		}
		g_nextAbsenceMinute = (curMinute + randMinute)%1440; //24*60
	}
}


static void USER_FUNC lum_checkAbsence(TIME_DATA_INFO* pCurTime, U16 curMinute)
{
	U8 i;
	ABSENXE_CHECK_STATUS checkStatus;


	checkStatus = OUTOF_ABSENCE;
	for(i=0; i<MAX_ABSENCE_COUNT; i++)
	{
		checkStatus |= lum_compareAbsence(i, pCurTime, curMinute);
	}
	if((checkStatus&EQUAL_START) != 0 || (checkStatus&WITHIN_ABSENCE) != 0)
	{
		if(!g_absenceRunning)
		{
			lum_changeSwitchByAbsence(SWITCH_OPEN, curMinute, FALSE);
			g_absenceRunning = TRUE;
		}
	}
	else
	{
		if(g_absenceRunning)
		{
			lum_changeSwitchByAbsence(SWITCH_CLOSE, curMinute, TRUE);
			g_absenceRunning = FALSE;
		}
	}

	if(g_absenceRunning && curMinute == g_nextAbsenceMinute)
	{
		SWITCH_STATUS curSwitchStatus;

		
		curSwitchStatus = getSwitchStatus();
		if(curSwitchStatus == SWITCH_CLOSE)
		{
			lum_changeSwitchByAbsence(SWITCH_OPEN, curMinute, FALSE);
		}
		else
		{
			lum_changeSwitchByAbsence(SWITCH_CLOSE, curMinute, FALSE);
		}
	}
	if(checkStatus != OUTOF_ABSENCE)
	{
		lumi_debug("Absence checkStatus = 0x%x g_absenceRunning=%d g_nextAbsenceMinute=%d curMinute=%d\n", checkStatus, g_absenceRunning, g_nextAbsenceMinute, curMinute);
	}
}


static void USER_FUNC lum_checkInactiveCountdown(U8 index, COUNTDOWN_DATA_INFO* pCountDownInfo)
{
	COUNTDOWN_DATA_INFO countDownInfo;

	memcpy(&countDownInfo, pCountDownInfo, sizeof(COUNTDOWN_DATA_INFO));
	countDownInfo.flag.bActive = EVENT_INCATIVE;
	setCountDownData(&countDownInfo, index);
}


static void USER_FUNC lum_compareCountdown(U8 index, TIME_DATA_INFO* pCurTime)
{
	COUNTDOWN_DATA_INFO* pCountDownInfo;
	TIME_DATA_INFO countdownTime;
	SWITCH_STATUS action;


	pCountDownInfo = getCountDownData(index);
	if(pCountDownInfo->flag.bActive == EVENT_INCATIVE)
	{
		return;
	}
	lum_gmtime(pCountDownInfo->count, &countdownTime);
	countdownTime.second = pCurTime->second;
	lumi_debug("countdown index=%d %04d-%02d-%02d %02d:%02d:%02d\n", index, countdownTime.year, countdownTime.month+1, countdownTime.day,
		countdownTime.hour, countdownTime.minute, countdownTime.second);

	if(memcmp(&countdownTime, pCurTime, sizeof(TIME_DATA_INFO)) == 0)
	{
		if(!lum_bAbsenceRunNow())
		{
			action = (pCountDownInfo->action == 1)?SWITCH_OPEN:SWITCH_CLOSE;
			setSwitchStatus(action);
		}

		lum_checkInactiveCountdown(index, pCountDownInfo);
	}
}


static void USER_FUNC lum_checkCountdown(TIME_DATA_INFO* pCurTime)
{
	U8 i;


	for(i=0; i<MAX_COUNTDOWN_COUNT; i++)
	{
		lum_compareCountdown(i, pCurTime);
	}
}


static void USER_FUNC lum_checkTimer(TIME_DATA_INFO* pCurTime)
{
	U16 curMinute;


	curMinute = pCurTime->hour*60 + pCurTime->minute;
	lumi_debug("\ncheckTimer %04d-%02d-%02d %02d:%02d:%02d [%d]\n", pCurTime->year, pCurTime->month+1, pCurTime->day,
		pCurTime->hour, pCurTime->minute, pCurTime->second, pCurTime->week);

	lum_checkAlarm(pCurTime, curMinute);
	lum_checkAbsence(pCurTime, curMinute);
	lum_checkCountdown(pCurTime);
}


static void USER_FUNC lum_minuteCheckProtect(TIME_DATA_INFO* pCurTime)
{
	U8 tmpMinute;
	U8 totalMinute;
	U8 i;
	U32 totalSecond;
	U32 tmpSecond;
	TIME_DATA_INFO tmpTime;


	if(g_lastCheckMinute == 0xFF)
	{
		totalMinute = 1;
	}
	else
	{
		if(pCurTime->minute < g_lastCheckMinute)
		{
			tmpMinute = pCurTime->minute + 60;
		}
		else
		{
			tmpMinute = pCurTime->minute;
		}
		totalMinute = tmpMinute - g_lastCheckMinute;
	}
	if(totalMinute > 10) //calibrate utc time
	{
		totalMinute = 1;
	}

	//lumDebug("totalMinute=%d g_lastCheckMinute=%d\n", totalMinute, g_lastCheckMinute);
	if(totalMinute > 1)
	{
		totalSecond = lum_getSystemTime();
		for(i=1; i<=totalMinute; i++)
		{
			tmpSecond = totalSecond - (totalMinute-i)*60;
			lum_gmtime(tmpSecond, &tmpTime);
			lum_checkTimer(&tmpTime);
		}
	}
	else
	{
		lum_checkTimer(pCurTime);
	}
}


static void USER_FUNC lum_checkTimerCallback(hftimer_handle_t htimer)
{
	TIME_DATA_INFO curTime;
	U32 timerPeriod;
	U32 curSecond;


	curSecond = lum_getSystemTime();
	if(curSecond > SEC_2015_01_01_00_00_00)	// > 2015
	{
		lum_gmtime(curSecond, &curTime);
		lum_minuteCheckProtect(&curTime);
		g_lastCheckMinute = curTime.minute;
		timerPeriod = (70 - curTime.second)*1000;
	}
	else
	{
		timerPeriod = NOT_NTP_CHECK_TIMER_PERIOD;
	}
	lum_initTimer(timerPeriod);
}


void USER_FUNC lum_checkAbsenceWhileChange(void)
{
	TIME_DATA_INFO curTime;
	U16 curMinute;


	lum_getGmtime(&curTime);
	curMinute = curTime.hour*60 + curTime.minute;
	lum_checkAbsence(&curTime, curMinute);
}




void USER_FUNC lum_initTimer(U32 period)
{
	static hftimer_handle_t checkTimerFd = NULL;

	
	if(checkTimerFd == NULL)
	{
		checkTimerFd = hftimer_create("AlarmCheck_TIMER",period, false, CHECK_TIME_TIMER_ID, lum_checkTimerCallback, 0);
	}
	hftimer_change_period(checkTimerFd, period);
}

#endif
