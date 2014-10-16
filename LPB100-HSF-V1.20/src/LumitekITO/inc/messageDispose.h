#ifndef __MESSAGE_DISPOSE_H__
#define __MESSAGE_DISPOSE_H__

#include <hsf.h>
#include "../inc/itoCommon.h"


//command 0x23
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
typedef struct
{
	U8 cmdCode;
	U16 inverval; 
}CMD_HEART_BEAT_RESP;











void USER_FUNC rebackFoundDevice(MSG_NODE* pNode);
void USER_FUNC rebackHeartBeat(MSG_NODE* pNode);


#endif
