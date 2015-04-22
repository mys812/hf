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
#include "../inc/serverSocketTcp.h"
#include "../inc/socketSendList.h"
#include "../inc/deviceMisc.h"
#include "../inc/deviceUpgrade.h"
#include "../inc/lumTimeData.h"
#ifdef RN8209C_SUPPORT
#include "../inc/rn8209c.h"
#endif
#ifdef SX1208_433M_SUPPORT
#include "../inc/lum_sx1208.h"
#endif



U16 USER_FUNC getRandomNumber(U16 mixNum, U16 maxNum)
{
	S32 randomData;


	randomData = rand();
	return mixNum + (randomData%(maxNum - mixNum));
}


static void USER_FUNC msgSendSocketData(CREATE_SOCKET_DATA* pSocketData, MSG_NODE* pNode)
{
	U32 sendSocketLen;
	U8* sendBuf;


	if(pSocketData->bReback == 1)
	{
		pSocketData->snIndex = pNode->nodeBody.snIndex;
	}
	else
	{
		pSocketData->snIndex = getSocketSn(TRUE);
	}
	pSocketData->msgOrigin = pNode->nodeBody.msgOrigin;
	pSocketData->keyType = getSocketAesKeyType(pNode->nodeBody.msgOrigin, pSocketData->bEncrypt);

	sendBuf = createSendSocketData(pSocketData, &sendSocketLen);
	if(sendBuf != NULL)
	{
		SEND_NODE_DATA sendData;

		memset(&sendData, 0, sizeof(SEND_NODE_DATA));
		
		sendData.cmdData = pNode->nodeBody.cmdData;
		sendData.snIndex = pSocketData->snIndex;
		sendData.dataLen = sendSocketLen;
		sendData.pData = sendBuf;
		sendData.msgOrigin = pNode->nodeBody.msgOrigin;
		sendData.socketIp = pNode->nodeBody.socketIp;
		sendData.bReback = pSocketData->bReback;

		addSendDataToNode(&sendData);
	}
}



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
	U32 ipAddr;
	
	pFoundDevResp->cmdCode = MSG_CMD_FOUND_DEVICE;
	ipAddr = getDeviceIpAddress();
	memcpy(pFoundDevResp->IP, (U8*)&ipAddr, SOCKET_IP_LEN);
	pFoundDevResp->keyLen = AES_KEY_LEN;
	getAesKeyData(AES_KEY_LOCAL, pFoundDevResp->keyData);
	getDeviceMacAddr(pFoundDevResp->macAddr);

}



void USER_FUNC rebackFoundDevice(MSG_NODE* pNode)
{
	CMD_FOUND_DEVIDE_RESP foundDevResp;
	CREATE_SOCKET_DATA socketData;


	memset(&foundDevResp, 0, sizeof(CMD_FOUND_DEVIDE_RESP));
	setFoundDeviceBody(&foundDevResp);

	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.bodyLen = sizeof(CMD_FOUND_DEVIDE_RESP);
	socketData.bodyData = (U8*)(&foundDevResp);

	//send Socket
	msgSendSocketData(&socketData, pNode);
}




/********************************************************************************
Request:|61|

Request:|61|Response:|61|Interval|
Interval：2-Byte

********************************************************************************/
static void USER_FUNC rebackUdpHeartBeat(MSG_NODE* pNode)
{
	U8 heartBeatResp[10];
	U16 intervalData = 0;
	CREATE_SOCKET_DATA socketData;
	U16 index = 0;
	U32* pCurTime;
	U32 appTime;


	memset(heartBeatResp, 0, sizeof(heartBeatResp));

	pCurTime = (U32*)(pNode->nodeBody.pData + SOCKET_HEADER_LEN + 1);
	appTime = ntohl(*pCurTime);
	lum_checlCaliDateByApp(appTime);
	//Fill CMD
	heartBeatResp[index] = MSG_CMD_HEART_BEAT;
	index += 1;

	//Fill Interval
#ifdef LUM_UDP_HEART_INTERVAL_30S
	intervalData = UDP_HEARTBEAT_INTERVAL;
#else
	intervalData = getRandomNumber(MIN_HEARTBEAT_INTERVAL, MAX_HEARTBEAT_INTERVAL);
#endif
#if 0
	if(!getDeviceLockedStatus()) //由于路由器原因导致的解锁，需要重新上锁
	{
		changeDeviceLockedStatus(TRUE);
	}
#endif	
	//lumi_debug("Reback heart beat Interval=%d\n", intervalData);
	intervalData = htons(intervalData);
	memcpy(heartBeatResp+index, &intervalData, 2);
	index += 2;

	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.bodyLen = index;
	socketData.bodyData = heartBeatResp;

	//send Socket
	msgSendSocketData(&socketData, pNode);
}



static void USER_FUNC rebackTcpHeartBeat(MSG_NODE* pNode)
{
	U16 interval;


	interval = ntohs(*(U16*)(pNode->nodeBody.pData + SOCKET_HEADER_LEN + 1));
	//lumi_debug("interval=%d\n", interval);
	lum_createHeartBeatTimer(interval);
	deleteRequstSendNode(pNode->nodeBody.snIndex);
}



static void USER_FUNC requstTcpHeartBeat(MSG_NODE* pNode)
{

	U8 data;
	CREATE_SOCKET_DATA socketData;

	data = MSG_CMD_HEART_BEAT;

	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 0;
	socketData.bodyLen = 1;
	socketData.bodyData = &data;

	pNode->nodeBody.msgOrigin = MSG_FROM_TCP;

	//send Socket
	msgSendSocketData(&socketData, pNode);
}



void USER_FUNC rebackHeartBeat(MSG_NODE* pNode)
{
	if(pNode->nodeBody.msgOrigin == MSG_LOCAL_EVENT)
	{
		requstTcpHeartBeat(pNode);
	}
	else if(pNode->nodeBody.msgOrigin == MSG_FROM_UDP)
	{
		rebackUdpHeartBeat(pNode);
	}
	else if(pNode->nodeBody.msgOrigin == MSG_FROM_TCP)
	{
		rebackTcpHeartBeat(pNode);
	}
}


/********************************************************************************
Request:|62|
Response:|62|Status|H-Len|H-Ver|S-Len|S-Ver|N-Len|Name|

参数说明：
H-Len：1-Byte，硬件版本号长度
H-Ver：X-Byte，硬件版本号
S-Len：1-Byte，软件版本号长度
S-Ver：X-Byte，软件版本号
N-Len：1-Byte，设备别名长度
Name：X-Byte，设备别名

********************************************************************************/
void USER_FUNC rebackGetDeviceInfo(MSG_NODE* pNode)
{
	U8 deviceNameResp[100];
	U8 dataLen;
	DEVICE_NAME_DATA* pNameData;
	CREATE_SOCKET_DATA socketData;
	U16 index = 0;


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
	pNameData = getDeviceName();
	deviceNameResp[index] = pNameData->nameLen;
	index += 1;

	//Device name data
	memcpy((deviceNameResp + index), pNameData->nameData, pNameData->nameLen);
	index += pNameData->nameLen;
	lumi_debug("name=%s, nameLen=%d\n", pNameData->nameData, pNameData->nameLen);

	//status
	deviceNameResp[index] = 0x11;
	index += 1;

	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.bodyLen = index;
	socketData.bodyData = deviceNameResp;

	//send Socket
	msgSendSocketData(&socketData, pNode);

}



/********************************************************************************
Request:		| 63 | N-Len | Name |
Response:	| 63 | Result |

********************************************************************************/
void USER_FUNC rebackSetDeviceName(MSG_NODE* pNode)
{
	U8 deviceNameResp[10];
	DEVICE_NAME_DATA nameData;
	CREATE_SOCKET_DATA socketData;
	U16 index = 0;


	memset(deviceNameResp, 0, sizeof(deviceNameResp));
	memset(&nameData, 0, sizeof(DEVICE_NAME_DATA));

	//Set device name
	nameData.nameLen = pNode->nodeBody.pData[SOCKET_HEADER_LEN+1];
	nameData.nameLen = (nameData.nameLen > (DEVICE_NAME_LEN - 2))?(DEVICE_NAME_LEN - 2):nameData.nameLen;
	memcpy(nameData.nameData, (pNode->nodeBody.pData + SOCKET_HEADER_LEN + 2), nameData.nameLen);
	setDeviceName(&nameData);
	lumi_debug("Set device name = %s\n", nameData.nameData);

	//Set reback socket body
	deviceNameResp[index] = MSG_CMD_SET_MODULE_NAME;
	index += 1;
	deviceNameResp[index] = REBACK_SUCCESS_MESSAGE;
	index += 1;

	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.bodyLen = index;
	socketData.bodyData = deviceNameResp;

	//send Socket
	msgSendSocketData(&socketData, pNode);
}




/********************************************************************************
User Request:		|24|dev_MAC|
Device Response:	|24|Result|

********************************************************************************/
void USER_FUNC rebackLockDevice(MSG_NODE* pNode)
{
	CMD_LOCK_DEVIDE_REQ* pLockDeviceReq;
	U8 deviceLockResp[10];
	CREATE_SOCKET_DATA socketData;
	U16 index = 0;
	U8 result;
	U8* macAddr;


	memset(deviceLockResp, 0, sizeof(deviceLockResp));

	//Lock device
	pLockDeviceReq = (CMD_LOCK_DEVIDE_REQ*)(pNode->nodeBody.pData + SOCKET_HEADER_LEN);

	macAddr = getDeviceMacAddr(NULL);
	if(memcmp(pLockDeviceReq->macAddr, macAddr, DEVICE_MAC_LEN) == 0)
	{
		changeDeviceLockedStatus(TRUE);
		lum_setUserName((U8*)(pNode->nodeBody.pData + SOCKET_HEADER_LEN + sizeof(CMD_LOCK_DEVIDE_REQ)));
		result = REBACK_SUCCESS_MESSAGE;

		lum_checkFactoryReset();
	}
	else
	{
		result = REBACK_FAILD_MESSAGE;
	}

	//Fill reback body
	deviceLockResp[index] = MSG_CMD_LOCK_DEVICE;
	index += 1;

	deviceLockResp[index] = result;
	index += 1;

	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.bodyLen = index;
	socketData.bodyData = deviceLockResp;

	//send Socket
	msgSendSocketData(&socketData, pNode);
}




/********************************************************************************
Request:		| 01 | Pin |
Response:	| 01 | Pin|

********************************************************************************/
static SWITCH_PIN_FLAG USER_FUNC lum_getSwitchPinFlag(U8 flag)
{
	SWITCH_PIN_FLAG switchFlag;


	if(flag == 0)
	{
		switchFlag = SWITCH_PIN_1;
	}
#ifdef TWO_SWITCH_SUPPORT
	else if(flag == 1)
	{
		switchFlag = SWITCH_PIN_2;
	}
#endif
	else
	{
		switchFlag = SWITCH_PIN_1;
	}
	return switchFlag;
}


void USER_FUNC rebackSetGpioStatus(MSG_NODE* pNode)
{
	GPIO_STATUS* pGpioStatus;
	U8 gpioStatusResp[20];
	CREATE_SOCKET_DATA socketData;
	U16 index = 0;
	SWITCH_PIN_FLAG switchFlag;
#ifdef SX1208_433M_SUPPORT
	U8 index2 = 0;
#endif

	memset(gpioStatusResp, 0, sizeof(gpioStatusResp));

	//set gpio status
	pGpioStatus = (GPIO_STATUS*)(pNode->nodeBody.pData + SOCKET_HEADER_LEN + 1);
	switchFlag = lum_getSwitchPinFlag(pGpioStatus->flag);

	//lumi_debug("flag=%d fre=%d duty=%d res=%d\n", pGpioStatus->flag, pGpioStatus->fre, pGpioStatus->duty, pGpioStatus->res);
	if(pGpioStatus->duty == 0xFF) //Open
	{
		setSwitchStatus(SWITCH_OPEN, switchFlag);
	}
	else //Close
	{
		setSwitchStatus(SWITCH_CLOSE, switchFlag);
	}

	//Set reback socket body
	gpioStatusResp[index] = MSG_CMD_SET_GPIO_STATUS;
	index += 1;
	memcpy((gpioStatusResp + index), pGpioStatus, sizeof(GPIO_STATUS));
	index += sizeof(GPIO_STATUS);

	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.bodyLen = index;
	socketData.bodyData = gpioStatusResp;

	//send Socket
	msgSendSocketData(&socketData, pNode);
#ifdef SX1208_433M_SUPPORT
	index2 = (pGpioStatus->duty == 0xFF)?0:1;
	insertLocalMsgToList(MSG_LOCAL_EVENT, &index2, 1, MSG_CMD_SEND_433_WAVE);
#endif
}




/********************************************************************************
Request:		| 02 | Pin |
Response:	| 02 | Pin|

********************************************************************************/
void USER_FUNC rebackGetGpioStatus(MSG_NODE* pNode)
{
	GPIO_STATUS* pGpioStatus;
	U8 gpioStatusResp[20];
	CREATE_SOCKET_DATA socketData;
	U16 index = 0;
	SWITCH_PIN_FLAG switchFlag;


	memset(gpioStatusResp, 0, sizeof(gpioStatusResp));

	//Get gpio status
	pGpioStatus = (GPIO_STATUS*)(pNode->nodeBody.pData + SOCKET_HEADER_LEN + 1);
	switchFlag = lum_getSwitchPinFlag(pGpioStatus->flag);
	lumi_debug("flag=%d fre=%d duty=%d res=%d switchFlag=%d\n", pGpioStatus->flag, pGpioStatus->fre, pGpioStatus->duty, pGpioStatus->res, switchFlag);
	if(getSwitchStatus(switchFlag)) //Open
	{
		pGpioStatus->duty = 0xFF;
	}
	else //Close
	{
		pGpioStatus->duty = 0x0;
	}
	pGpioStatus->res = 0xFF;

	//Set reback socket body
	gpioStatusResp[index] = MSG_CMD_GET_GPIO_STATUS;
	index += 1;
	memcpy((gpioStatusResp + index), pGpioStatus, sizeof(GPIO_STATUS));
	index += sizeof(GPIO_STATUS);

	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.bodyLen = index;
	socketData.bodyData = gpioStatusResp;

	//send Socket
	msgSendSocketData(&socketData, pNode);
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
	U8* urlData;
	U8 urlLen;
	U8 deviceUpgradeResp[20];
	CREATE_SOCKET_DATA socketData;
	U16 index = 0;


	memset(deviceUpgradeResp, 0, sizeof(deviceUpgradeResp));

	//Get upgrade URL addr
	urlLen = pNode->nodeBody.pData[SOCKET_HEADER_LEN + 1];
	urlData = pNode->nodeBody.pData + SOCKET_HEADER_LEN + 2;

	//start upgrade
	setSoftwareUpgradeUrl((S8*)urlData, urlLen);
	lumi_debug("urlLen=%d urlData=%s\n", urlLen, urlData);

	//Set reback socket body
	deviceUpgradeResp[index] = MSG_CMD_MODULE_UPGRADE;
	index += 1;
	deviceUpgradeResp[index] = REBACK_SUCCESS_MESSAGE;
	index += 1;

	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.bodyLen = index;
	socketData.bodyData = deviceUpgradeResp;

	//send Socket
	msgSendSocketData(&socketData, pNode);
}



void USER_FUNC LocalGetDeviceUpgrade(void)
{
	resetForUpgrade();
}




/********************************************************************************
Request:		| 66 |
Response:	| 66 | Result |

********************************************************************************/
void USER_FUNC rebackEnterSmartLink(MSG_NODE* pNode)
{
	U8 enterSmartLinkResp[10];
	CREATE_SOCKET_DATA socketData;
	U16 index = 0;
	U8 enterReson;


	enterReson = ENTER_SMARTLINK_BY_NETWORK;
	memset(enterSmartLinkResp, 0, sizeof(enterSmartLinkResp));

	//Send enter smartlink message
	insertLocalMsgToList(MSG_LOCAL_EVENT, &enterReson, 1, MSG_CMD_LOCAL_ENTER_SMARTLINK);
	changeDeviceLockedStatus(FALSE);

	//Set reback socket body
	enterSmartLinkResp[index] = MSG_CMD_ENTER_SMART_LINK;
	index += 1;
	enterSmartLinkResp[index] = REBACK_SUCCESS_MESSAGE;
	index += 1;

	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.bodyLen = index;
	socketData.bodyData = enterSmartLinkResp;

	//send Socket
	msgSendSocketData(&socketData, pNode);
}


#ifdef DEVICE_NO_KEY
void USER_FUNC localEnterSmartLink(MSG_NODE* pNode)
{
	if(pNode->nodeBody.pData != NULL && *pNode->nodeBody.pData == ENTER_SMARTLINK_BY_NETWORK)
	{
		lum_setFactorySmartlink(TRUE);
	}
	sendSmartLinkCmd();
}
#endif


/********************************************************************************
Request:		| 03 |Pin_Num|Num|Flag|Start_Hour|Start_Min|Stop_Hour|Stop_min|Reserve|
Response:	| 03 |Result|


********************************************************************************/

static U8 USER_FUNC lum_getAlarmIndexOffset(U8 pinNum)
{
	U8 offset;


	if(pinNum == SWITCH_PIN_1)
	{
		offset = 0;
	}
#ifdef TWO_SWITCH_SUPPORT
	else if(pinNum == SWITCH_PIN_2)
	{
		offset = MAX_ALARM_COUNT;
	}
#endif
	else
	{
		lumi_error("Alarm pinNum error pinNum=%d\n", pinNum);
		offset = 0;
	}
	return offset;
}


void USER_FUNC rebackSetAlarmData(MSG_NODE* pNode)
{
	ALRAM_DATA* pAlarmData;
	U8 SetAlarmResp[10];
	CREATE_SOCKET_DATA socketData;
	U16 index = 0;
	U8 indexOffset;


	memset(SetAlarmResp, 0, sizeof(SetAlarmResp));

	//Save alarm data
	pAlarmData = (ALRAM_DATA*)(pNode->nodeBody.pData + SOCKET_HEADER_LEN);
	indexOffset = lum_getAlarmIndexOffset(pAlarmData->pinNum);
	setAlarmData(&pAlarmData->alarmInfo, (pAlarmData->index - 1 + indexOffset)); //pAlarmData->index from 1 to 32

	//Set reback socket body
	SetAlarmResp[index] = MSG_CMD_SET_ALARM_DATA;
	index += 1;
	SetAlarmResp[index] = REBACK_SUCCESS_MESSAGE;
	index += 1;

	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.bodyLen = index;
	socketData.bodyData = SetAlarmResp;

	//send Socket
	msgSendSocketData(&socketData, pNode);
}




/********************************************************************************
Request: |04|Pin_Num|Num| ... |
Response:|04|Pin_Num|Num|Flag|Start_Hour|Start_Min|Stop_Hour|Stop_Min|Reserve|... |


********************************************************************************/
static U8 USER_FUNC fillAlarmRebackData(U8* pdata, U8 alarmIndex, U8 indexOffset, BOOL needEmptyData)
{
	ALARM_DATA_INFO* pAlarmInfo;
	U8 index = 0;


	pAlarmInfo = getAlarmData(alarmIndex - 1);
	//if(pAlarmInfo->startHour != INVALID_ALARM_FLAG || needEmptyData)
	{
		pdata[index] = alarmIndex - indexOffset; //num
		index += 1;
		memcpy((pdata+index), pAlarmInfo, sizeof(ALARM_DATA_INFO)); //flag
		index += sizeof(ALARM_DATA_INFO);

		//showHexData("Alarm ", pdata, index);
	}

	return index;
}



void USER_FUNC rebackGetAlarmData(MSG_NODE* pNode)
{
	U8 alarmIndex;
	U8 pinNum;
	U8 GetAlarmResp[250];  //(4+4)*MAX_ALARM_COUNT + 2+1
	CREATE_SOCKET_DATA socketData;
	U16 index = 0;
	U8 i;
	U8 indexOffset;
	U8 indexStart;
	U8 indexEnd;


	memset(GetAlarmResp, 0, sizeof(GetAlarmResp));

	//Get data
	pinNum = pNode->nodeBody.pData[SOCKET_HEADER_LEN + 1];
	alarmIndex = pNode->nodeBody.pData[SOCKET_HEADER_LEN + 2];

	indexOffset = lum_getAlarmIndexOffset(pinNum);

	//Set reback socket body
	GetAlarmResp[index] = MSG_CMD_GET_ALARM_DATA;
	index += 1;
	GetAlarmResp[index] = pinNum;
	index += 1;
	if(alarmIndex == 0)
	{
		indexStart = 1 + indexOffset;
		indexEnd = MAX_ALARM_COUNT + indexOffset;
		for(i=indexStart; i<=indexEnd; i++) // form 1 to MAX_ALARM_COUNT
		{
			index += fillAlarmRebackData((GetAlarmResp + index), i, indexOffset, FALSE);
		}
	}
	else
	{
		index += fillAlarmRebackData((GetAlarmResp + index), (alarmIndex + indexOffset), indexOffset, TRUE);
	}

	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.bodyLen = index;
	socketData.bodyData = GetAlarmResp;
	//send Socket
	msgSendSocketData(&socketData, pNode);
}




/********************************************************************************
Request:		| 05 | Pin_num|Num |
Response:	| 05 | Result |

********************************************************************************/
void USER_FUNC rebackDeleteAlarmData(MSG_NODE* pNode)
{
	U8 alarmIndex;
	U8 pinNum;
	U8 DeleteAlarmResp[10];
	CREATE_SOCKET_DATA socketData;
	U16 index = 0;
	U8 indexOffset;


	memset(DeleteAlarmResp, 0, sizeof(DeleteAlarmResp));

	pinNum = pNode->nodeBody.pData[SOCKET_HEADER_LEN + 1];
	alarmIndex = pNode->nodeBody.pData[SOCKET_HEADER_LEN + 2];
	indexOffset = lum_getAlarmIndexOffset(pinNum);
	deleteAlarmData((alarmIndex - 1 + indexOffset), TRUE);

	//Set reback socket body
	DeleteAlarmResp[index] = MSG_CMD_DELETE_ALARM_DATA;
	index += 1;
	DeleteAlarmResp[index] = REBACK_SUCCESS_MESSAGE;
	index += 1;

	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.bodyLen = index;
	socketData.bodyData = DeleteAlarmResp;

	//send Socket
	msgSendSocketData(&socketData, pNode);

}




/********************************************************************************
Request:		|09| Pin_num|Num|Flag|Start_hour| Start_min | Stop_hour |Stop_min|Time|
Response:	|09| Result |

********************************************************************************/
static U8 USER_FUNC lum_getAbsenceIndexOffset(U8 pinNum)
{
	U8 offset;


	if(pinNum == SWITCH_PIN_1)
	{
		offset = 0;
	}
#ifdef TWO_SWITCH_SUPPORT
	else if(pinNum == SWITCH_PIN_2)
	{
		offset = MAX_ABSENCE_COUNT;
	}
#endif
	else
	{
		lumi_error("Absence pinNum error pinNum=%d\n", pinNum);
		offset = 0;
	}
	return offset;
}


void USER_FUNC rebackSetAbsenceData(MSG_NODE* pNode)
{
	ASBENCE_DATA_INFO* pAbsenceInfo;
	U8 absenceIndex;
	U8 SetAbsenceResp[10];
	CREATE_SOCKET_DATA socketData;
	U16 index = 0;
	U8 pinNum;
	U8 indexOffset;


	memset(SetAbsenceResp, 0, sizeof(SetAbsenceResp));

	//Save absence data
	pinNum = pNode->nodeBody.pData[SOCKET_HEADER_LEN + 1];
	absenceIndex = pNode->nodeBody.pData[SOCKET_HEADER_LEN + 2];
	pAbsenceInfo = (ASBENCE_DATA_INFO*)(pNode->nodeBody.pData + SOCKET_HEADER_LEN + 3);

	indexOffset = lum_getAbsenceIndexOffset(pinNum);
	setAbsenceData(pAbsenceInfo, absenceIndex - 1 + indexOffset);

	//Set reback socket body
	SetAbsenceResp[index] = MSG_CMD_SET_ABSENCE_DATA;
	index += 1;
	SetAbsenceResp[index] = REBACK_SUCCESS_MESSAGE;
	index += 1;

	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.bodyLen = index;
	socketData.bodyData = SetAbsenceResp;

	//send Socket
	msgSendSocketData(&socketData, pNode);
}




/********************************************************************************
Request:		|0A | Pin_num|Num|
Response:	|0A| Pin_num|Num|Flag|Start_hour|Start_min| Stop_hour |Stop_min|Time|…|

********************************************************************************/
void USER_FUNC rebackGetAbsenceData(MSG_NODE* pNode)
{
	U8 absenceIndex;
	ASBENCE_DATA_INFO* pAbsenceInfo;
	U8 GetAbsenceResp[100];  //(8)*MAX_ABSENCE_COUNT + 2+1
	CREATE_SOCKET_DATA socketData;
	U16 index = 0;
	U8 i;
	U8 pinNum;
	U8 indexOffset;
	U8 startIndex;
	U8 endIndex;


	memset(GetAbsenceResp, 0, sizeof(GetAbsenceResp));

	pinNum = pNode->nodeBody.pData[SOCKET_HEADER_LEN + 1];
	absenceIndex = pNode->nodeBody.pData[SOCKET_HEADER_LEN + 2];
	indexOffset = lum_getAbsenceIndexOffset(pinNum);

	//Set reback socket body
	GetAbsenceResp[index] = MSG_CMD_GET_ABSENCE_DATA;
	index = 1;
	
	GetAbsenceResp[index] = pinNum; //pinNum
	index += 1;

	if(absenceIndex == 0)
	{
		startIndex = 1 + indexOffset;
		endIndex = MAX_ABSENCE_COUNT + indexOffset;
		for(i=startIndex; i<=endIndex; i++)
		{
			pAbsenceInfo = getAbsenceData(i - 1);
			//if(pAbsenceInfo->startHour != INVALID_ALARM_FLAG)
			{			
				GetAbsenceResp[index] = i - indexOffset; //Num
				index += 1;

				memcpy((GetAbsenceResp + index), pAbsenceInfo, sizeof(ASBENCE_DATA_INFO));
				index += sizeof(ASBENCE_DATA_INFO);
				//showHexData("Absence ", (GetAbsenceResp + absenceIndex), (sizeof(ASBENCE_DATA_INFO)+1));
			}
		}
	}
	else
	{
		pAbsenceInfo = getAbsenceData(absenceIndex - 1 + indexOffset);
		
		GetAbsenceResp[index] = absenceIndex; //Num
		index += 1;

		memcpy((GetAbsenceResp + index), pAbsenceInfo, sizeof(ASBENCE_DATA_INFO));
		index += sizeof(ASBENCE_DATA_INFO);
	}

	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.bodyLen = index;
	socketData.bodyData = GetAbsenceResp;

	//send Socket
	msgSendSocketData(&socketData, pNode);
}



/********************************************************************************
Request:		| 0B | Pin_num|Num|
Response:	|0B|Result|

********************************************************************************/
void USER_FUNC rebackDeleteAbsenceData(MSG_NODE* pNode)
{
	U8 absenceIndex;
	U8 DeleteAbsenceResp[10];
	CREATE_SOCKET_DATA socketData;
	U16 index = 0;
	U8 pinNum;
	U8 indexOffset;


	memset(DeleteAbsenceResp, 0, sizeof(DeleteAbsenceResp));

	pinNum = pNode->nodeBody.pData[SOCKET_HEADER_LEN + 1];
	absenceIndex = pNode->nodeBody.pData[SOCKET_HEADER_LEN + 2];
	indexOffset = lum_getAbsenceIndexOffset(pinNum);
	deleteAbsenceData((absenceIndex - 1 + indexOffset), TRUE);

	//Set reback socket body
	DeleteAbsenceResp[index] = MSG_CMD_DELETE_ABSENCE_DATA;
	index += 1;
	DeleteAbsenceResp[index] = REBACK_SUCCESS_MESSAGE;
	index += 1;

	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.bodyLen = index;
	socketData.bodyData = DeleteAbsenceResp;

	//send Socket
	msgSendSocketData(&socketData, pNode);
}



/********************************************************************************
Request:		|0C| Pin_num|Num|Flag|Stop_time|Pin|
Response:	|0C|Result|

********************************************************************************/
static U8 USER_FUNC lum_getCountdownIndexOffset(U8 pinNum)
{
	U8 offset;


	if(pinNum == SWITCH_PIN_1)
	{
		offset = 0;
	}
#ifdef TWO_SWITCH_SUPPORT
	else if(pinNum == SWITCH_PIN_2)
	{
		offset = MAX_COUNTDOWN_COUNT;
	}
#endif
	else
	{
		lumi_error("Absence pinNum error pinNum=%d\n", pinNum);
		offset = 0;
	}
	return offset;
}


void USER_FUNC rebackSetCountDownData(MSG_NODE* pNode)
{
	COUNTDOWN_DATA_INFO countDownData;
	U8 countDownIndex;
	GPIO_STATUS* pGpioStatus;
	U8* pData;
	U32 count;
	U8 SetcountDownResp[10];
	CREATE_SOCKET_DATA socketData;
	U16 index = 0;
	U8 pinNum;
	U8 indexOffset;


	memset(SetcountDownResp, 0, sizeof(SetcountDownResp));

	//Save countDown data
	pinNum = pNode->nodeBody.pData[SOCKET_HEADER_LEN + 1];
	pData = pNode->nodeBody.pData + SOCKET_HEADER_LEN + 2;

	indexOffset = lum_getCountdownIndexOffset(pinNum);
	countDownIndex = pData[0];
	memcpy(&countDownData.flag, (pData + 1), sizeof(COUNTDOWN_FLAG));
	memcpy(&count, (pData + 2), sizeof(U32));
	countDownData.count = ntohl(count);
	pGpioStatus = (GPIO_STATUS*)(pData + 6);
	countDownData.action = (pGpioStatus->duty == 0xFF)?SWITCH_OPEN:SWITCH_CLOSE;

	//Set reback socket body
	SetcountDownResp[index] = MSG_CMD_SET_COUNDDOWN_DATA;
	index += 1;

	//check countdown is avalid
	count = lum_getSystemTime();
	if(count >= countDownData.count)
	{
		SetcountDownResp[index] = REBACK_FAILD_MESSAGE;
	}
	else
	{
		setCountDownData(&countDownData, (countDownIndex - 1 + indexOffset));
		SetcountDownResp[index] = REBACK_SUCCESS_MESSAGE;
	}
	index += 1;

	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.bodyLen = index;
	socketData.bodyData = SetcountDownResp;

	//send Socket
	msgSendSocketData(&socketData, pNode);

}




/********************************************************************************
Request:		|0D| Pin_num|Num|
Response:	|0D| Pin_num|Num|Flag|Stop_time|Pin|…|

********************************************************************************/
static U8 USER_FUNC fillCountDownRebackData(U8* pdata, U8 countDownIndex, U8 indexOffset)
{

	COUNTDOWN_DATA_INFO* pCountDownData;
	U32* pData;
	GPIO_STATUS gpioStatus;
	U16 index = 0;


	pCountDownData = getCountDownData(countDownIndex);
	if(pCountDownData == NULL)
	{
		return index;
	}
	pdata[index] = countDownIndex + 1 - indexOffset; //set Num
	index += 1;

	memcpy((pdata + index), &pCountDownData->flag, sizeof(COUNTDOWN_FLAG)); //set Flag
	index += sizeof(COUNTDOWN_FLAG);

	pData = (U32*)(pdata + index);
	pData[0] = htonl(pCountDownData->count); // Set stop time
	index += sizeof(U32);

	memset(&gpioStatus, 0, sizeof(GPIO_STATUS));
	gpioStatus.duty = (pCountDownData->action == SWITCH_OPEN)?0xFF:0;
	gpioStatus.res = 0xFF;


#ifdef TWO_SWITCH_SUPPORT
	if(indexOffset == MAX_COUNTDOWN_COUNT)
	{
		gpioStatus.flag = 1;
	}
#endif
	memcpy((pdata + index), &gpioStatus, sizeof(GPIO_STATUS)); //pin
	index += sizeof(GPIO_STATUS);

	//showHexData("CountDown ", pdata, index);

	return index;
}



void USER_FUNC rebackGetCountDownData(MSG_NODE* pNode)
{
	U8 countDownIndex;
	U8 GetCountDownResp[20];
	CREATE_SOCKET_DATA socketData;
	U16 index = 0;
	U8 i;
	U8 pinNum;
	U8 indexOffset;
	U8 startIndex;
	U8 endIndex;


	memset(GetCountDownResp, 0, sizeof(GetCountDownResp));

	//Get data
	pinNum =  pNode->nodeBody.pData[SOCKET_HEADER_LEN + 1];
	countDownIndex = pNode->nodeBody.pData[SOCKET_HEADER_LEN + 2];
	indexOffset = lum_getCountdownIndexOffset(pinNum);

	//Set reback socket body
	GetCountDownResp[index] = MSG_CMD_GET_COUNTDOWN_DATA; //set CMD
	index += 1;

	GetCountDownResp[index] = pinNum; //set pinNum
	index += 1;
	
	if(countDownIndex == 0)
	{
		startIndex = indexOffset;
		endIndex = MAX_COUNTDOWN_COUNT + indexOffset;
		for (i=startIndex; i<endIndex; i++)
		{
			index += fillCountDownRebackData((GetCountDownResp + index), i, indexOffset);
		}
	}
	else
	{
		index += fillCountDownRebackData((GetCountDownResp + index), (countDownIndex - 1 + indexOffset), indexOffset);
	}

	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.bodyLen = index;
	socketData.bodyData = GetCountDownResp;

	//send Socket
	msgSendSocketData(&socketData, pNode);
}




/********************************************************************************
Request:		|0E| Pin_num|Num|
Response:	|0E|Result||

********************************************************************************/
void USER_FUNC rebackDeleteCountDownData(MSG_NODE* pNode)
{
	U8 countDownIndex;
	U8 DeleteCountDownResp[10];
	CREATE_SOCKET_DATA socketData;
	U16 index = 0;
	U8 pinNum;
	U8 indexOffset;


	memset(DeleteCountDownResp, 0, sizeof(DeleteCountDownResp));

	pinNum = pNode->nodeBody.pData[SOCKET_HEADER_LEN + 1];
	countDownIndex = pNode->nodeBody.pData[SOCKET_HEADER_LEN + 2];
	indexOffset = lum_getCountdownIndexOffset(pinNum);
	deleteCountDownData(countDownIndex - 1 + indexOffset);

	//Set reback socket body
	DeleteCountDownResp[index] = MSG_CMD_DELETE_COUNTDOWN_DATA;
	index += 1;
	DeleteCountDownResp[index] = REBACK_SUCCESS_MESSAGE;
	index += 1;

	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.bodyLen = index;
	socketData.bodyData = DeleteCountDownResp;

	//send Socket
	msgSendSocketData(&socketData, pNode);
}



/********************************************************************************
UserRequest:		| 81 |
Server Response:	| 81 | IP Address | Port |

********************************************************************************/
void USER_FUNC localGetServerAddr(MSG_NODE* pNode)
{
	U8 data;
	CREATE_SOCKET_DATA socketData;

	data = MSG_CMD_GET_SERVER_ADDR;

	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 0;
	socketData.bodyLen = 1;
	socketData.bodyData = &data;

	pNode->nodeBody.msgOrigin = MSG_FROM_TCP;

	//send Socket
	msgSendSocketData(&socketData, pNode);
}



void USER_FUNC rebackGetServerAddr(MSG_NODE* pNode)
{
	SOCKET_ADDR socketAddr;

	//socketAddr.ipAddr = ntohl(*((U32*)(pNode->nodeBody.pData + SOCKET_HEADER_LEN + 1)));
	//socketAddr.port= ntohs(*((U16*)(pNode->nodeBody.pData + SOCKET_HEADER_LEN + 1 + sizeof(U32))));
	socketAddr.ipAddr = *((U32*)(pNode->nodeBody.pData + SOCKET_HEADER_LEN + 1));
	socketAddr.port= *((U16*)(pNode->nodeBody.pData + SOCKET_HEADER_LEN + 1 + sizeof(U32)));
	afterGetServerAddr(&socketAddr);
	deleteRequstSendNode(pNode->nodeBody.snIndex);
}



/********************************************************************************
User Request:		| 42 |
Server Response:	| 42 | Key Len | Key |

********************************************************************************/
void USER_FUNC localRequstConnectServer(MSG_NODE* pNode)
{
	U8 data;
	CREATE_SOCKET_DATA socketData;

	data = MSG_CMD_REQUST_CONNECT;

	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 0;
	socketData.bodyLen = 1;
	socketData.bodyData = &data;

	pNode->nodeBody.msgOrigin = MSG_FROM_TCP;

	//send Socket
	msgSendSocketData(&socketData, pNode);
}


void USER_FUNC rebackRequstConnectServer(MSG_NODE* pNode)
{
	U8* pAesKey;
	//U8 keyLen;


	//keyLen = pNode->nodeBody.pData[SOCKET_HEADER_LEN + 1];
	pAesKey = pNode->nodeBody.pData + SOCKET_HEADER_LEN + 2;
	lumi_debug("keyLen=%d key=%s\n",  pNode->nodeBody.pData[SOCKET_HEADER_LEN + 1], pAesKey);
	setServerAesKey(pAesKey);
	deleteRequstSendNode(pNode->nodeBody.snIndex);
	lum_AfterConnectServer();

}




/********************************************************************************
Request:		| 06 | Pin |…|
Response:	| 06 | Pin |…|

********************************************************************************/
void USER_FUNC reportGpioChangeEvent(MSG_NODE* pNode)
{
	U8 gpioChangeData[10];
	GPIO_STATUS* pGioStatus;
	CREATE_SOCKET_DATA socketData;
	U16 index = 0;
	GPIO_CHANGE_REPORT* pReportData;


	memset(gpioChangeData, 0, sizeof(gpioChangeData));


	//Set reback socket body
	gpioChangeData[index] = MSG_CMD_REPORT_GPIO_CHANGE;
	index += 1;

	pReportData = (GPIO_CHANGE_REPORT*)pNode->nodeBody.pData;
	pGioStatus = (GPIO_STATUS*)(gpioChangeData + index);
	pGioStatus->duty = (pReportData->action == SWITCH_OPEN)?0xFF:0;
	pGioStatus->flag = (U8)(pReportData->pinFlag);
	pGioStatus->res = 0xFF;
	index += sizeof(GPIO_STATUS);

	lumi_debug("reportGpioChangeEvent gpioStatus = %d\n", pNode->nodeBody.pData[0]);
	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 0;
	socketData.bodyLen = index;
	socketData.bodyData = gpioChangeData;

	//send Socket	
	pNode->nodeBody.msgOrigin = MSG_FROM_UDP;
	pNode->nodeBody.socketIp = getBroadcastAddr();
	msgSendSocketData(&socketData, pNode);

	pNode->nodeBody.msgOrigin = MSG_FROM_TCP;
	msgSendSocketData(&socketData, pNode);
}


void USER_FUNC rebackReportGpioChange(MSG_NODE* pNode)
{
	deleteRequstSendNode(pNode->nodeBody.snIndex);
}



#if 0

/********************************************************************************
Request:		| 07 | Pin_num|Num | Flag | Hour | Min | Pin |
Response:	| 07 | Pin_num|Num |

********************************************************************************/
void USER_FUNC reportAlarmArrivedEvent(MSG_NODE* pNode)
{
	U8 alarmData[15];  //(4+4)*MAX_ALARM_COUNT + 2+1
	CREATE_SOCKET_DATA socketData;
	U16 index = 0;


	memset(alarmData, 0, sizeof(alarmData));

	//Set reback socket body
	alarmData[index] = MSG_CMD_REPORT_ALARM_CHANGE;
	index += 1;
	
	alarmData[index] = 0x0;
	index += 1;

	index += fillAlarmRebackData((alarmData + index), pNode->nodeBody.pData[0]);

	lumi_debug("reportAlarmArrivedEvent alarmIndex = %d\n", pNode->nodeBody.pData[0]);
	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.bodyLen = index;
	socketData.bodyData = alarmData;

	//send Socket
	pNode->nodeBody.msgOrigin = MSG_FROM_TCP;
	msgSendSocketData(&socketData, pNode);

	pNode->nodeBody.msgOrigin = MSG_FROM_UDP;
	pNode->nodeBody.socketIp = getBroadcastAddr();
	msgSendSocketData(&socketData, pNode);

}


void USER_FUNC rebackReportAlarmArrived(MSG_NODE* pNode)
{
	deleteRequstSendNode(pNode->nodeBody.snIndex);
}
#endif



/********************************************************************************
Device Request: | 43 | Username |
Server Response: | 43 | Result |

********************************************************************************/

void USER_FUNC localRequstFactoryDataReset(MSG_NODE* pNode)
{
	U8 data[100];
	U8 index;
	U8 userNamelen;
	U8* userName;
	CREATE_SOCKET_DATA socketData;;


	memset(data, 0, sizeof(data));
	data[0] = MSG_CMD_DEVICE_RESET_FACTORY;
	index = 1;

	userName = lum_getUserName();
	userNamelen = strlen((S8*)userName);
	userNamelen = (userNamelen >= MAX_USER_NAME_LEN)?(MAX_USER_NAME_LEN-2):userNamelen;
	memcpy((data+index), userName, userNamelen);
	index += userNamelen;
	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 0;
	socketData.bodyLen = index;
	socketData.bodyData = data;

	pNode->nodeBody.msgOrigin = MSG_FROM_TCP;

	//send Socket
	msgSendSocketData(&socketData, pNode);
}


void USER_FUNC lum_replyFactoryDataReset(MSG_NODE* pNode)
{
	lum_clearFactoryResetFlag();
	lum_stopFactoryResetTimer();
}


/********************************************************************************
Request: | 67 |
Response: | 67 | Result |

********************************************************************************/
void USER_FUNC lum_appResetFactory(MSG_NODE* pNode)
{
	U8 data[10];
	CREATE_SOCKET_DATA socketData;
	U16 index = 0;


	memset(data, 0, sizeof(data));
	data[index] = MSG_CMD_APP_RESET_FACTORY;
	index++;
	data[index] = REBACK_SUCCESS_MESSAGE;
	index++;

	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.bodyLen = index;
	socketData.bodyData = data;

	//send Socket
	msgSendSocketData(&socketData, pNode);
	//lum_deviceFactoryReset(FALSE);
	insertLocalMsgToList(MSG_LOCAL_EVENT, NULL, 0, MSG_CMD_LOCAL_RESET_FACTORY);
}


#ifdef RN8209C_SUPPORT

/********************************************************************************
Device Request:|07|
Server Response:|07|V | I |P |U|

********************************************************************************/
static void USER_FUNC lum_EnergyDataTimerCallback( hftimer_handle_t htimer )
{
	insertLocalMsgToList(MSG_LOCAL_EVENT, NULL, 0, MSG_CMD_REPORT_ENERGY_DATA);
	lum_startReportEnergyUdataTimer(RESEND_ENERGY_DATA_TIMER_GAP);
}


void USER_FUNC lum_startReportEnergyUdataTimer(S32 timerGap)
{
	static hftimer_handle_t reportEnergyTimer = NULL;


	if(reportEnergyTimer == NULL)
	{
		reportEnergyTimer = hftimer_create("ReportEnergyDataTimer", timerGap, false, REPORT_ENERGY_DATA_TIMER_ID, lum_EnergyDataTimerCallback, 0);
	}
	hftimer_change_period(reportEnergyTimer, timerGap);
}


static U32 USER_FUNC lum_coverBcdfFormat(U32 data)
{
	U16 bcdData;
	U32 tmpData;
	U8 i;


	bcdData = 0;
	tmpData = data;
	for(i=0; i<8; i++)
	{
		bcdData += (tmpData%10)<<(i*4);
		tmpData /= 10;
	}
	lumi_debug("bcdData=%X\n", bcdData);
	return bcdData;
}


static U8 USER_FUNC lum_fillEnergyData(U8* pData)
{
	MeatureEnergyData energyData;
	U16 energyI;
	U16 energyV;
	U32 energyP;
	U32 energyU;
	U8 index = 0;
	U16* pTmp;
	U32* pTmp2;


	lum_rn8209cGetIVPData(&energyData);
	energyI = (U16)lum_coverBcdfFormat((U32)energyData.irms);
	energyV = (U16)lum_coverBcdfFormat((U32)energyData.urms);
	energyP = lum_coverBcdfFormat(energyData.powerP);
	energyU = lum_coverBcdfFormat(energyData.energyU);

	pTmp = (U16*)(pData + index);
	pTmp[0] = htons(energyV);
	index += 2;

	pTmp = (U16*)(pData + index);
	pTmp[0] = htons(energyI);
	index += 2;

	pTmp2 = (U32*)(pData + index);
	pTmp2[0] = htonl(energyP);
	index += 4;

	pTmp2 = (U32*)(pData + index);
	pTmp2[0] = htonl(energyU);
	index += 4;

	return index;
}


void USER_FUNC lum_queryEnergyData(MSG_NODE* pNode)
{
	U8 energData[20];
	CREATE_SOCKET_DATA socketData;
	U16 index = 0;


	memset(energData, 0, sizeof(energData));

	energData[0] = MSG_CMD_QUERY_ENERGY_DATA;
	index++;

	index += lum_fillEnergyData(energData + 1);

	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 1;
	socketData.bodyLen = index;
	socketData.bodyData = energData;

	//send Socket
	msgSendSocketData(&socketData, pNode);
}



/********************************************************************************
Device Request:|08|U|
Server Response:|08|Result |

********************************************************************************/

void USER_FUNC lum_localReportEnergyUdata(MSG_NODE* pNode)
{
	U8 data[10];
	U8 index = 0;
	U32 energyUdata;
	U32* pTmp;
	CREATE_SOCKET_DATA socketData;;


	memset(data, 0, sizeof(data));
	data[0] = MSG_CMD_REPORT_ENERGY_DATA;
	index ++;

	energyUdata = lum_coverBcdfFormat(lum_rn8209cGetUData());
	pTmp = (U32*)(data + 1);
	pTmp[0] = htonl(energyUdata);
	index += 4;
	

	//fill socket data
	socketData.bEncrypt = 1;
	socketData.bReback = 0;
	socketData.bodyLen = index;
	socketData.bodyData = data;

	pNode->nodeBody.msgOrigin = MSG_FROM_TCP;

	//send Socket
	msgSendSocketData(&socketData, pNode);
	lum_startReportEnergyUdataTimer(RESEND_ENERGY_DATA_TIMER_GAP);
}


void USER_FUNC lum_replyEnergyUdata(MSG_NODE* pNode)
{
	U8 ret;


	ret = pNode->nodeBody.pData[SOCKET_HEADER_LEN + 1];
	if(ret == REBACK_SUCCESS_MESSAGE)
	{
		lum_startReportEnergyUdataTimer(REPORT_ENERGY_DATA_TIMER_GAP);
	}
}


#ifdef LUM_READ_ENERGY_TEST
void USER_FUNC lum_showEnergyData(void)
{
	MeatureEnergyData meatureData;

	lum_rn8209cGetIVPData(&meatureData);
}
#endif //LUM_READ_ENERGY_TEST
#endif //RN8209C_SUPPORT
#endif

