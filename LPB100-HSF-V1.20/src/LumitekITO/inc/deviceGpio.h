/*
******************************
*Company:Lumitek
*Data:2014-10-07
*Author:Meiyusong
******************************
*/

#ifndef __DEVICE_GPIO_H__
#define __DEVICE_GPIO_H__

#include <hsf.h>
#include "itoCommon.h"


#ifdef DEEVICE_LUMITEK_P1

typedef struct
{
	U8 maxRingTimes;
	U16 ringPeriod;
	U16 stopPeriod;
	const U16* pRindPeriod;
}BUZZER_RING_DATA;



typedef struct
{
	U8 ringPeriodIndex;
	U8 curTimes;
	BOOL ringStop;
	time_t startTime;
	const BUZZER_RING_DATA* pRingData;
}BUZZER_RING_INFO;

#endif


void USER_FUNC initDevicePin(BOOL initBeforNormal);

//switch status
SWITCH_STATUS USER_FUNC getSwitchStatus(void);
void USER_FUNC setSwitchStatus(SWITCH_STATUS action);
void USER_FUNC changeSwitchStatus(void);


//buzzer status
void USER_FUNC switchBuzzerStatus(void);
S32 USER_FUNC getBuzzerRingPeriod(const BUZZER_RING_DATA* initRingData);

void USER_FUNC buzzerRingNotice(S32 period, S32 ringTims);

#endif

