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


void USER_FUNC initDevicePin(BOOL initBeforNormal);

//switch status
SWITCH_STATUS USER_FUNC getSwitchStatus(void);
void USER_FUNC setSwitchStatus(SWITCH_STATUS action);
void USER_FUNC changeSwitchStatus(void);


//buzzer status
#ifdef BUZZER_RING_SUPPORT
void USER_FUNC switchBuzzerStatus(void);
S32 USER_FUNC getBuzzerRingPeriod(const BUZZER_RING_DATA* initRingData);
void USER_FUNC setBuzzerStatus(BUZZER_STATUS buzzerStatus);
void USER_FUNC buzzerRingNotice(S32 ringPeriod, S32 stopPeriod, S32 ringTims);
#endif


#ifdef DEVICE_KEY_SUPPORT
void USER_FUNC initKeyGpio(void);
#endif

#ifdef DEVICE_WIFI_LED_SUPPORT
void USER_FUNC changeWifiLedStatus(BOOL bClose);
#endif


#endif
