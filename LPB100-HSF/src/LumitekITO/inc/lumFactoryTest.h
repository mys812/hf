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


#define FACTORY_TEST_SSID		"moduletest"
#define FACTORY_TEST_PASSWORD	"test1234"
//#define FACTORY_TEST_SSID		"Lumlink_test"
//#define FACTORY_TEST_PASSWORD	"12340000"

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
#endif
} FACORY_TEST_DATA;



BOOL USER_FUNC lum_checkNeedFactoryTest(void);
void USER_FUNC lum_factoryTestThreadInit(void);
void USER_FUNC lum_addFactoryKeyPressTimes(BOOL key, BOOL extraKey, BOOL extraKey2);
void USER_FUNC lum_setFactoryTestFlag(BOOL bClear);
BOOL USER_FUNC lum_getFactoryTestStatus(void);
#endif /* LUM_FACTORY_TEST_SUPPORT */

#endif

