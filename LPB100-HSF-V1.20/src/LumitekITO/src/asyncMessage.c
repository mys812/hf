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
		pNode->pData = (U8*)mallocSocketData(dataSize);
		if(pNode->pData == NULL)
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
	if(pNode->pData != NULL)
	{
		FreeSocketData(pNode->pData);
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
		
		if(listHeader->noteCount !=0)
		{
			curNode = listHeader->firstNodePtr;


			switch(curNode->cmdData)
			{
				case MSG_CMD_FOUND_DEVICE:
					break;

				default:
					HF_Debug(DEBUG_ERROR, "meiyusong===> deviceMessageThread not found MSG  curNode->cmdData=0x%X\n", curNode->cmdData);
					break;
			}
			deleteListNode(curNode);
		}
		msleep(100);
	}
}

#endif
