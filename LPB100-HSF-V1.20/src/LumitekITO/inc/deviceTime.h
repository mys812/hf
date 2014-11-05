#ifndef __LUMI_DEVICE_TIME_H__
#define __LUMI_DEVICE_TIME_H__

#include <hsf.h>


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



#endif
