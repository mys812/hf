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



U8 USER_FUNC socketSelectRead(S32 sockFd, U32 waitSecond)
{
	fd_set fdRead;
	struct timeval timeout;
	S32 ret;
	U8 sel= 0;


	timeout.tv_sec = waitSecond;
	timeout.tv_usec = 0;

	FD_ZERO(&fdRead);
	if (sockFd != -1)
	{
		FD_SET(sockFd,&fdRead);
	}
	ret= select((sockFd + 1), &fdRead, NULL, NULL, &timeout);
	if (ret <= 0)
	{
		//return 0;
	}
	else
	{
		if (FD_ISSET(sockFd, &fdRead))
		{
			sel = SOCKET_READ_ENABLE;
		}
	}
	
	return sel;
}



U8 USER_FUNC socketSelectWrite(S32 sockFd)
{
	fd_set fdWrite;
	struct timeval timeout;
	S32 ret;
	U8 sel= 0;


	timeout.tv_sec = 3;
	timeout.tv_usec = 0;

	FD_ZERO(&fdWrite);
	if (sockFd != -1)
	{
		FD_SET(sockFd,&fdWrite);
	}
	ret= select((sockFd + 1), NULL, &fdWrite, NULL, &timeout);
	if (ret <= 0)
	{
		//return 0;
	}
	else
	{
		if (FD_ISSET(sockFd, &fdWrite))
		{
			sel = SOCKET_WRITE_ENABLE;
		}
	}
	
	return sel;
}



BOOL USER_FUNC sendSocketData(S32 tcpSockFd, S32 udpSockFd)
{
	SEND_NODE* pCurNode = g_sendListHeader.firstNodePtr;
	BOOL sendSuccess = FALSE;
	time_t curTime;
	BOOL fdReady = TRUE;

	
	curTime = time(NULL);
	while(pCurNode != NULL)
	{
#if 0
		lumi_debug("curTime = %d nextSendTime = %d sendCount=%d faildTimes=%d sn=%d noteCount=%d mallocCount=%d\n",
			curTime,
			pCurNode->nodeBody.nextSendTime,
			pCurNode->nodeBody.sendCount,
			pCurNode->nodeBody.faildTimes,
			pCurNode->nodeBody.snIndex,
			g_sendListHeader.noteCount,
			getMallocCount());
#endif		
		if(pCurNode->nodeBody.msgOrigin != MSG_FROM_UDP && pCurNode->nodeBody.msgOrigin != MSG_FROM_TCP)
		{
			lumi_error("socketType=%d\n", pCurNode->nodeBody.msgOrigin);
			pCurNode = pCurNode->pNodeNext;
			continue;
		}
		
		if(pCurNode->nodeBody.nextSendTime == 0 || pCurNode->nodeBody.nextSendTime < curTime)
		{
			if(pCurNode->nodeBody.msgOrigin == MSG_FROM_UDP)
			{
				if(socketSelectWrite(udpSockFd))
				{
					sendSuccess = sendUdpData(pCurNode->nodeBody.pData, pCurNode->nodeBody.dataLen, pCurNode->nodeBody.socketIp);
				}
				else
				{
					fdReady = FALSE;
				}
			}
			else
			{
				if(socketSelectWrite(tcpSockFd))
				{
					sendSuccess = sendTcpData(pCurNode->nodeBody.pData, pCurNode->nodeBody.dataLen);
				}
				else
				{
					fdReady = FALSE;
				}
			}

			if(!fdReady)
			{
				lumi_error("socketFd not ready msgOrigin=%d", pCurNode->nodeBody.msgOrigin);
				pCurNode->nodeBody.nextSendTime = curTime + (MAX_RESEND_INTERVAL>>1);
				pCurNode = pCurNode->pNodeNext;
				continue;
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
			break;	//delay sometime after send a socket 
		}
		pCurNode = pCurNode->pNodeNext;
	}
	return sendSuccess;
}
#endif

