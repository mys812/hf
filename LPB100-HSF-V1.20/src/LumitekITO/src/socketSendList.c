/*
******************************
*Company:Lumitek
*Data:2014-11-01
*Author:Meiyusong
******************************
*/

#include "../inc/lumitekConfig.h"

#ifdef CONFIG_LUMITEK_DEVICE
#include <hsf.h>
#include <string.h>
#include <stdio.h>

#include "../inc/itoCommon.h"
#include "../inc/socketSendList.h"
#include "../inc/localSocketUdp.h"
#include "../inc/serverSocketTcp.h"




static SEND_LIST_HEADER g_sendListHeader;
hfthread_mutex_t g_sendList_mutex;




void USER_FUNC sendListInit(void)
{
	g_sendListHeader.firstNodePtr = NULL;
	g_sendListHeader.noteCount = 0;

	if((hfthread_mutext_new(&g_sendList_mutex)!= HF_SUCCESS))
	{
		HF_Debug(DEBUG_ERROR, "failed to create g_sendList_mutex");

	}
}



static void USER_FUNC insertSendListNode(SEND_NODE* pNode)
{
	SEND_LIST_HEADER* pListHeader = &g_sendListHeader;
	SEND_NODE* curNode;


	hfthread_mutext_lock(g_sendList_mutex);

	curNode = pListHeader->firstNodePtr;
	if(curNode == NULL)
	{
		pListHeader->firstNodePtr = pNode;
		pNode->pNodeNext = NULL;
	}
	else
	{
		while(curNode->pNodeNext != NULL)
		{
			curNode = curNode->pNodeNext;
		}
		curNode->pNodeNext = pNode;
		pNode->pNodeNext = NULL;
	}
	pListHeader->noteCount++;
	hfthread_mutext_unlock(g_sendList_mutex);
}



static void USER_FUNC freeSendNodeMemory(SEND_NODE* pNode)
{
	if(pNode->nodeBody.pData != NULL)
	{
		FreeSocketData(pNode->nodeBody.pData);
		pNode->nodeBody.pData = NULL;
	}
	FreeSocketData((U8*)pNode);
}


static BOOL USER_FUNC deleteSendListNode(SEND_NODE* pNode)
{
	SEND_LIST_HEADER* pListHeader = &g_sendListHeader;
	SEND_NODE* pCurNode;
	BOOL ret = FALSE;


	hfthread_mutext_lock(g_sendList_mutex);

	pCurNode = pListHeader->firstNodePtr;
	if(pCurNode == NULL || pNode == NULL)
	{
		HF_Debug(DEBUG_ERROR, "deleteListNode error no node to delete\n");
		hfthread_mutext_unlock(g_sendList_mutex);
		return FALSE;
	}
	if(pCurNode == pNode)
	{
		pListHeader->firstNodePtr = pNode->pNodeNext;
		ret = TRUE;
	}
	else
	{
		while(pCurNode->pNodeNext != NULL)
		{
			if(pCurNode->pNodeNext == pNode)
			{
				pCurNode->pNodeNext = pCurNode->pNodeNext->pNodeNext;
				ret = TRUE;
				break;
			}
			pCurNode->pNodeNext = pCurNode->pNodeNext->pNodeNext;
		}
	}
	if(ret)
	{
		pListHeader->noteCount--;
		freeSendNodeMemory(pNode);
	}
	else
	{
		HF_Debug(DEBUG_ERROR, "deleteSendListNode not found \n");
	}
	hfthread_mutext_unlock(g_sendList_mutex);
	return ret;
}



BOOL USER_FUNC deleteRequstSendNode(U16 snIndex)
{
	SEND_NODE* pCurNode = g_sendListHeader.firstNodePtr;
	BOOL ret = FALSE;

	while(pCurNode != NULL)
	{
		if(pCurNode->nodeBody.snIndex == snIndex)
		{
			deleteSendListNode(pCurNode);
			ret = TRUE;
			break;
		}
		pCurNode = pCurNode->pNodeNext;
	}
	return ret;
}



BOOL USER_FUNC addSendDataToNode(SEND_NODE_DATA* pSendData)
{
	SEND_NODE* pSendNode;


	if(pSendData->msgOrigin != MSG_FROM_UDP && pSendData->msgOrigin != MSG_FROM_TCP)
	{
		lumi_error("sendData->msgOrigin=%d\n", pSendData->msgOrigin);
		return FALSE;
	}
	pSendNode = (SEND_NODE*)mallocSocketData(sizeof(SEND_NODE));
	if(pSendNode == NULL)
	{
		lumi_error("malloc error\n");
		return FALSE;
	}
	memset(pSendNode, 0, sizeof(SEND_NODE));
	memcpy(&pSendNode->nodeBody, pSendData, sizeof(SEND_NODE_DATA));
	insertSendListNode(pSendNode);
	return TRUE;
}



BOOL USER_FUNC sendSocketData(MSG_ORIGIN socketType)
{
	SEND_NODE* pCurNode = g_sendListHeader.firstNodePtr;
	BOOL found = FALSE;
	BOOL sendSuccess = FALSE;
	time_t curTime;

	if(socketType != MSG_FROM_UDP && socketType != MSG_FROM_TCP)
	{
		lumi_error("socketType=%d\n", socketType);
		return FALSE;
	}
	
	curTime = time(NULL);
	while(pCurNode != NULL)
	{
		if(pCurNode->nodeBody.msgOrigin == socketType)
		{
			if(pCurNode->nodeBody.nextSendTime == 0 || pCurNode->nodeBody.nextSendTime >= curTime)
			{
				found = TRUE;
			}

			if(found)
			{
				if(socketType == MSG_FROM_UDP)
				{
					sendSuccess = sendUdpData(pCurNode->nodeBody.pData, pCurNode->nodeBody.dataLen, pCurNode->nodeBody.socketIp);
				}
				else
				{
					sendSuccess = sendTcpData(pCurNode->nodeBody.pData, pCurNode->nodeBody.dataLen);
				}

				if(sendSuccess)
				{
					if(pCurNode->nodeBody.bReback == SEND_REQUST)
					{
						pCurNode->nodeBody.faildTimes = 0;
						pCurNode->nodeBody.sendCount++;
						if(pCurNode->nodeBody.sendCount >= MAX_RESEND_COUNT)
						{
							deleteSendListNode(pCurNode);
						}
						else
						{
							pCurNode->nodeBody.nextSendTime = curTime + MAX_RESEND_INTERVAL;
						}
					}
					else
					{
						deleteSendListNode(pCurNode);
					}
				}
				else //send faild
				{
					pCurNode->nodeBody.faildTimes++;
					if(pCurNode->nodeBody.faildTimes >= MAX_FAILD_COUNT)
					{
						deleteSendListNode(pCurNode);
					}
					else
					{
						pCurNode->nodeBody.nextSendTime = curTime + MAX_RESEND_INTERVAL;
					}
				}
				break;
			}
		}
		pCurNode = pCurNode->pNodeNext;
	}
	return sendSuccess;
}
#endif

