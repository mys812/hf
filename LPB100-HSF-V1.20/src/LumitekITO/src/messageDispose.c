/*
******************************
*Company:Lumitek
*Data:2014-10-07
*Author:Meiyusong
******************************
*/

#include "../inc/lumitekConfig.h"

#ifdef CONFIG_LUMITEK_DEVICE
#include <hsf.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#include "../inc/itoCommon.h"
#include "../inc/asyncMessage.h"
#include "../inc/messageDispose.h"
#include "../inc/localSocketUdp.h"





/********************************************************************************

User Request: 		|23|Dev_MAC|
Device Response: 	|23|IP|MAC| Key-Len | Key |

IP：4 - Byte，设备局域网的MAC地址
MAC：6- Byte，设备MAC地址
Key-Len：1 - Byte，通信密钥的长度
Key：X - Byte，通信密钥

********************************************************************************/
static void USER_FUNC setFoundDeviceBody(CMD_FOUND_DEVIDE_RESP* pFoundDevResp)
{
	pFoundDevResp->cmdCode = MSG_CMD_FOUND_DEVICE;
	getDeviceIPAddr(pFoundDevResp->IP);
	pFoundDevResp->keyLen = AES_KEY_LEN;
	getAesKeyData(AES_KEY_LOCAL, pFoundDevResp->keyData);
	getDeviceMacAddr(pFoundDevResp->macAddr);
	
}



void USER_FUNC rebackFoundDevice(MSG_NODE* pNode)
{
	CMD_FOUND_DEVIDE_RESP foundDevResp;
	CREATE_SOCKET_DATA socketData;
	U32 sendSocketLen;
	U8* sendBuf;


	memset(&foundDevResp, 0, sizeof(CMD_FOUND_DEVIDE_RESP));
	memset(&socketData, 0, sizeof(CREATE_SOCKET_DATA));

	setFoundDeviceBody(&foundDevResp);
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.keyType = getAesKeyType(pNode->dataBody.msgOrigin, pNode->dataBody.pData);
	socketData.bodyLen = sizeof(CMD_FOUND_DEVIDE_RESP);;
	socketData.snIndex = pNode->dataBody.snIndex;
	socketData.bodyData = (U8*)(&foundDevResp);
	
	sendBuf = createSendSocketData(&socketData, &sendSocketLen);
	if(sendBuf != NULL)
	{
		udpSocketSendData(sendBuf, sendSocketLen, pNode->dataBody.socketIp);
		FreeSocketData(sendBuf);
	}

}




/********************************************************************************
Request:|61|

Request:|61|Response:|61|Interval|
Interval：2-Byte

********************************************************************************/
static U16 USER_FUNC getHeartBeatInterval(void)
{
	S32 randomData;


	randomData = rand();
	return htons(MIN_HEARTBEAT_INTERVAL + randomData%MAX_HEARTBEAT_INTERVAL);
}



void USER_FUNC rebackHeartBeat(MSG_NODE* pNode)
{
	CREATE_SOCKET_DATA socketData;
	U32 sendSocketLen;
	U8* sendBuf;
	U8 heartBeatResp[20];
	U16 intervalData = 0;
	U16 index = 0;


	memset(&heartBeatResp, 0, sizeof(heartBeatResp));
	memset(&socketData, 0, sizeof(CREATE_SOCKET_DATA));

	//Fill CMD
	heartBeatResp[index] = MSG_CMD_HEART_BEAT;
	index += 1;
	
	//Fill Interval
	intervalData = getHeartBeatInterval();
	memcpy(heartBeatResp+index, &intervalData, 2);
	index += 2;

	
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.keyType = getAesKeyType(pNode->dataBody.msgOrigin, pNode->dataBody.pData);
	socketData.bodyLen = index;
	socketData.snIndex = pNode->dataBody.snIndex;
	socketData.bodyData = heartBeatResp;
	
	sendBuf = createSendSocketData(&socketData, &sendSocketLen);
	if(sendBuf != NULL)
	{
		udpSocketSendData(sendBuf, sendSocketLen, pNode->dataBody.socketIp);
		FreeSocketData(sendBuf);
	}
}



/********************************************************************************
Request:|62|
Response:|62|H-Len|H-Ver|S-Len|S-Ver|N-Len|Name|

参数说明：
H-Len：1-Byte，硬件版本号长度
H-Ver：X-Byte，硬件版本号
S-Len：1-Byte，软件版本号长度
S-Ver：X-Byte，软件版本号
N-Len：1-Byte，设备别名长度
Name：X-Byte，设备别名

********************************************************************************/
void USER_FUNC rebackDeviceName(MSG_NODE* pNode)
{
	CREATE_SOCKET_DATA socketData;
	U32 sendSocketLen;
	U8* sendBuf;
	U8 deviceNameResp[100];
	U16 index = 0;
	U8 dataLen;
	U8* pData;


	memset(&socketData, 0, sizeof(CREATE_SOCKET_DATA));
	memset(deviceNameResp, 0, sizeof(deviceNameResp));

	//Fill CMD
	deviceNameResp[index] = MSG_CMD_QUARY_MODULE_INFO;
	index += 1;

	//HW version lenth
	dataLen = strlen(HW_VERSION);
	deviceNameResp[index] = dataLen;
	index += 1;

	//HW version data
	memcpy((deviceNameResp+index), HW_VERSION, dataLen);
	index += dataLen;

	//SW version lenth
	dataLen = strlen(SW_VERSION);
	deviceNameResp[index] = dataLen;
	index += 1;

	//SW version data
	memcpy((deviceNameResp+index), SW_VERSION, dataLen);
	index += dataLen;

	//Device name lenth
	pData = getDeviceName(&dataLen);
	deviceNameResp[index] = dataLen;
	index += 1;

	//Device name data
	memcpy((deviceNameResp+index), pData, dataLen);
	index += dataLen;

	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.keyType = getAesKeyType(pNode->dataBody.msgOrigin, pNode->dataBody.pData);
	socketData.bodyLen = index;
	socketData.snIndex = pNode->dataBody.snIndex;
	socketData.bodyData = deviceNameResp;
	
	sendBuf = createSendSocketData(&socketData, &sendSocketLen);
	if(sendBuf != NULL)
	{
		udpSocketSendData(sendBuf, sendSocketLen, pNode->dataBody.socketIp);
		FreeSocketData(sendBuf);
	}
	
}



#if 0

/********************************************************************************
User Request:		|24|dev_MAC|
Device Response:	|24|Result|

********************************************************************************/
void USER_FUNC rebackLockDevice(MSG_NODE* pNode)
{
	CREATE_SOCKET_DATA socketData;
	U32 sendSocketLen;
	U8* sendBuf;
	U8 deviceLockResp[10];
	U16 index = 0;


	memset(&socketData, 0, sizeof(CREATE_SOCKET_DATA));
	memset(deviceLockResp, 0, sizeof(deviceLockResp));

	//setFoundDeviceBody(&foundDevResp);
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.keyType = getAesKeyType(pNode->dataBody.msgOrigin, pNode->dataBody.pData);
	socketData.bodyLen = sizeof(CMD_FOUND_DEVIDE_RESP);;
	socketData.snIndex = pNode->dataBody.snIndex;
	socketData.bodyData = (U8*)(&foundDevResp);
	
	sendBuf = createSendSocketData(&socketData, &sendSocketLen);
	if(sendBuf != NULL)
	{
		udpSocketSendData(sendBuf, sendSocketLen, pNode->dataBody.socketIp);
		FreeSocketData(sendBuf);
	}

}
#endif

#endif
