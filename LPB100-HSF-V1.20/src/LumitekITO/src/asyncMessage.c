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




static void USER_FUNC showSocketOutsideData(U8* pData)
{
	SOCKET_HEADER_DATA* pHearderData = (SOCKET_HEADER_DATA*)pData;

	
	u_printf("pv=%d, flag=0x%x, mac=%x-%x-%x-%x-%x-%x, len=%d  reserved=%d snIndex=0x%x, deviceType=0x%x, factoryCode=0x%x, licenseData=0x%x\n",
	         pHearderData->outsideData.openData.pv,
	         pHearderData->outsideData.openData.flag,
	         pHearderData->outsideData.openData.mac[0],
	         pHearderData->outsideData.openData.mac[1],
	         pHearderData->outsideData.openData.mac[2],
	         pHearderData->outsideData.openData.mac[3],
	         pHearderData->outsideData.openData.mac[4],
	         pHearderData->outsideData.openData.mac[5],
	         pHearderData->outsideData.openData.dataLen,
	         pHearderData->outsideData.reserved,
	         pHearderData->outsideData.snIndex,
	         pHearderData->outsideData.deviceType,
	         pHearderData->outsideData.factoryCode,
	         pHearderData->outsideData.licenseData);

}




BOOL USER_FUNC addToMessageList(MSG_ORIGIN msgOrigin, U8* pData, U32 dataLen, U32 socketIp)
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
		u_printf("=================> CMD=0x%X \n", pSocketData[sizeof(SCOKET_HERADER_OUTSIDE)]);
		showHexData("Recv", pSocketData, aesDataLen);
		

		pMsgNode = (MSG_NODE*)mallocSocketData(sizeof(MSG_NODE));
		if(pMsgNode == NULL)
		{
			HF_Debug(DEBUG_ERROR, "meiyusong===> addToMessageList malloc faild \n");
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
					rebackDeviceName(curNode);
					break;

				case MSG_CMD_LOCK_DEVICE:
					rebackLockDevice(curNode);
					
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
