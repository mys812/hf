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


//Command 0x62 ==> Quary modual info


//Command 0x24 ==> Lock Device
typedef struct
{
	U8 cmdCode;
	U8 macAddr[DEVICE_MAC_LEN];
}CMD_LOCK_DEVIDE_REQ;




void USER_FUNC rebackFoundDevice(MSG_NODE* pNode);
void USER_FUNC rebackHeartBeat(MSG_NODE* pNode);
void USER_FUNC rebackDeviceName(MSG_NODE* pNode);
void USER_FUNC rebackLockDevice(MSG_NODE* pNode);




#endif
