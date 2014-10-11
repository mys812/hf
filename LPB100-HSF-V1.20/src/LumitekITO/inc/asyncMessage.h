#ifndef __ASYNC_MESSAGE_H__
#define __ASYNC_MESSAGE_H__

#include <hsf.h>
#include <string.h>
#include <stdio.h>

#include "../inc/itoCommon.h"



typedef struct MSG_NODE
{
	U8 cmdData;
	BOOL bReback;
	U16 snIndex;
	U8* socketData;
	struct MSG_NODE* pNodeNext;
}MSG_NODE;


typedef struct
{
	U8 nodeNum;
	MSG_NODE* headerNext;
}NODE_HEADER;

void USER_FUNC deviceMessageThread(void);

MSG_NODE* USER_FUNC mallocNodeMemory(U16 dataSize);
void USER_FUNC insertListNode(BOOL insetToHeader, MSG_NODE* pNode);
static void USER_FUNC deleteNodeItem(MSG_NODE* pNode);
void USER_FUNC deleteListNode(U16 snIndex, BOOL bReback);



#endif
