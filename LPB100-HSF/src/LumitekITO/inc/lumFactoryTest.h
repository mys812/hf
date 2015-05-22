/*
******************************
*Company:Lumlink
*Data:2015-03-27
*Author:Meiyusong
******************************
*/
#ifndef __LUM_FACTORY_TEST_H__
#define __LUM_FACTORY_TEST_H__

#include <hsf.h>

#ifdef LUM_FACTORY_TEST_SUPPORT


#define MAX_KEY_PRESS_TIMES		4
#define FACTORY_TEST_FLAG		0x1234ABCD

typedef struct
{
	BOOL bInFactoryTest;
	BOOL wifiConnect;
#ifdef DEVICE_KEY_SUPPORT
	U8 keyPressTimes;
#endif
#ifdef EXTRA_SWITCH_SUPPORT
	U8 exteaKeyPressTimes;
#endif
#ifdef TWO_SWITCH_SUPPORT
	U8 extraKey2PressTimes;
#endif
#ifdef RN8209C_SUPPORT
	BOOL calibrate_succ;
#endif
} FACORY_TEST_DATA;



#ifdef RN8209C_SUPPORT
typedef enum
{
	CALI_CLOSED = 0,
	CALI_FIRST = 1,
	CALI_CHECK = 2,
	CALI_SUCC = 3
}RN6209_CALI_STATUS;

#define RN8209C_CALI_GET_BASE_DATA_GAP		500
#define RN8209C_CALI_TIMER_PROTECT			7000

#endif


void USER_FUNC lum_factoryTestDhcpSucc(void);
BOOL USER_FUNC lum_checkNeedFactoryTest(void);
void USER_FUNC lum_addFactoryKeyPressTimes(BOOL key, BOOL extraKey, BOOL extraKey2);
void USER_FUNC lum_setFactoryTestFlag(BOOL bClear);
BOOL USER_FUNC lum_bEnterFactoryTest(void);
void USER_FUNC lum_enterFactoryTestThread(void *arg);
BOOL USER_FUNC lum_getFactoryTestFlag(void);

#ifdef RN8209C_SUPPORT
void lum_checkCaliData(U8* caliData);
BOOL USER_FUNC lum_getKeyEnableStatus(void);
#endif
#endif /* LUM_FACTORY_TEST_SUPPORT */

#endif

