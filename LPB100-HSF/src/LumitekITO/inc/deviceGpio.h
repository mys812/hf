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
	U32 startTime;
	const BUZZER_RING_DATA* pRingData;
}BUZZER_RING_INFO;


typedef enum
{
	SWITCH_PIN_1	= 0,
	SWITCH_PIN_2	= 1,
	SWITCH_PIN_3	= 2,
	SWITCH_PIN_4	= 3,
}SWITCH_PIN_FLAG;


typedef struct
{
	SWITCH_STATUS	action;
	SWITCH_PIN_FLAG pinFlag;
} GPIO_CHANGE_REPORT;



#ifdef LIGHT_CHENGE_SUPPORT
#define MAX_LIGHT_LEVEL			7				//7档
#define LIGHT_DIM_BASE_TIME		1000			//2ms
#define LIGHT_DIM_SHUTDOWN_TIME	1000			//1ms
#define LIGHT_DIM_LEVEL_GAP		1200			//1.2ms
#define LIGHT_KEY_DEBOUNCE		30				//30ms去抖


typedef enum
{
	GET_AC_FREQ,		//获取市电频率
	ZERO_DETECT,		//过零检测
	SHUT_DOWN_DIM,		//关断 DIM
}LIGHT_DIM_STATUS;
#endif

void USER_FUNC initDevicePin(void);

//switch status
SWITCH_STATUS USER_FUNC getSwitchStatus(SWITCH_PIN_FLAG switchFlag);
void USER_FUNC setSwitchStatus(SWITCH_STATUS action, SWITCH_PIN_FLAG switchFlag);
void USER_FUNC changeSwitchStatus(SWITCH_PIN_FLAG switchFlag);


//buzzer status
#ifdef BUZZER_RING_SUPPORT
void USER_FUNC switchBuzzerStatus(void);
S32 USER_FUNC getBuzzerRingPeriod(const BUZZER_RING_DATA* initRingData);
void USER_FUNC setBuzzerStatus(BUZZER_STATUS buzzerStatus);
void USER_FUNC buzzerRingNotice(S32 ringPeriod, S32 stopPeriod, S32 ringTims);
#endif


#ifdef DEVICE_KEY_SUPPORT
void USER_FUNC initKeyGpio(void);
#ifdef LUM_FACTORY_TEST_SUPPORT
void USER_FUNC lum_DisableKeyInterrupt(void);
void USER_FUNC lum_EnableKeyInterrupt(void);
#endif //LUM_FACTORY_TEST_SUPPORT

#endif //DEVICE_KEY_SUPPORT

#ifdef DEVICE_WIFI_LED_SUPPORT
void USER_FUNC changeWifiLedStatus(BOOL needClose);
#endif


#ifdef RN8209C_SUPPORT
void USER_FUNC lum_rn8209cInitCfPin(void);
#endif

#ifdef LIGHT_CHENGE_SUPPORT
void USER_FUNC lum_lightChangeIRQInit(void);
void USER_FUNC lum_lightChangeLevel(U8 level);
#endif
#endif

