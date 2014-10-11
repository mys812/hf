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




static MSG_NODE* g_pHeader = NULL;



MSG_NODE* USER_FUNC mallocNodeMemory(U16 dataSize)
{
	MSG_NODE* pNode;

	pNode = (MSG_NODE*)mallocSocketData(sizeof(MSG_NODE));
	if(pNode != NULL)
	{
		pNode->socketData = (U8*)mallocSocketData(dataSize);
		if(pNode->socketData == NULL)
		{
			FreeSocketData((U8*)pNode);
			pNode = NULL;
		}
	}
	return pNode;
}


void USER_FUNC insertListNode(BOOL insetToHeader, MSG_NODE* pNode)
{
	if(g_pHeader == NULL)
	{
		g_pHeader = pNode;
		pNode->pNodeNext = NULL;
	}
	else
	{
		if(insetToHeader)
		{
			pNode->pNodeNext = g_pHeader->pNodeNext;
			g_pHeader = pNode;
		}
		else
		{
			MSG_NODE* pTempNode = g_pHeader;
			while(pTempNode->pNodeNext != NULL)
			{
				pTempNode = pTempNode->pNodeNext;
			}
			pTempNode->pNodeNext = pNode;
			pNode->pNodeNext = NULL;
		}
	}
}



static void USER_FUNC deleteNodeItem(MSG_NODE* pNode)
{
	if(pNode->socketData != NULL)
	{
		FreeSocketData(pNode->socketData);
	}
	FreeSocketData((U8*)pNode);
}



void USER_FUNC deleteListNode(U16 snIndex, BOOL bReback)
{
	MSG_NODE* pTempNode = g_pHeader;


	if(g_pHeader == NULL)
	{
		HF_Debug(DEBUG_ERROR, "meiyusong===> deleteListNode error no node to delete\n");
		return;
	}

	if(pTempNode->pNodeNext == NULL)
	{
		if(pTempNode->snIndex == snIndex && pTempNode->bReback == bReback)
		{
			deleteNodeItem(pTempNode);
		}
	}
	else
	{
		while(pTempNode->pNodeNext != NULL)
		{
			if(pTempNode->pNodeNext->snIndex == snIndex && pTempNode->pNodeNext->bReback == bReback)
			{
				pTempNode->pNodeNext = pTempNode->pNodeNext->pNodeNext;
				deleteNodeItem(pTempNode);
				break;
			}
		}
	}
}



void USER_FUNC deviceMessageThread(void)
{
	while(1)
	{

		//u_printf(" deviceMessageThread \n");
		msleep(100);
	}
}

#endif
