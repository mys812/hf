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


#if 0
static MSG_NODE* USER_FUNC mallocNodeMemory(U16 dataSize)
{
	MSG_NODE* pNode;

	pNode = (MSG_NODE*)mallocSocketData(sizeof(MSG_NODE) + 1);
	if(pNode != NULL)
	{
		pNode->dataBody.pData = (U8*)mallocSocketData(dataSize + 1);
		if(pNode->dataBody.pData == NULL)
		{
			FreeSocketData((U8*)pNode);
			pNode = NULL;
		}
	}
	return pNode;
}
#endif


void USER_FUNC insertListNode(BOOL insetToHeader, MSG_NODE* pNode)
{
	LIST_HEADER* listHeader = &g_list_header;
	MSG_NODE* pTempNode;

	//u_printf("go into insertListNode pNode= 0x%X\n", pNode);
	hfthread_mutext_lock(g_message_mutex);
	pTempNode = listHeader->firstNodePtr;
	if(listHeader->noteCount == 0)
	{
		listHeader->firstNodePtr = pNode;
		pNode->pNodeNext = NULL;
	}
	else
	{
		if(insetToHeader)
		{
			pNode->pNodeNext = pTempNode->pNodeNext;
			listHeader->firstNodePtr = pNode;
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
	listHeader->noteCount++;
	hfthread_mutext_unlock(g_message_mutex);
}



static void USER_FUNC freeNodeMemory(MSG_NODE* pNode)
{
	if(pNode->dataBody.pData != NULL)
	{
		FreeSocketData(pNode->dataBody.pData);
		pNode->dataBody.pData = NULL;
	}
	FreeSocketData((U8*)pNode);
}




BOOL USER_FUNC deleteListNode(MSG_NODE* pNode)
{
	LIST_HEADER* listHeader = &g_list_header;
	MSG_NODE* curNode;
	MSG_NODE* pTempNode;
	BOOL ret = FALSE;


	hfthread_mutext_lock(g_message_mutex);
	curNode = listHeader->firstNodePtr;
	if(curNode == NULL || pNode == NULL)
	{
		HF_Debug(DEBUG_ERROR, "meiyusong===> deleteListNode error no node to delete\n");
		hfthread_mutext_unlock(g_message_mutex);
		return FALSE;
	}

	while(curNode != NULL)
	{
		if(curNode == pNode)
		{
			if(curNode == listHeader->firstNodePtr)
			{
				listHeader->firstNodePtr = listHeader->firstNodePtr->pNodeNext;
			}
			else
			{
				pTempNode->pNodeNext = curNode->pNodeNext;
			}
			ret = TRUE;
			break;
		}
		pTempNode = curNode;
		curNode = curNode->pNodeNext;
	}
	if(ret)
	{
		listHeader->noteCount--;
		freeNodeMemory(pNode);
	}
	else
	{
		HF_Debug(DEBUG_ERROR, "meiyusong===> deleteListNode not found deleteListNode \n");
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
		pSocketData = encryptRecvSocketData(msgOrigin, pData, &aesDataLen);
		pOutSide = (SCOKET_HERADER_OUTSIDE*)pSocketData;
		if(pSocketData == NULL)
		{
			return ret;
		}
		else if(pSocketData[SOCKET_CMD_OFFSET] != MSG_CMD_FOUND_DEVICE && !needRebackFoundDevice((pSocketData + SOCKET_MAC_ADDR_OFFSET), TRUE))
		{
			FreeSocketData(pSocketData);
			return ret;
		}
		else if(pOutSide->openData.flag.bReback != 0)
		{
			// add something
			freeNodeMemory(pMsgNode);
			return ret;
		}
		u_printf("=================> CMD=0x%X \n", pSocketData[SOCKET_HEADER_LEN]);
		showHexData("Recv", pSocketData, aesDataLen);
		

		pMsgNode = (MSG_NODE*)mallocSocketData(sizeof(MSG_NODE));
		if(pMsgNode == NULL)
		{
			HF_Debug(DEBUG_ERROR, "meiyusong===> insertSocketMsgToList malloc faild \n");
			return ret;
		}
		pMsgNode->dataBody.cmdData = pSocketData[SOCKET_HEADER_LEN];
		pMsgNode->dataBody.bReback = pOutSide->openData.flag.bReback;
		pMsgNode->dataBody.snIndex = pOutSide->snIndex;
		pMsgNode->dataBody.pData = pSocketData;
		pMsgNode->dataBody.dataLen = aesDataLen;
		pMsgNode->dataBody.msgOrigin = msgOrigin;
		pMsgNode->dataBody.socketIp = socketIp;
		
		insertListNode(FALSE, pMsgNode);
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
		HF_Debug(DEBUG_ERROR, "meiyusong===> insertSocketMsgToList malloc faild \n");
		return ret;
	}
	pMsgNode->dataBody.cmdData = cmdData;
	pMsgNode->dataBody.msgOrigin = MSG_LOCAL_EVENT;
	pMsgNode->dataBody.dataLen = dataLen;

	if(pData != NULL)
	{
		localData = mallocSocketData(dataLen+1);
		if(localData == NULL)
		{
			FreeSocketData((U8*)pMsgNode);
			return ret;
		}
		pMsgNode->dataBody.pData = localData;
		ret = TRUE;
	}

	insertListNode(FALSE, pMsgNode);
	return ret;
}




void USER_FUNC deviceMessageThread(void)
{
	LIST_HEADER* listHeader = &g_list_header;
	MSG_NODE* curNode;


	messageListInit();
	hfthread_enable_softwatchdog(NULL,30); //Start watchDog
	while(1)
	{
		//u_printf(" deviceMessageThread \n");
		hfthread_reset_softwatchdog(NULL); //tick watchDog

		curNode = listHeader->firstNodePtr;
		if(curNode != NULL)
		{
			switch(curNode->dataBody.cmdData)
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



				// Local message start
				case MSG_CMD_LOCAL_ENTER_SMARTLINK:
					localEnterSmartLink(curNode);
					break;

				default:
					HF_Debug(DEBUG_ERROR, "meiyusong===> deviceMessageThread not found MSG  curNode->cmdData=0x%X\n", curNode->dataBody.cmdData);
					break;
			}
			deleteListNode(curNode);
		}
		msleep(100);
	}
}

#endif
