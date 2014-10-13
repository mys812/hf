#ifndef __MESSAGE_HEADER_H__
#define __MESSAGE_HEADER_H__

#include <hsf.h>
#include "../inc/itoCommon.h"


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
	U8 keyPtr[AES_KEY_LEN];
}CMD_FOUND_DEVIDE_RESP;


#endif
