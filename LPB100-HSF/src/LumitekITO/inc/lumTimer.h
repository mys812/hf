/*
******************************
*Company:Lumlink
*Data:2015-02-07
*Author:Meiyusong
******************************
*/

#ifndef __LUMLINK_TIMER__H__
#define __LUMLINK_TIMER__H__


#define MIN_ABSENCE_OPEN_INTERVAL		20
#define MAX_ABSENCE_OPEN_INTERVAL		30
#define MIN_ABSENCE_CLOSE_INTERVAL		30
#define MAX_ABSENCE_CLOSE_INTERVAL		50
#define INVALUE_ABSENCE_MINUTE			0xFFFF
#define NOT_NTP_CHECK_TIMER_PERIOD		10000


typedef enum
{
	OUTOF_ABSENCE		= 0x00,
	EQUAL_START			= 0x01,
	WITHIN_ABSENCE		= 0x02,
	EQUAL_END			= 0x04,
}ABSENXE_CHECK_STATUS;


void USER_FUNC lum_initTimer(U32 period);
void USER_FUNC lum_checkAbsenceWhileChange(U8 index);

#endif

