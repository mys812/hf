#ifndef __MESSAGE_DISPOSE_H__
#define __MESSAGE_DISPOSE_H__

#include <hsf.h>
#include "../inc/itoCommon.h"



#define REBACK_SUCCESS_MESSAGE			0
#define REBACK_FAILD_MESSAGE			0xFF


//Command 0x23 ==> Found device
typedef struct
{
	U8 cmdCode;
	U8 macAddr[DEVICE_MAC_LEN];
}CMD_FOUND_DEVIDE_REQ;


typedef struct
{
	U8 cmdCode;
	U8 IP[SOCKET_IP_LEN];
	U8 macAddr[DEVICE_MAC_LEN];
	U8 keyLen;
	U8 keyData[AES_KEY_LEN];
}CMD_FOUND_DEVIDE_RESP;



//Command 0x61 ==>Heart beat


//Command 0x62 ==> Quary device info


//Command 0x63 ==> Set device name


//Command 0x01 ===> Set Gpio status
typedef struct{
	U8 flag;  
	U8 fre;   //固定为0x00
	U8 duty;   //输出高电平为0xFF，低电平为0x00
	U8 res;  //保留字节，固定为0xFF
}GPIO_STATUS;

//Command 0x24 ==> Lock Device
typedef struct
{
	U8 cmdCode;
	U8 macAddr[DEVICE_MAC_LEN];
}CMD_LOCK_DEVIDE_REQ;


//Command 0x03 ==> Set alarm
typedef struct
{
	U8 cmdCode;
	U8 pinNum;
	U8 index;
	ALARM_REPEAT_DATA flag;
	U8 hour;
	U8 minute;
}ALRAM_DATA;



//Command 0x09 ==> Set absence data


void USER_FUNC rebackFoundDevice(MSG_NODE* pNode);
void USER_FUNC rebackHeartBeat(MSG_NODE* pNode);
void USER_FUNC rebackGetDeviceName(MSG_NODE* pNode);
void USER_FUNC rebackLockDevice(MSG_NODE* pNode);
void USER_FUNC rebackSetDeviceName(MSG_NODE* pNode);
void USER_FUNC rebackSetGpioStatus(MSG_NODE* pNode);
void USER_FUNC rebackGetGpioStatus(MSG_NODE* pNode);
void USER_FUNC rebackGetDeviceUpgrade(MSG_NODE* pNode);
void USER_FUNC rebackEnterSmartLink(MSG_NODE* pNode);
void USER_FUNC rebackSetAlarmData(MSG_NODE* pNode);
void USER_FUNC rebackGetAlarmData(MSG_NODE* pNode);
void USER_FUNC rebackDeleteAlarmData(MSG_NODE* pNode);
void USER_FUNC rebackSetAbsenceData(MSG_NODE* pNode);
void USER_FUNC rebackGetAbsenceData(MSG_NODE* pNode);
void USER_FUNC rebackDeleteAbsenceData(MSG_NODE* pNode);










void USER_FUNC localEnterSmartLink(MSG_NODE* pNode);



#endif
