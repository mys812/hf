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




//static MSG_NODE* g_pHeader = NULL;
static LIST_HEADER g_list_header;
static LIST_HEADER g_resend_list_header;

hfthread_mutex_t g_message_mutex;




static void USER_FUNC messageListInit(void)
{
	g_list_header.firstNodePtr = NULL;
	g_list_header.noteCount = 0;

	g_resend_list_header.firstNodePtr = NULL;
	g_resend_list_header.noteCount = 0;

	if((hfthread_mutext_new(&g_message_mutex)!= HF_SUCCESS))
	{
		HF_Debug(DEBUG_ERROR, "failed to create g_message_mutex");

	}
}


#if 0
static MSG_NODE* USER_FUNC mallocNodeMemory(U16 dataSize)
{
	MSG_NODE* pNode;

	pNode = (MSG_NODE*)mallocSocketData(sizeof(MSG_NODE) + 1);
	if(pNode != NULL)
	{
		pNode->nodeBody.pData = (U8*)mallocSocketData(dataSize + 1);
		if(pNode->nodeBody.pData == NULL)
		{
			FreeSocketData((U8*)pNode);
			pNode = NULL;
		}
	}
	return pNode;
}
#endif


static void USER_FUNC insertListNode(BOOL insetToHeader, MSG_NODE* pNode, BOOL bResend)
{
	LIST_HEADER* pListHeader;
	MSG_NODE* pTempNode;


	hfthread_mutext_lock(g_message_mutex);
	if(bResend)
	{
		pListHeader = &g_resend_list_header;
	}
	else
	{
		pListHeader = &g_list_header;
	}

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




static BOOL USER_FUNC deleteListNode(MSG_NODE* pNode, BOOL bResend)
{
	LIST_HEADER* pListHeader;
	MSG_NODE* curNode;
	MSG_NODE* pTempNode;
	BOOL ret = FALSE;


	hfthread_mutext_lock(g_message_mutex);
	if(bResend)
	{
		pListHeader = &g_resend_list_header;
	}
	else
	{
		pListHeader = &g_list_header;
	}

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



BOOL USER_FUNC deleteResendData(U16 snIndex, U8 cmdCode)
{
	LIST_HEADER* pListHeader = &g_resend_list_header;
	MSG_NODE* pNode = pListHeader->firstNodePtr;
	BOOL ret = FALSE;

	while(pNode != NULL)
	{
		if(pNode->nodeBody.cmdData == cmdCode && pNode->nodeBody.snIndex == snIndex)
		{
			deleteListNode(pNode, TRUE);
			ret = TRUE;
			break;
		}
		pNode = pNode->pNodeNext;
	}
	if(!ret)
	{
		HF_Debug(DEBUG_ERROR, "meiyusong===> deleteResendData not found \n");
	}
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
		pSocketData = encryptRecvSocketData(msgOrigin, pData, &aesDataLen);
		pOutSide = (SCOKET_HERADER_OUTSIDE*)pSocketData;
		if(pSocketData == NULL)
		{
			return ret;
		}
		else if(pSocketData[SOCKET_CMD_OFFSET] != MSG_CMD_FOUND_DEVICE && !needRebackRecvSocket((pSocketData + SOCKET_MAC_ADDR_OFFSET), TRUE))
		{
			FreeSocketData(pSocketData);
			return ret;
		}
#if 0
		else if(pOutSide->openData.flag.bReback != 0)
		{
			// add something
			freeNodeMemory(pMsgNode);
			return ret;
		}
#endif
		lumi_debug("CMD=0x%X \n", pSocketData[SOCKET_HEADER_LEN]);
		if(msgOrigin == MSG_FROM_UDP)
		{
			showHexData("UDP Recv", pSocketData, aesDataLen);
		}
		else if(msgOrigin == MSG_FROM_TCP)
		{
			showHexData("TCP Recv", pSocketData, aesDataLen);
		}


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

		insertListNode(FALSE, pMsgNode, FALSE);
		ret = TRUE;

	}
	return ret;
}



BOOL USER_FUNC insertLocalMsgToList(MSG_ORIGIN msgOrigin, U8* pData, U32 dataLen, U8 cmdData)
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
	pMsgNode->nodeBody.dataLen = dataLen;

	if(pData != NULL)
	{
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

	insertListNode(FALSE, pMsgNode, FALSE);
	return ret;
}


BOOL USER_FUNC insertResendMsgToList(RESEND_NODE_DATA* resendNodeData)
{
	MSG_NODE* pMsgNode;
	U8* localData;
	BOOL ret = FALSE;


	pMsgNode = (MSG_NODE*)mallocSocketData(sizeof(MSG_NODE));
	if(pMsgNode == NULL)
	{
		HF_Debug(DEBUG_ERROR, "insertSocketMsgToList malloc faild \n");
		return ret;
	}
	pMsgNode->nodeBody.cmdData = resendNodeData->cmdData;
	pMsgNode->nodeBody.msgOrigin = resendNodeData->msgOrigin;
	pMsgNode->nodeBody.dataLen = resendNodeData->dataLen;
	pMsgNode->nodeBody.snIndex = resendNodeData->snIndex;
	pMsgNode->nodeBody.socketIp = resendNodeData->socketIp;
	pMsgNode->nodeBody.sendTime = resendNodeData->sendTime;
	pMsgNode->nodeBody.resendCount = resendNodeData->resendCount;

	if(resendNodeData->pData != NULL)
	{
		localData = mallocSocketData(resendNodeData->dataLen + 1);
		if(localData == NULL)
		{
			FreeSocketData((U8*)pMsgNode);
			return ret;
		}
		memcpy(localData, resendNodeData->pData, resendNodeData->dataLen);
		pMsgNode->nodeBody.pData = localData;
		ret = TRUE;
	}

	insertListNode(FALSE, pMsgNode, TRUE);
	return ret;
}



static void USER_FUNC checkNeedResendSocket(void)
{
	time_t curTime = time(NULL);
	MSG_NODE* pCurNode = g_resend_list_header.firstNodePtr;
	MSG_NODE* PTmpNode;
	U32 timeInterval;


	while(pCurNode != NULL)
	{
		timeInterval = curTime - pCurNode->nodeBody.sendTime;

		//lumi_debug("time=%ld timeInterval=%d, resendCount=%d sendTime=%ld\n", curTime, timeInterval, pCurNode->nodeBody.resendCount, pCurNode->nodeBody.sendTime);
		PTmpNode = pCurNode->pNodeNext;
		
		if(pCurNode->nodeBody.resendCount >= MAX_RESEND_COUNT)
		{
			deleteListNode(pCurNode, TRUE);
		}
		else if(timeInterval >= MAX_RESEND_INTERVAL)
		{
			if(pCurNode->nodeBody.pData != NULL)
			{
				if(pCurNode->nodeBody.msgOrigin == MSG_FROM_UDP)
				{
					sendUdpData(pCurNode->nodeBody.pData, pCurNode->nodeBody.dataLen, pCurNode->nodeBody.socketIp);
				}
				else
				{
					sendTcpData(pCurNode->nodeBody.pData, pCurNode->nodeBody.dataLen);
				}
				pCurNode->nodeBody.resendCount++;
				pCurNode->nodeBody.sendTime = curTime;
				msleep(100);
				lumi_debug("start resent cmd=0x%x, sn=%d resendCount=%d\n",
					pCurNode->nodeBody.cmdData,
					pCurNode->nodeBody.snIndex,
					pCurNode->nodeBody.resendCount);
			}
			else
			{
				deleteListNode(pCurNode, TRUE);
			}
		}

		pCurNode = PTmpNode ;
	}
}



void USER_FUNC deviceMessageThread(void)
{
	LIST_HEADER* listHeader = &g_list_header;
	MSG_NODE* curNode;


	messageListInit();
	hfthread_enable_softwatchdog(NULL,30); //Start watchDog
	while(1)
	{
		//lumi_debug(" deviceMessageThread \n");
		hfthread_reset_softwatchdog(NULL); //tick watchDog

		curNode = listHeader->firstNodePtr;
		if(curNode != NULL)
		{
			switch(curNode->nodeBody.cmdData)
			{
			case MSG_CMD_FOUND_DEVICE:
				rebackFoundDevice(curNode);
				break;

			case MSG_CMD_HEART_BEAT:
				rebackHeartBeat(curNode);
				break;

			case MSG_CMD_QUARY_MODULE_INFO:
				rebackGetDeviceName(curNode);
				break;

			case MSG_CMD_SET_MODULE_NAME:
				rebackSetDeviceName(curNode);
				break;

			case MSG_CMD_MODULE_UPGRADE:
				rebackGetDeviceUpgrade(curNode);
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

				// Local message start
			case MSG_CMD_LOCAL_ENTER_SMARTLINK:
				localEnterSmartLink(curNode);
				break;

			default:
				HF_Debug(DEBUG_ERROR, "meiyusong===> deviceMessageThread not found MSG  curNode->cmdData=0x%X\n", curNode->nodeBody.cmdData);
				break;
			}

			deleteListNode(curNode, FALSE);
		}

		if(listHeader->firstNodePtr == NULL)
		{
			checkNeedResendSocket(); //check have any socket need resend
			msleep(100);
		}
	}
}

#endif
