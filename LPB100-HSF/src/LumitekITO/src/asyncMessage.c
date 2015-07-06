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

#include "../inc/itoCommon.h"
#include "../inc/asyncMessage.h"
#include "../inc/messageDispose.h"
#include "../inc/localSocketUdp.h"
#include "../inc/serverSocketTcp.h"
#include "../inc/socketSendList.h"
#include "../inc/deviceMisc.h"
#include "../inc/lumTimeData.h"
#include "../inc/lumLog.h"
#ifdef SX1208_433M_SUPPORT
#include "../inc/lum_sx1208.h"
#endif
#ifdef EXTRA_SWITCH_SUPPORT
#include "../inc/deviceGpio.h"
#endif


//static MSG_NODE* g_pHeader = NULL;
static LIST_HEADER g_list_header;

hfthread_mutex_t g_message_mutex;




static void USER_FUNC messageListInit(void)
{
	g_list_header.firstNodePtr = NULL;
	g_list_header.noteCount = 0;

	if((hfthread_mutext_new(&g_message_mutex)!= HF_SUCCESS))
	{
		HF_Debug(DEBUG_ERROR, "failed to create g_message_mutex");

	}
}


static void USER_FUNC insertListNode(BOOL insetToHeader, MSG_NODE* pNode)
{
	LIST_HEADER* pListHeader = &g_list_header;
	MSG_NODE* pTempNode;


	hfthread_mutext_lock(g_message_mutex);

	pTempNode = pListHeader->firstNodePtr;
	if(pListHeader->noteCount == 0)
	{
		pListHeader->firstNodePtr = pNode;
		pNode->pNodeNext = NULL;
	}
	else
	{
		if(insetToHeader)
		{
			pNode->pNodeNext = pTempNode->pNodeNext;
			pListHeader->firstNodePtr = pNode;
		}
		else
		{
			while(pTempNode->pNodeNext != NULL)
			{
				pTempNode = pTempNode->pNodeNext;
			}
			pTempNode->pNodeNext = pNode;
			pNode->pNodeNext = NULL;
		}
	}
	pListHeader->noteCount++;
	hfthread_mutext_unlock(g_message_mutex);
}



static void USER_FUNC freeNodeMemory(MSG_NODE* pNode)
{
	if(pNode->nodeBody.pData != NULL)
	{
		FreeSocketData(pNode->nodeBody.pData);
		pNode->nodeBody.pData = NULL;
	}
	FreeSocketData((U8*)pNode);
}




static BOOL USER_FUNC deleteListNode(MSG_NODE* pNode)
{
	LIST_HEADER* pListHeader = &g_list_header;
	MSG_NODE* curNode;
	MSG_NODE* pTempNode;
	BOOL ret = FALSE;


	if(pListHeader->firstNodePtr == NULL || pNode == NULL)
	{
		HF_Debug(DEBUG_ERROR, "meiyusong===> deleteListNode error no node to delete\n");
		hfthread_mutext_unlock(g_message_mutex);
		return FALSE;
	}

	if(pNode == pListHeader->firstNodePtr)
	{
		pListHeader->firstNodePtr = pNode->pNodeNext;
		ret = TRUE;
	}
	else
	{
		curNode = pListHeader->firstNodePtr;
		pTempNode = curNode->pNodeNext;
		while(pTempNode != NULL)
		{
			if(pTempNode == pNode)
			{
				curNode->pNodeNext = pNode->pNodeNext;
				ret = TRUE;
				break;
			}
			else
			{
				curNode = curNode->pNodeNext;
				pTempNode = pTempNode->pNodeNext;
			}
		}
	}
	if(ret)
	{
		pListHeader->noteCount--;
		freeNodeMemory(pNode);
	}
	else
	{
		HF_Debug(DEBUG_ERROR, "meiyusong===> deleteListNode not found \n");
	}
	hfthread_mutext_unlock(g_message_mutex);
	return ret;
}



BOOL USER_FUNC insertSocketMsgToList(MSG_ORIGIN msgOrigin, U8* pData, U32 dataLen, U32 socketIp)
{
	U8* pSocketData;
	MSG_NODE* pMsgNode;
	SCOKET_HERADER_OUTSIDE* pOutSide;
	BOOL ret = FALSE;
	U32 aesDataLen = dataLen;


	if(msgOrigin == MSG_FROM_UDP || msgOrigin == MSG_FROM_TCP)
	{
		if(!lum_checkSocketBeforeAES(dataLen, pData))
		{
			return ret;
		}
		pSocketData = encryptRecvSocketData(msgOrigin, pData, &aesDataLen);
		pOutSide = (SCOKET_HERADER_OUTSIDE*)pSocketData;
		if(pSocketData == NULL)
		{
			return ret;
		}
		else if(!lum_checkSocketAfterAES(pSocketData))
		{
			FreeSocketData(pSocketData);
			return ret;
		}
#if defined(SAVE_LOG_TO_FLASH) || defined(LUM_UART_SOCKET_LOG) || defined(LUM_UDP_SOCKET_LOG)
		saveSocketData(TRUE, msgOrigin, pSocketData, aesDataLen);
#endif
#if 0
		else if(pOutSide->openData.flag.bReback != 0)
		{
			// add something
			freeNodeMemory(pMsgNode);
			return ret;
		}

		lumi_debug("CMD=0x%X \n", pSocketData[SOCKET_HEADER_LEN]);
		if(msgOrigin == MSG_FROM_UDP)
		{
			showHexData("UDP Recv", pSocketData, aesDataLen);
		}
		else if(msgOrigin == MSG_FROM_TCP)
		{
			showHexData("TCP Recv", pSocketData, aesDataLen);
		}
#endif


		pMsgNode = (MSG_NODE*)mallocSocketData(sizeof(MSG_NODE));
		if(pMsgNode == NULL)
		{
			HF_Debug(DEBUG_ERROR, "insertSocketMsgToList malloc faild \n");
			return ret;
		}
		pMsgNode->nodeBody.cmdData = pSocketData[SOCKET_HEADER_LEN];
		pMsgNode->nodeBody.bReback = pOutSide->openData.flag.bReback;
		pMsgNode->nodeBody.snIndex = pOutSide->snIndex;
		pMsgNode->nodeBody.pData = pSocketData;
		pMsgNode->nodeBody.dataLen = aesDataLen;
		pMsgNode->nodeBody.msgOrigin = msgOrigin;
		pMsgNode->nodeBody.socketIp = socketIp;

		insertListNode(FALSE, pMsgNode);
		ret = TRUE;

	}
	return ret;
}



BOOL USER_FUNC insertLocalMsgToList(MSG_ORIGIN msgOrigin, U8* pData, U32 dataLen, U16 cmdData)
{
	MSG_NODE* pMsgNode;
	U8* localData;
	BOOL ret = FALSE;


	pMsgNode = (MSG_NODE*)mallocSocketData(sizeof(MSG_NODE));
	if(pMsgNode == NULL)
	{
		HF_Debug(DEBUG_ERROR, "meiyusong===> insertLocalMsgToList malloc faild \n");
		return ret;
	}
	pMsgNode->nodeBody.cmdData = cmdData;
	pMsgNode->nodeBody.msgOrigin = msgOrigin;

	if(pData != NULL)
	{
		pMsgNode->nodeBody.dataLen = dataLen;
		localData = mallocSocketData(dataLen + 1);
		if(localData == NULL)
		{
			FreeSocketData((U8*)pMsgNode);
			return ret;
		}
		memcpy(localData, pData, dataLen);
		pMsgNode->nodeBody.pData = localData;
		ret = TRUE;
	}

	insertListNode(FALSE, pMsgNode);
	return ret;
}


#ifdef LUMITEK_DEBUG_SWITCH

S8* USER_FUNC getMsgComeFrom(MSG_ORIGIN msgOrigin)
{
	if(msgOrigin == MSG_LOCAL_EVENT)
	{
		return "Local";
	}
	else if(msgOrigin == MSG_FROM_UDP)
	{
		return "UDP";
	}
	
	else if(msgOrigin == MSG_FROM_TCP)
	{
		return "TCP";
	}
	else
	{
		return "Unknow";
	}

}


#if 0
static S8* USER_FUNC getMsgName(U16 cmdData)
{
	S8* pName;

	switch(cmdData)
	{
		case MSG_CMD_SET_GPIO_STATUS:
			pName = "SET_GPIO_STATUS";
			break;
			
		case MSG_CMD_GET_GPIO_STATUS:
			pName = "GET_GPIO_STATUS";
			break;
			
		//Alarm data
		case MSG_CMD_SET_ALARM_DATA:
			pName = "SET_ALARM_DATA";
			break;
			
		case MSG_CMD_GET_ALARM_DATA:
			pName = "GET_ALARM_DATA";
			break;
			
		case MSG_CMD_DELETE_ALARM_DATA:
			pName = "DELETE_ALARM_DATA";
			break;
			
		//Report data
		case MSG_CMD_REPORT_GPIO_CHANGE:
			pName = "REPORT_GPIO_CHANGE";
			break;
			
		//case MSG_CMD_REPORT_ALARM_CHANGE:
		//	pName = "REPORT_ALARM_CHANGE";
		//	break;
			
		//Against thief
		case MSG_CMD_SET_ABSENCE_DATA:
			pName = "SET_ABSENCE_DATA";
			break;
			
		case MSG_CMD_GET_ABSENCE_DATA:
			pName = "GET_ABSENCE_DATA";
			break;
			
		case MSG_CMD_DELETE_ABSENCE_DATA:
			pName = "DELETE_ABSENCE_DATA";
			break;
			
		//stop watch
		case MSG_CMD_SET_COUNDDOWN_DATA:
			pName = "SET_COUNDDOWN_DATA";
			break;
			
		case MSG_CMD_GET_COUNTDOWN_DATA:
			pName = "GET_COUNTDOWN_DATA";
			break;
			
		case MSG_CMD_DELETE_COUNTDOWN_DATA:
			pName = "DELETE_COUNTDOWN_DATA";
			break;
			


		case MSG_CMD_FOUND_DEVICE:
			pName = "FOUND_DEVICE";
			break;
			
		case MSG_CMD_LOCK_DEVICE:
			pName = "LOCK_DEVICE";
			break;
			

		case MSG_CMD_GET_SERVER_ADDR:
			pName = "GET_SERVER_ADDR";
			break;
			
		case MSG_CMD_REQUST_CONNECT:
			pName = "REQUST_CONNECT";
			break;
			

		case MSG_CMD_HEART_BEAT:
			pName = "HEART_BEAT";
			break;
			

		case MSG_CMD_QUARY_MODULE_INFO:
			pName = "QUARY_MODULE_INFO";
			break;
			
		case MSG_CMD_SET_MODULE_NAME:
			pName = "SET_MODULE_NAME";
			break;
			
		case MSG_CMD_MODULE_UPGRADE:
			pName = "MODULE_UPGRADE";
			break;
			

		case MSG_CMD_ENTER_SMART_LINK:
			pName = "ENTER_SMART_LINK";
			break;
			

		//Local message start from 0xE1
		case MSG_CMD_LOCAL_ENTER_SMARTLINK:
			pName = "LOCAL_ENTER_SMARTLINK";
			break;
			
		case MSG_CMD_LOCAL_GET_UTC_TIME:
			pName = "GET_UTC_TIME";
			break;

		default:
			pName = "Nuknow MSG";
			break;
	}
	return pName;
}
#endif
#endif


#ifdef EXTRA_SWITCH_SUPPORT
static S32 g_oldExtraketStatus;
static U8 keyStillCount;

#define MAX_KEY_STILL_COUNT		10


static void USER_FUNC lum_extraKeyInit(void)
{
	hfgpio_configure_fpin(HFGPIO_F_EXTRA_SWITCH, HFM_IO_TYPE_INPUT);
	keyStillCount = 0;
	g_oldExtraketStatus = hfgpio_fpin_is_high(HFGPIO_F_EXTRA_SWITCH);
#if 0
	if(g_oldExtraketStatus)
	{
		hfgpio_fset_out_high(HFGPIO_F_TEST);
	}
	else
	{
		hfgpio_fset_out_low(HFGPIO_F_TEST);
	}
#endif
}


static void USER_FUNC lum_checkExtraKey(void)
{
	S32 curExtraKeyStatus;


	curExtraKeyStatus = hfgpio_fpin_is_high(HFGPIO_F_EXTRA_SWITCH);
	if(curExtraKeyStatus != g_oldExtraketStatus)
	{
		keyStillCount = 0;
		g_oldExtraketStatus = curExtraKeyStatus;
#if 0
		if(curExtraKeyStatus)
		{
			hfgpio_fset_out_high(HFGPIO_F_TEST);
		}
		else
		{
			hfgpio_fset_out_low(HFGPIO_F_TEST);
		}
#endif
	}
	else
	{
		if(keyStillCount < MAX_KEY_STILL_COUNT)
		{
			keyStillCount++;
		}
		else if(keyStillCount == MAX_KEY_STILL_COUNT)
		{
			changeSwitchStatus(SWITCH_PIN_1);
			keyStillCount++;;
		}
	}
}
#endif


BOOL USER_FUNC needWaitSocketReback(U8 cmdData)
{
	BOOL ret = TRUE;


	switch(cmdData)
	{
		case MSG_CMD_REPORT_GPIO_CHANGE:
			ret = FALSE;
			break;

		case MSG_CMD_REPORT_ENERGY_DATA:
			ret = FALSE;
			break;

		case MSG_CMD_DEVICE_RESET_FACTORY:
			ret = FALSE;
			break;

		case MSG_CMD_GET_CALIBRATE_DATA:
			ret = FALSE;
			break;
			
		default:
			break;
	}
	return ret;
}


void USER_FUNC deviceMessageThread(void *arg)
{
	LIST_HEADER* listHeader = &g_list_header;
	MSG_NODE* curNode;
	S32 tcpSockFd;
	S32 udpSockFd;


	messageListInit();
#ifdef EXTRA_SWITCH_SUPPORT
	lum_extraKeyInit();
#endif
	hfthread_enable_softwatchdog(NULL,30); //Start watchDog
	while(1)
	{
		//lumi_debug(" deviceMessageThread \n");
		hfthread_reset_softwatchdog(NULL); //tick watchDog

		curNode = listHeader->firstNodePtr;
		if(curNode != NULL)
		{
#ifdef LUMITEK_DEBUG_SWITCH
			//lumi_debug("CMD====>0x%04x ==>%s (%s)\n", curNode->nodeBody.cmdData,
			//getMsgName(curNode->nodeBody.cmdData),
			//getMsgComeFrom(curNode->nodeBody.msgOrigin));
#endif
			switch(curNode->nodeBody.cmdData)
			{
			case MSG_CMD_FOUND_DEVICE:
				rebackFoundDevice(curNode);
				break;

			case MSG_CMD_HEART_BEAT:
				rebackHeartBeat(curNode);
				break;

			case MSG_CMD_QUARY_MODULE_INFO:
				rebackGetDeviceInfo(curNode);
				break;

			case MSG_CMD_SET_MODULE_NAME:
				rebackSetDeviceName(curNode);
				break;

			case MSG_CMD_MODULE_UPGRADE:
				if(curNode->nodeBody.msgOrigin == MSG_LOCAL_EVENT)
				{
					LocalGetDeviceUpgrade();
				}
				else
				{
					rebackGetDeviceUpgrade(curNode);
				}
				break;

			case MSG_CMD_ENTER_SMART_LINK:
				rebackEnterSmartLink(curNode);
				break;

			case MSG_CMD_LOCK_DEVICE:
				rebackLockDevice(curNode);
				break;

			case MSG_CMD_SET_GPIO_STATUS:
				rebackSetGpioStatus(curNode);
				break;

			case MSG_CMD_GET_GPIO_STATUS:
				rebackGetGpioStatus(curNode);
				break;

			case MSG_CMD_SET_ALARM_DATA:
				rebackSetAlarmData(curNode);
				break;

			case MSG_CMD_GET_ALARM_DATA:
				rebackGetAlarmData(curNode);
				break;

			case MSG_CMD_DELETE_ALARM_DATA:
				rebackDeleteAlarmData(curNode);
				break;

			case MSG_CMD_SET_ABSENCE_DATA:
				rebackSetAbsenceData(curNode);
				break;

			case MSG_CMD_GET_ABSENCE_DATA:
				rebackGetAbsenceData(curNode);
				break;

			case MSG_CMD_DELETE_ABSENCE_DATA:
				rebackDeleteAbsenceData(curNode);
				break;

			case MSG_CMD_SET_COUNDDOWN_DATA:
				rebackSetCountDownData(curNode);
				break;

			case MSG_CMD_GET_COUNTDOWN_DATA:
				rebackGetCountDownData(curNode);
				break;

			case MSG_CMD_DELETE_COUNTDOWN_DATA:
				rebackDeleteCountDownData(curNode);
				break;


			case MSG_CMD_GET_SERVER_ADDR:
				if(curNode->nodeBody.msgOrigin == MSG_LOCAL_EVENT)
				{
					localGetServerAddr(curNode);
				}
				else
				{
					rebackGetServerAddr(curNode);
				}
				break;

			case MSG_CMD_REQUST_CONNECT:
				if(curNode->nodeBody.msgOrigin == MSG_LOCAL_EVENT)
				{
					localRequstConnectServer(curNode);
				}
				else
				{
					rebackRequstConnectServer(curNode);
				}
				break;

			case MSG_CMD_REPORT_GPIO_CHANGE:
				if(curNode->nodeBody.msgOrigin == MSG_LOCAL_EVENT)
				{
					reportGpioChangeEvent(curNode);
				}
				else
				{
					rebackReportGpioChange(curNode);
				}
				break;

#if 0
			case MSG_CMD_REPORT_ALARM_CHANGE:
				if(curNode->nodeBody.msgOrigin == MSG_LOCAL_EVENT)
				{
					reportAlarmArrivedEvent(curNode);
				}
				else
				{
					rebackReportAlarmArrived(curNode);
				}
				break;
#endif

				// Local message start
#ifdef DEVICE_NO_KEY
			case MSG_CMD_LOCAL_ENTER_SMARTLINK:
				localEnterSmartLink(curNode);
				break;
#endif

			//local MSG
			case MSG_CMD_LOCAL_GET_UTC_TIME:
				getUtcTimeByMessage();
				break;

			case MSG_CMD_DEVICE_RESET_FACTORY:
				if(curNode->nodeBody.msgOrigin == MSG_LOCAL_EVENT)
				{
					localRequstFactoryDataReset(curNode);
				}
				else
				{
					lum_replyFactoryDataReset(curNode);
				}
				break;

			case MSG_CMD_APP_RESET_FACTORY:
				lum_appResetFactory(curNode);
				break;

			case MSG_CMD_LOCAL_RESET_FACTORY:
				lum_deviceFactoryReset();
				break;

#ifdef RN8209C_SUPPORT
			case MSG_CMD_QUERY_ENERGY_DATA:
				lum_queryEnergyData(curNode);
				break;

			case MSG_CMD_REPORT_ENERGY_DATA:
				if(curNode->nodeBody.msgOrigin == MSG_LOCAL_EVENT)
				{
					lum_localReportEnergyUdata(curNode);
				}
				else
				{
					lum_replyEnergyUdata(curNode);
				}
				break;

#ifdef LUM_READ_ENERGY_TEST
			case MSG_CMD_READ_ENERGY_DATA:
				lum_showEnergyData();
				break;
#endif //LUM_READ_ENERGY_TEST


			case MSG_CMD_GET_CALIBRATE_DATA:
#ifdef RN8209_PRECISION_MACHINE
				lum_replyCalibrateData(curNode);
				break;

#elif defined(RN8209_CALIBRATE_SELF)
				if(curNode->nodeBody.msgOrigin == MSG_LOCAL_EVENT)
				{
					lum_getCalibrateData(curNode);
				}
				else
				{
					lum_setCalibrateData(curNode);
				}
				break;
#endif //RN8209_PRECISION_MACHINE

#endif //RN8209C_SUPPORT

#ifdef SX1208_433M_SUPPORT
			case MSG_CMD_433M_STUDY_KEY:
				lum_revc433StudyCmd(curNode);
				break;

			case MSG_CMD_433M_REPLY_STUDY_STATUS:
				lum_send433StudyStatus();
				break;

			case MSG_CMD_433M_CONTROL_KEY:
				lum_sendControlCmd(curNode);
				break;
#endif

			case MSG_CMD_SET_UDP_LOG_FLAG:
				lum_cmdSetUdpLogFlag(curNode);
				break;

			default:
				HF_Debug(DEBUG_ERROR, "meiyusong===> deviceMessageThread not found MSG  curNode->cmdData=0x%X\n", curNode->nodeBody.cmdData);
				break;
			}

			//lumi_debug("bReback=%d, sn=%d\n", curNode->nodeBody.bReback, curNode->nodeBody.snIndex);
			/*
			if(curNode->nodeBody.bReback == SEND_RESPOND)
			{
				deleteRequstSendNode(curNode->nodeBody.snIndex);
			}
			*/
			deleteListNode(curNode);
		}

		udpSockFd = getUdpSocketFd();
		tcpSockFd = getTcpSocketFd();
		sendSocketData(tcpSockFd, udpSockFd);

		if(listHeader->firstNodePtr == NULL)
		{
#ifdef EXTRA_SWITCH_SUPPORT
			lum_checkExtraKey();
#endif
			msleep(20);
		}
	}
}

#endif
