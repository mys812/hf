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
#include "../inc/deviceGpio.h"





/********************************************************************************

User Request: 		|23|Dev_MAC|
Device Response: 	|23|IP|MAC| Key-Len | Key |

IP:			4-Byte，设备局域网的MAC地址
MAC:		6-Byte，设备MAC地址
Key-Len:		1-Byte，通信密钥的长度
Key			X-Byte，通信密钥

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
	socketData.keyType = getSendSocketAesKeyType(pNode->dataBody.msgOrigin, socketData.bEncrypt);
	socketData.bodyLen = sizeof(CMD_FOUND_DEVIDE_RESP);
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
	return MIN_HEARTBEAT_INTERVAL + (randomData%(MAX_HEARTBEAT_INTERVAL-MIN_HEARTBEAT_INTERVAL));
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
	u_printf("meiyusong===> Reback heart beat Interval=%d\n", intervalData);
	intervalData = htons(intervalData);
	memcpy(heartBeatResp+index, &intervalData, 2);
	index += 2;

	
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.keyType = getSendSocketAesKeyType(pNode->dataBody.msgOrigin, socketData.bEncrypt);
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
void USER_FUNC rebackGetDeviceName(MSG_NODE* pNode)
{
	CREATE_SOCKET_DATA socketData;
	U32 sendSocketLen;
	U8* sendBuf;
	U8 deviceNameResp[100];
	U16 index = 0;
	U8 dataLen;
	U8* pNameData;


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
	pNameData = getDeviceName(&dataLen);
	deviceNameResp[index] = dataLen;
	index += 1;

	//Device name data
	memcpy((deviceNameResp+index), pNameData, dataLen);
	index += dataLen;

	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.keyType = getSendSocketAesKeyType(pNode->dataBody.msgOrigin, socketData.bEncrypt);
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



/********************************************************************************
Request:		| 63 | N-Len | Name |
Response:	| 63 | Result |

********************************************************************************/
void USER_FUNC rebackSetDeviceName(MSG_NODE* pNode)
{
	CREATE_SOCKET_DATA socketData;
	U32 sendSocketLen;
	U8* sendBuf;
	U8 deviceNameResp[10];
	U8 nameLen;
	U8* pNameData;
	U16 index = 0;


	memset(&socketData, 0, sizeof(CREATE_SOCKET_DATA));
	memset(deviceNameResp, 0, sizeof(deviceNameResp));

	//Set device name
	nameLen = pNode->dataBody.pData[SOCKET_HEADER_LEN+1];
	pNameData = pNode->dataBody.pData + SOCKET_HEADER_LEN + 2;
	setDeviceName(pNameData, nameLen);
	u_printf("meiyusong===> Set device name = %s\n", pNameData);
	
	//Set reback socket body
	deviceNameResp[index] = MSG_CMD_SET_MODULE_NAME;
	index += 1;
	deviceNameResp[index] = REBACK_SUCCESS_MESSAGE;
	index += 1;


	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.keyType = getSendSocketAesKeyType(pNode->dataBody.msgOrigin, socketData.bEncrypt);
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




/********************************************************************************
User Request:		|24|dev_MAC|
Device Response:	|24|Result|

********************************************************************************/
void USER_FUNC rebackLockDevice(MSG_NODE* pNode)
{
	CMD_LOCK_DEVIDE_REQ* pLockDeviceReq;
	CREATE_SOCKET_DATA socketData;
	U32 sendSocketLen;
	U8* sendBuf;
	U8 deviceLockResp[10];
	U16 index = 0;
	U8 result;
	U8* macAddr;


	memset(&socketData, 0, sizeof(CREATE_SOCKET_DATA));
	memset(deviceLockResp, 0, sizeof(deviceLockResp));

	//Lock device
	pLockDeviceReq = (CMD_LOCK_DEVIDE_REQ*)(pNode->dataBody.pData + SOCKET_HEADER_LEN);

	macAddr = getDeviceMacAddr(NULL);
	if(memcmp(pLockDeviceReq->macAddr, macAddr, DEVICE_MAC_LEN) == 0)
	{
		changeDeviceLockedStatus(TRUE);
		result = REBACK_SUCCESS_MESSAGE;
	}
	else
	{
		result = REBACK_FAILD_MESSAGE;
	}

	//Fill reback body
	deviceLockResp[index] = MSG_CMD_LOCK_DEVICE;
	index += 1;
	//memcpy((deviceLockResp + index), &result, sizeof(U16));
	deviceLockResp[index] = result;
	index += 1;
	
	
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.keyType = getSendSocketAesKeyType(pNode->dataBody.msgOrigin, socketData.bEncrypt);
	socketData.bodyLen = index;
	socketData.snIndex = pNode->dataBody.snIndex;
	socketData.bodyData = deviceLockResp;
	
	sendBuf = createSendSocketData(&socketData, &sendSocketLen);
	if(sendBuf != NULL)
	{
		udpSocketSendData(sendBuf, sendSocketLen, pNode->dataBody.socketIp);
		FreeSocketData(sendBuf);
	}
}




/********************************************************************************
Request:		| 01 | Pin |
Response:	| 01 | Pin|

********************************************************************************/
void USER_FUNC rebackSetGpioStatus(MSG_NODE* pNode)
{
	GPIO_STATUS* pGpioStatus;
	CREATE_SOCKET_DATA socketData;
	U32 sendSocketLen;
	U8* sendBuf;
	U8 gpioStatusResp[20];
	U16 index = 0;


	memset(&socketData, 0, sizeof(CREATE_SOCKET_DATA));
	memset(gpioStatusResp, 0, sizeof(gpioStatusResp));

	//set gpio status
	pGpioStatus = (GPIO_STATUS*)(pNode->dataBody.pData + SOCKET_HEADER_LEN + 1);
	u_printf("meiyusong===> flag=%d fre=%d duty=%d res=%d\n", pGpioStatus->flag, pGpioStatus->fre, pGpioStatus->duty, pGpioStatus->res);
	if(pGpioStatus->duty == 0xFF) //Open
	{
		setSwitchStatus(TRUE);
	}
	else //Close
	{
		setSwitchStatus(FALSE);
	}

	//Set reback socket body
	gpioStatusResp[index] = MSG_CMD_SET_GPIO_STATUS;
	index += 1;
	memcpy((gpioStatusResp + index), pGpioStatus, sizeof(GPIO_STATUS));
	index += sizeof(GPIO_STATUS);

	
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.keyType = getSendSocketAesKeyType(pNode->dataBody.msgOrigin, socketData.bEncrypt);
	socketData.bodyLen = index;
	socketData.snIndex = pNode->dataBody.snIndex;
	socketData.bodyData = gpioStatusResp;
	
	sendBuf = createSendSocketData(&socketData, &sendSocketLen);
	if(sendBuf != NULL)
	{
		udpSocketSendData(sendBuf, sendSocketLen, pNode->dataBody.socketIp);
		FreeSocketData(sendBuf);
	}
}




/********************************************************************************
Request:		| 02 | Pin |
Response:	| 02 | Pin|

********************************************************************************/
void USER_FUNC rebackGetGpioStatus(MSG_NODE* pNode)
{
	GPIO_STATUS* pGpioStatus;
	CREATE_SOCKET_DATA socketData;
	U32 sendSocketLen;
	U8* sendBuf;
	U8 gpioStatusResp[20];
	U16 index = 0;


	memset(&socketData, 0, sizeof(CREATE_SOCKET_DATA));
	memset(gpioStatusResp, 0, sizeof(gpioStatusResp));

	//Get gpio status
	pGpioStatus = (GPIO_STATUS*)(pNode->dataBody.pData + SOCKET_HEADER_LEN + 1);
	u_printf("meiyusong===> flag=%d fre=%d duty=%d res=%d\n", pGpioStatus->flag, pGpioStatus->fre, pGpioStatus->duty, pGpioStatus->res);
	if(getSwitchStatus()) //Open
	{
		pGpioStatus->duty = 0xFF;
	}
	else //Close
	{
		pGpioStatus->duty = 0x0;
	}

	//Set reback socket body
	gpioStatusResp[index] = MSG_CMD_GET_GPIO_STATUS;
	index += 1;
	memcpy((gpioStatusResp + index), pGpioStatus, sizeof(GPIO_STATUS));
	index += sizeof(GPIO_STATUS);

	
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.keyType = getSendSocketAesKeyType(pNode->dataBody.msgOrigin, socketData.bEncrypt);
	socketData.bodyLen = index;
	socketData.snIndex = pNode->dataBody.snIndex;
	socketData.bodyData = gpioStatusResp;
	
	sendBuf = createSendSocketData(&socketData, &sendSocketLen);
	if(sendBuf != NULL)
	{
		udpSocketSendData(sendBuf, sendSocketLen, pNode->dataBody.socketIp);
		FreeSocketData(sendBuf);
	}
}



/********************************************************************************
Request:		| 65 | URL-Len | URL |
Response:	| 65 | Result |

参数说明：
URL-Len:	1 - Byte，新固件URL地址的长度
URL:		X - Byte，新固件的URL地址

********************************************************************************/
void USER_FUNC rebackGetDeviceUpgrade(MSG_NODE* pNode)
{
	CREATE_SOCKET_DATA socketData;
	U32 sendSocketLen;
	U8* sendBuf;
	U8* urlData;
	U8 urlLen;
	U8 deviceUpgradeResp[20];
	U16 index = 0;


	memset(&socketData, 0, sizeof(CREATE_SOCKET_DATA));
	memset(deviceUpgradeResp, 0, sizeof(deviceUpgradeResp));

	//Get upgrade URL addr
	urlLen = pNode->dataBody.pData[SOCKET_HEADER_LEN + 1];
	urlData = pNode->dataBody.pData + SOCKET_HEADER_LEN + 2;

	//start upgrade
	u_printf("meiyusong===>urlLen=%d urlData=%s\n", urlLen, urlData);
	
	//Set reback socket body
	deviceUpgradeResp[index] = MSG_CMD_MODULE_UPGRADE;
	index += 1;
	deviceUpgradeResp[index] = REBACK_SUCCESS_MESSAGE;
	index += 1;

	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.keyType = getSendSocketAesKeyType(pNode->dataBody.msgOrigin, socketData.bEncrypt);
	socketData.bodyLen = index;
	socketData.snIndex = pNode->dataBody.snIndex;
	socketData.bodyData = deviceUpgradeResp;
	
	sendBuf = createSendSocketData(&socketData, &sendSocketLen);
	if(sendBuf != NULL)
	{
		udpSocketSendData(sendBuf, sendSocketLen, pNode->dataBody.socketIp);
		FreeSocketData(sendBuf);
	}
}




/********************************************************************************
Request:		| 66 |
Response:	| 66 | Result |

********************************************************************************/
void USER_FUNC rebackEnterSmartLink(MSG_NODE* pNode)
{
	CREATE_SOCKET_DATA socketData;
	U32 sendSocketLen;
	U8* sendBuf;
	U8 enterSmartLinkResp[10];
	U16 index = 0;


	memset(&socketData, 0, sizeof(CREATE_SOCKET_DATA));
	memset(enterSmartLinkResp, 0, sizeof(enterSmartLinkResp));

	//Send enter smartlink message
	insertLocalMsgToList(MSG_LOCAL_EVENT, NULL, 0, MSG_CMD_LOCAL_ENTER_SMARTLINK);
	
	//Set reback socket body
	enterSmartLinkResp[index] = MSG_CMD_ENTER_SMART_LINK;
	index += 1;
	enterSmartLinkResp[index] = REBACK_SUCCESS_MESSAGE;
	index += 1;

	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.keyType = getSendSocketAesKeyType(pNode->dataBody.msgOrigin, socketData.bEncrypt);
	socketData.bodyLen = index;
	socketData.snIndex = pNode->dataBody.snIndex;
	socketData.bodyData = enterSmartLinkResp;
	
	sendBuf = createSendSocketData(&socketData, &sendSocketLen);
	if(sendBuf != NULL)
	{
		udpSocketSendData(sendBuf, sendSocketLen, pNode->dataBody.socketIp);
		FreeSocketData(sendBuf);
	}
}



/********************************************************************************
Request:		| 04 | Pin_num|Num| ... |
Response:	| 04 | Pin_num|Num | Flag | Hour | Min | Pin | ... |

********************************************************************************/
void USER_FUNC rebackSetAlarmData(MSG_NODE* pNode)
{
	CREATE_SOCKET_DATA socketData;
	U32 sendSocketLen;
	U8* sendBuf;
	ALRAM_DATA* pAlarmData;
	GPIO_STATUS* pGpioStatus;
	ALARM_DATA_INFO alarmInfo;
	U8 enterSetAlarmResp[10];
	U16 index = 0;


	memset(&socketData, 0, sizeof(CREATE_SOCKET_DATA));
	memset(enterSetAlarmResp, 0, sizeof(enterSetAlarmResp));

	//Save alarm data
	pAlarmData = (ALRAM_DATA*)(pNode->dataBody.pData + SOCKET_HEADER_LEN);
	pGpioStatus = (GPIO_STATUS*)(pNode->dataBody.pData + SOCKET_HEADER_LEN + sizeof(ALRAM_DATA));

	memcpy(&alarmInfo.repeatData, &pAlarmData->flag, sizeof(ALARM_REPEAT_DATA));
	alarmInfo.hourData = pAlarmData->hour;
	alarmInfo.minuteData = pAlarmData->minute;
	alarmInfo.action = (pGpioStatus->duty == 0xFF)?SWITCH_OPEN:SWITCH_CLOSE;
	
	setAlarmData(&alarmInfo, (pAlarmData->index - 1)); //pAlarmData->index from 1 to 16

	//Set reback socket body
	enterSetAlarmResp[index] = MSG_CMD_SET_ALARM_DATA;
	index += 1;
	enterSetAlarmResp[index] = 0x0;
	index += 1;
	enterSetAlarmResp[index] = pAlarmData->index;
	index += 1;

	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.keyType = getSendSocketAesKeyType(pNode->dataBody.msgOrigin, socketData.bEncrypt);
	socketData.bodyLen = index;
	socketData.snIndex = pNode->dataBody.snIndex;
	socketData.bodyData = enterSetAlarmResp;
	
	sendBuf = createSendSocketData(&socketData, &sendSocketLen);
	if(sendBuf != NULL)
	{
		udpSocketSendData(sendBuf, sendSocketLen, pNode->dataBody.socketIp);
		FreeSocketData(sendBuf);
	}
}




/********************************************************************************
Request:		| 04 | Pin_num|Num| ... |
Response:	| 04 | Pin_num|Num | Flag | Hour | Min | Pin | ... |

********************************************************************************/
static U8 USER_FUNC fillAlarmRebackData(U8* pdata, U8 alarmIndex)
{
	ALARM_DATA_INFO* pAlarmInfo;
	GPIO_STATUS gpioStatus;
	U8 index = 0;


	pAlarmInfo = getAlarmData(alarmIndex - 1);
	pdata[index] = alarmIndex; //num
	index += 1;
	memcpy((pdata+index), &pAlarmInfo->repeatData, sizeof(ALARM_REPEAT_DATA)); //flag
	index += sizeof(ALARM_REPEAT_DATA);
	pdata[index] = pAlarmInfo->hourData; //hour
	index += 1;
	pdata[index] = pAlarmInfo->minuteData; //minute
	index += 1;

	memset(&gpioStatus, 0, sizeof(gpioStatus));
	gpioStatus.duty = (pAlarmInfo->action == SWITCH_OPEN)?0xFF:0;
	gpioStatus.res = 0xFF;
	memcpy((pdata + index), &gpioStatus, sizeof(GPIO_STATUS)); //pin
	index += sizeof(GPIO_STATUS);

	//showHexData(NULL, pdata, index);

	return index;
}



void USER_FUNC rebackGetAlarmData(MSG_NODE* pNode)
{
	CREATE_SOCKET_DATA socketData;
	U32 sendSocketLen;
	U8* sendBuf;
	U8 alarmIndex;
	U8 enterGetAlarmResp[150];  //(4+4)*MAX_ALARM_COUNT + 2+1
	U16 index = 0;
	U8 i;


	memset(&socketData, 0, sizeof(CREATE_SOCKET_DATA));
	memset(enterGetAlarmResp, 0, sizeof(enterGetAlarmResp));

	//Get data
	alarmIndex = pNode->dataBody.pData[SOCKET_HEADER_LEN + 2];

	//Set reback socket body
	enterGetAlarmResp[index] = MSG_CMD_GET_ALARM_DATA;
	index += 1;
	enterGetAlarmResp[index] = 0x0;
	index += 1;
	if(alarmIndex == 0)
	{
		for(i=1; i<=MAX_ALARM_COUNT; i++) // form 1 to MAX_ALARM_COUNT
		{
			index += fillAlarmRebackData((enterGetAlarmResp + index), i);
		}
	}
	else
	{
		index += fillAlarmRebackData((enterGetAlarmResp + index), alarmIndex);
	}


	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.keyType = getSendSocketAesKeyType(pNode->dataBody.msgOrigin, socketData.bEncrypt);
	socketData.bodyLen = index;
	socketData.snIndex = pNode->dataBody.snIndex;
	socketData.bodyData = enterGetAlarmResp;
	
	sendBuf = createSendSocketData(&socketData, &sendSocketLen);
	if(sendBuf != NULL)
	{
		udpSocketSendData(sendBuf, sendSocketLen, pNode->dataBody.socketIp);
		FreeSocketData(sendBuf);
	}
}




/********************************************************************************
Request:		| 05 | Pin_num|Num |
Response:	| 05 | Pin_num| Num | 

********************************************************************************/
void USER_FUNC rebackDeleteAlarmData(MSG_NODE* pNode)
{
	CREATE_SOCKET_DATA socketData;
	U32 sendSocketLen;
	U8* sendBuf;
	U8 alarmIndex;
	U8 enterDeleteAlarmResp[10];  
	U16 index = 0;


	memset(&socketData, 0, sizeof(CREATE_SOCKET_DATA));
	memset(enterDeleteAlarmResp, 0, sizeof(enterDeleteAlarmResp));

	alarmIndex = pNode->dataBody.pData[SOCKET_HEADER_LEN + 2];
	deleteAlarmData(alarmIndex -1);

	//Set reback socket body
	enterDeleteAlarmResp[index] = MSG_CMD_DELETE_ALARM_DATA;
	index += 1;
	enterDeleteAlarmResp[index] = 0x0;
	index += 1;
	enterDeleteAlarmResp[index] = alarmIndex;
	index += 1;

	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.keyType = getSendSocketAesKeyType(pNode->dataBody.msgOrigin, socketData.bEncrypt);
	socketData.bodyLen = index;
	socketData.snIndex = pNode->dataBody.snIndex;
	socketData.bodyData = enterDeleteAlarmResp;
	
	sendBuf = createSendSocketData(&socketData, &sendSocketLen);
	if(sendBuf != NULL)
	{
		udpSocketSendData(sendBuf, sendSocketLen, pNode->dataBody.socketIp);
		FreeSocketData(sendBuf);
	}

}




/********************************************************************************
Request:		|09|Num|Flag|Start_hour| Start_min | Stop_hour |Stop_min|Time|
Response:	|09| Result |

********************************************************************************/
void USER_FUNC rebackSetAbsenceData(MSG_NODE* pNode)
{
	CREATE_SOCKET_DATA socketData;
	U32 sendSocketLen;
	U8* sendBuf;
	ASBENCE_DATA_INFO* pAbsenceInfo;
	U8 absenceIndex;
	U8 enterSetAbsenceResp[10];
	U16 index = 0;


	memset(&socketData, 0, sizeof(CREATE_SOCKET_DATA));
	memset(enterSetAbsenceResp, 0, sizeof(enterSetAbsenceResp));

	//Save absence data
	absenceIndex = pNode->dataBody.pData[SOCKET_HEADER_LEN + 1];
	pAbsenceInfo = (ASBENCE_DATA_INFO*)(pNode->dataBody.pData + SOCKET_HEADER_LEN + 2);
	setAbsenceData(pAbsenceInfo, absenceIndex - 1);

	//Set reback socket body
	enterSetAbsenceResp[index] = MSG_CMD_SET_ABSENCE_DATA;
	index += 1;
	enterSetAbsenceResp[index] = REBACK_SUCCESS_MESSAGE;
	index += 1;

	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.keyType = getSendSocketAesKeyType(pNode->dataBody.msgOrigin, socketData.bEncrypt);
	socketData.bodyLen = index;
	socketData.snIndex = pNode->dataBody.snIndex;
	socketData.bodyData = enterSetAbsenceResp;
	
	sendBuf = createSendSocketData(&socketData, &sendSocketLen);
	if(sendBuf != NULL)
	{
		udpSocketSendData(sendBuf, sendSocketLen, pNode->dataBody.socketIp);
		FreeSocketData(sendBuf);
	}
}




/********************************************************************************
Request:		|0A |Num|
Response:	|0A|Num|Flag|Start_hour|Start_min| Stop_hour |Stop_min|Time|…|

********************************************************************************/
void USER_FUNC rebackGetAbsenceData(MSG_NODE* pNode)
{
	CREATE_SOCKET_DATA socketData;
	U32 sendSocketLen;
	U8* sendBuf;
	U8 absenceIndex;
	ASBENCE_DATA_INFO* pAbsenceInfo;
	U8 enterGetAbsenceResp[100];  //(8)*MAX_ABSENCE_COUNT + 2+1
	U16 index = 0;
	U8 i;


	memset(&socketData, 0, sizeof(CREATE_SOCKET_DATA));
	memset(enterGetAbsenceResp, 0, sizeof(enterGetAbsenceResp));

	absenceIndex = pNode->dataBody.pData[SOCKET_HEADER_LEN + 1];

	//Set reback socket body
	enterGetAbsenceResp[index] = MSG_CMD_GET_ABSENCE_DATA;
	index += 1;
	
	if(absenceIndex == 0)
	{
		for(i=1; i<=MAX_ABSENCE_COUNT; i++) 
		{
			absenceIndex = index;
			pAbsenceInfo = getAbsenceData(i - 1);
			enterGetAbsenceResp[index] = i; //Num
			index += 1;
			
			memcpy((enterGetAbsenceResp + index), pAbsenceInfo, sizeof(ASBENCE_DATA_INFO));			
			index += sizeof(ASBENCE_DATA_INFO);
			showHexData(NULL, (enterGetAbsenceResp + absenceIndex), (sizeof(ASBENCE_DATA_INFO)+1));
		}
	}
	else
	{
		pAbsenceInfo = getAbsenceData(absenceIndex - 1);
		enterGetAbsenceResp[index] = i; //Num
		index += 1;
		
		memcpy((enterGetAbsenceResp + index), pAbsenceInfo, sizeof(ASBENCE_DATA_INFO));			
		index += sizeof(ASBENCE_DATA_INFO);
	}


	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.keyType = getSendSocketAesKeyType(pNode->dataBody.msgOrigin, socketData.bEncrypt);
	socketData.bodyLen = index;
	socketData.snIndex = pNode->dataBody.snIndex;
	socketData.bodyData = enterGetAbsenceResp;
	
	sendBuf = createSendSocketData(&socketData, &sendSocketLen);
	if(sendBuf != NULL)
	{
		udpSocketSendData(sendBuf, sendSocketLen, pNode->dataBody.socketIp);
		FreeSocketData(sendBuf);
	}


}



/********************************************************************************
Request:		| 0B |Num|
Response:	|0B|Result|

********************************************************************************/
void USER_FUNC rebackDeleteAbsenceData(MSG_NODE* pNode)
{
	CREATE_SOCKET_DATA socketData;
	U32 sendSocketLen;
	U8* sendBuf;
	U8 absenceIndex;
	U8 enterDeleteAbsenceResp[10];  
	U16 index = 0;


	memset(&socketData, 0, sizeof(CREATE_SOCKET_DATA));
	memset(enterDeleteAbsenceResp, 0, sizeof(enterDeleteAbsenceResp));

	absenceIndex = pNode->dataBody.pData[SOCKET_HEADER_LEN + 1];
	deleteAbsenceData(absenceIndex -1);

	//Set reback socket body
	enterDeleteAbsenceResp[index] = MSG_CMD_DELETE_ABSENCE_DATA;
	index += 1;
	enterDeleteAbsenceResp[index] = REBACK_SUCCESS_MESSAGE;
	index += 1;

	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.keyType = getSendSocketAesKeyType(pNode->dataBody.msgOrigin, socketData.bEncrypt);
	socketData.bodyLen = index;
	socketData.snIndex = pNode->dataBody.snIndex;
	socketData.bodyData = enterDeleteAbsenceResp;
	
	sendBuf = createSendSocketData(&socketData, &sendSocketLen);
	if(sendBuf != NULL)
	{
		udpSocketSendData(sendBuf, sendSocketLen, pNode->dataBody.socketIp);
		FreeSocketData(sendBuf);
	}

}




void USER_FUNC localEnterSmartLink(MSG_NODE* pNode)
{
	deviceEnterSmartLink();
}

#endif
