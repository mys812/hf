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



MSG_NODE* USER_FUNC mallocNodeMemory(U16 dataSize)
{
	MSG_NODE* pNode;

	pNode = (MSG_NODE*)mallocSocketData(sizeof(MSG_NODE));
	if(pNode != NULL)
	{
		pNode->dataBody.pData = (U8*)mallocSocketData(dataSize);
		if(pNode->dataBody.pData == NULL)
		{
			FreeSocketData((U8*)pNode);
			pNode = NULL;
		}
	}
	return pNode;
}



void USER_FUNC insertListNode(BOOL insetToHeader, MSG_NODE* pNode)
{
	LIST_HEADER* listHeader = &g_list_header;
	MSG_NODE* pTempNode;

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



static void USER_FUNC deleteNodeItem(MSG_NODE* pNode)
{
	if(pNode->dataBody.pData != NULL)
	{
		FreeSocketData(pNode->dataBody.pData);
	}
	FreeSocketData((U8*)pNode);
}


MSG_NODE* USER_FUNC searchNodePointer(BOOL bReback, U16 snIndex)
{
	MSG_NODE* pTempNode = g_list_header.firstNodePtr;


	if(pTempNode == NULL)
	{
		HF_Debug(DEBUG_ERROR, "meiyusong===> searchNodePointer no node\n");
		return NULL;
	}

	while(pTempNode != NULL)
	{
		if(pTempNode->dataBody.bReback == bReback && pTempNode->dataBody.snIndex == snIndex)
		{
			break;
		}
		pTempNode = pTempNode->pNodeNext;
	}
	return pTempNode;
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



	if(curNode == pNode)
	{
		listHeader->firstNodePtr = NULL;
	}
	else
	{
		while(curNode != NULL)
		{
			if(curNode == pNode)
			{
				pTempNode->pNodeNext = curNode->pNodeNext;
				deleteNodeItem(pNode);
				ret = TRUE;
				break;
			}
			pTempNode = curNode;
			curNode = curNode->pNodeNext;
		}
	}
	if(ret)
	{
		listHeader->noteCount--;
	}
	else
	{
		HF_Debug(DEBUG_ERROR, "meiyusong===> deleteListNode not found deleteListNode \n");
	}
	hfthread_mutext_unlock(g_message_mutex);
	return ret;
}




static void USER_FUNC showSocketOutsideData(SOCKET_HEADER_DATA* pHearderData)
{
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



static void USER_FUNC showHexData(S8* showData, U8 lenth)
{
	U8 i;
	S8 temData[512];
	U8 index = 0;


	memset(temData, 0, sizeof(temData));
	for(i=0; i<lenth && index<511; i++)
	{
		if(i == 0)
		{
			//do nothing
		}
		else if(i%8 == 0)
		{
			temData[index++] = ' ';
			temData[index++] = ' ';
		}
		else if(i%2 == 0)
		{
			temData[index++] = ',';
			temData[index++] = ' ';
		}
		sprintf(temData+index, "%02X", showData[i]);
		index += 2;
	}
	u_printf("meiyusong==> data=%s\n", temData);
}



#if 0
BOOL USER_FUNC socketDataAesDecrypt(S8 *inData, S8* outData, U32* aesDataLen, AES_KEY_TYPE keyType);


void USER_FUNC sendMessage(MSG_ORIGIN msgOrigin, U8* pData, U32 dataLen, AES_KEY_TYPE keyType)
{
	S8* aesBuf = NULL;
	MSG_NODE* msgNode = NULL;
	AES_KEY_TYPE aesKeyType;
	BOOL aesSucc;



	msgNode = (MSG_NODE*)mallocSocketData(sizeof(msgNode) + 1);
	if(msgNode == NULL)
	{
		HF_Debug(DEBUG_ERROR, "sendMessage malloc msgNode faild\n");
		return;
	}
	memset(msgNode, 0, (sizeof(msgNode) + 1));
	if(msgOrigin == MSG_FROM_UDP || msgOrigin == MSG_FROM_TCP)
	{
		msgNode->dataBody.pData = mallocSocketData(dataLen+1);
		if(msgNode->dataBody.pData == NULL)
		{
			HF_Debug(DEBUG_ERROR, "sendMessage malloc pData faild\n");
			FreeSocketData((U8*)msgNode);
			return;
		}
		memset(msgNode->dataBody.pData, 0, (dataLen+1));
		if()
		}
}



static void USER_FUNC udpSocketGetDecryptData(void)
{
	S32 recvCount;
	S8* recvBuf = getSocketRecvBuf(TRUE);
	struct sockaddr_in addr;
	SOCKET_HEADER_DATA* pHearderData = NULL;
	U8* pDecryptData = NULL;
	SOCKET_HEADER_OPEN* pOpenData = NULL;
	U32 decryptDataLen;


	recvCount= udpSocketRecvData(recvBuf, NETWORK_MAXRECV_LEN, g_udp_socket_fd, &addr);
	if (recvCount >= 10)
	{
		pOpenData = (SOCKET_HEADER_OPEN*)recvBuf;
		checkSocketLen(recvBuf, recvCount);
		decryptDataLen = pOpenData->dataLen + SOCKET_HEADER_OPEN_DATA_LEN;
		if(recvCount != decryptDataLen)
		{
			HF_Debug(DEBUG_ERROR, "Socket receive data len error recvDataLen=%d, data len should = %d\n", recvCount, decryptDataLen);
		}
		pDecryptData = socketDataAesDecrypt(recvBuf, decryptDataLen, AES_KEY_DEFAULT);
		if(pDecryptData == NULL)
		{
			return;
		}

		showHexData((S8*)pDecryptData, decryptDataLen);
		pHearderData = (SOCKET_HEADER_DATA*)pDecryptData;
		pHearderData->insideData = (S8*)(pDecryptData + sizeof(SCOKET_HERADER_OUTSIDE));
		showSocketOutsideData(pHearderData);
		FreeSocketData((U8*)pDecryptData);
	}
}
#endif



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
