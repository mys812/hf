/*
******************************
*Company:Lumitek
*Data:2014-11-01
*Author:Meiyusong
******************************
*/
#ifndef __SOCCKET_SEND_LIST_H__
#define __SOCCKET_SEND_LIST_H__

#include <hsf.h>

typedef struct
{
	U16 snIndex;
	U16 dataLen;
	U8* pData;
	MSG_ORIGIN msgOrigin;
	U32 socketIp;
    U8 bReback;
	U8 sendCount;
	U8 faildTimes;
	time_t nextSendTime;
} SEND_NODE_DATA;


typedef struct send_node
{
	SEND_NODE_DATA nodeBody;
	struct send_node* pNodeNext;
} SEND_NODE;


typedef struct
{
	U16 noteCount;
	SEND_NODE* firstNodePtr;
} SEND_LIST_HEADER;


typedef enum
{
	SEND_REQUST = 0,
	SEND_RESPOND = 1
}SEND_SOCKET_TYPE;


typedef enum
{
	SOCKET_READ_ENABLE = 0x01,
	SOCKET_WRITE_ENABLE = 0x02,
}SOCKET_SELECT_TYPE;



void USER_FUNC sendListInit(void);
BOOL USER_FUNC addSendDataToNode(SEND_NODE_DATA* pSendData);
BOOL USER_FUNC sendSocketData(MSG_ORIGIN socketType);
BOOL USER_FUNC deleteRequstSendNode(U16 snIndex);


#endif

