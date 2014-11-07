#ifndef __LUMI_DEVICE_TIME_H__
#define __LUMI_DEVICE_TIME_H__

#include <hsf.h>


#define START_TIMER_INTERVAL		5
#define MIN_ABSENCE_OPEN_INTERVAL		20
#define MAX_ABSENCE_OPEN_INTERVAL		30
#define MIN_ABSENCE_CLOSE_INTERVAL		30
#define MAX_ABSENCE_CLOSE_INTERVAL		50

//#define MAX_TIME_THREAD_SLEEP			120
#define MAX_TIME_THREAD_SLEEP			30




typedef struct
{
	U16 year;
	U8 month;
	U8 day;
	U8 week;
	U8 hour;
	U8 minute;
	U8 second;
	U16 dayCount;
}TIME_DATA_INFO;



void USER_FUNC deviceAlarmArrived(U8 action, U8 alarmIndex);
void USER_FUNC deviceAbsenceArrived(U8 index);
void USER_FUNC deviceCountDownArrived(U8 action);

void USER_FUNC checkAlarmTimerAfterChange(U8 index);
void USER_FUNC checkAbsenceTimerAfterChange(U8 index);
void USER_FUNC checkCountDownTimerAfterChange(U8 index);

void USER_FUNC deviceTimeThread(void);


#endif

