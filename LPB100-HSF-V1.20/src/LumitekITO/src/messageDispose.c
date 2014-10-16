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



static void USER_FUNC setHeartBeatBody(CMD_HEART_BEAT_RESP* pHeartBeatResp)
{
	S32 randomData;


	randomData = rand();
	pHeartBeatResp->cmdCode = MSG_CMD_HEART_BEAT;
	pHeartBeatResp->inverval = MIN_HEARTBEAT_INTERVAL + randomData%MAX_HEARTBEAT_INTERVAL;
}



void USER_FUNC rebackHeartBeat(MSG_NODE* pNode)
{
	CMD_HEART_BEAT_RESP heartBeatResp;
	CREATE_SOCKET_DATA socketData;
	U32 sendSocketLen;
	U8* sendBuf;


	memset(&heartBeatResp, 0, sizeof(CMD_HEART_BEAT_RESP));
	memset(&socketData, 0, sizeof(CREATE_SOCKET_DATA));
	
	setHeartBeatBody(&heartBeatResp);
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.keyType = getAesKeyType(pNode->dataBody.msgOrigin, pNode->dataBody.pData);
	socketData.bodyLen = sizeof(CMD_HEART_BEAT_RESP);;
	socketData.snIndex = pNode->dataBody.snIndex;
	socketData.bodyData = (U8*)(&heartBeatResp);
	
	sendBuf = createSendSocketData(&socketData, &sendSocketLen);
	if(sendBuf != NULL)
	{
		udpSocketSendData(sendBuf, sendSocketLen, pNode->dataBody.socketIp);
		FreeSocketData(sendBuf);
	}
}

#endif
