#ifndef __ASYNC_MESSAGE_H__
#define __ASYNC_MESSAGE_H__

#include <hsf.h>
#include <string.h>
#include <stdio.h>

#include "../inc/itoCommon.h"



typedef struct msg_node
{
	U16 cmdData;
	BOOL bReback;
	U16 snIndex;
	U8* pData;
	struct msg_node* pNodeNext;
}MSG_NODE;


typedef struct
{
	U8 noteCount;
	MSG_NODE* firstNodePtr;
}LIST_HEADER;

typedef struct
{
	U8 nodeNum;
	MSG_NODE* headerNext;
}NODE_HEADER;


typedef enum
{
	MSG_CMD_SET_GPIO_STATUS			= 0x01,
	MSG_CMD_GET_GPIO_STATUS			= 0x02,
	MSG_CMD_SET_ALARM_DATA			= 0x03,
	MSG_CMD_GET_ALARM_DATA			= 0x04,
	MSG_CMD_DELETE_ALARM_DATA		= 0x05,

	MSG_CMD_FOUND_DEVICE			= 0x23,
	MSG_CMD_LOCK_DEVICE				= 0x24,

	MSG_CMD_SEARCH_SERVER_ADDR		= 0x41,
	MSG_CMD_REQUEST_CONNECT			= 0x42,
	
	MSG_CMD_HEART_BEAT 				= 0x61,
	MSG_CMD_QUARY_MODULE_INFO		= 0x62,
	MSG_CMD_SET_MODULE_NAME			= 0x63,
	MSG_CMD_MODULE_UPGRADE			= 0x65,
	MSG_CMD_ENTER_SMART_LINK		= 0x66,
}MESSAGE_CMD_TYPE;
void USER_FUNC deviceMessageThread(void);

MSG_NODE* USER_FUNC mallocNodeMemory(U16 dataSize);
void USER_FUNC insertListNode(BOOL insetToHeader, MSG_NODE* pNode);
BOOL USER_FUNC deleteListNode(MSG_NODE* pNode);




#endif
