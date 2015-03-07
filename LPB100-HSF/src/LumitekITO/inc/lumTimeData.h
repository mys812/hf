#ifndef __LUMLINK_TIME_DATA_H__
#define __LUMLINK_TIME_DATA_H__

#include <hsf.h>

#define DIFF_SEC_1900_1970         			(2208988800UL)
#define SEC_2014_01_01_00_00_00				(1388505600UL)
#define SEC_2015_01_01_00_00_00				(1420041600UL)
#define MAX_CALIBRATE_TIME_INTERVAL			(3600000UL)
#define MAX_FAILD_CALIBRATE_TIME_INTERVAL	(10000UL)


typedef struct
{
	U32 lastUTCTime;  // network UTC time
	U32 lastSystemTime; //get time from system
} TIME_DATE_INFO;


typedef struct
{
	U16 year;		/* year. The number of years */
	U8 month;		/* month [0-11] */
	U8 day;			/* day of the month [1-31] */
	U8 week;		/* day of the week [0-6] 0-Sunday...6-Saturday */
	U8 hour;		/* hours [0-23] */
	U8 minute;		/* minutes [0-59] */
	U8 second;		/* seconds [0-59] */
} TIME_DATA_INFO;


void USER_FUNC getUtcTimeByMessage(void);
void USER_FUNC lum_initSystemTime(void);
U32 USER_FUNC lum_getSystemTime(void);
void USER_FUNC lum_getGmtime(TIME_DATA_INFO* timeInfo);
void USER_FUNC lum_getStringTime(S8* timeData, BOOL needDay, BOOL chinaDate);
void USER_FUNC lum_gmtime(U32 second, TIME_DATA_INFO* timeInfo);
void USER_FUNC lum_checlCaliDateByApp(U32 appDate);

#endif

