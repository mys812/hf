#ifndef __LUMI_DEVICE_MISC_H__
#define __LUMI_DEVICE_MISC_H__

#include <hsf.h>

#ifdef DEVICE_WIFI_LED_SUPPORT
typedef enum
{
	WIFI_LED_SMARTLINK,
	WIFI_LED_AP_DISCONNECT,
	WIFI_LED_TCP_DISCONNECT,
	WIFILED_CLOSE
}WIFI_LED_STATUS;
#endif



void USER_FUNC closeNtpMode(void);
BOOL USER_FUNC bRuningStaMode(void);

void USER_FUNC lum_createHeartBeatTimer(U16 interval);
void USER_FUNC lum_AfterConnectServer(void);

void USER_FUNC sendSmartLinkCmd(void);

#ifdef DEVICE_NO_KEY
void USER_FUNC checkNeedEnterSmartLink(void);
void USER_FUNC cancelCheckSmartLinkTimer(void);
#endif
void USER_FUNC deviceEnterSmartLink(void);

void USER_FUNC lum_checkFactoryReset(void);

#ifdef DEVICE_WIFI_LED_SUPPORT
void USER_FUNC setWifiLedStatus(WIFI_LED_STATUS ledStatus);
#endif

#ifdef LUM_FACTORY_TEST_SUPPORT
void USER_FUNC lum_showFactoryTestSucc(void);
void USER_FUNC lum_showEnterFactoryTest(void);
void USER_FUNC lum_showFactoryTestApConnect(void);
#endif

void USER_FUNC lum_stopFactoryResetTimer(void);
#endif

