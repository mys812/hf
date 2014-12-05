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
#ifdef LUMITEK_DEBUG_SWITCH
	U8 cmdData;
#endif
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
BOOL USER_FUNC sendSocketData(S32 tcpSockFd, S32 udpSockFd);
BOOL USER_FUNC deleteRequstSendNode(U16 snIndex);

U8 USER_FUNC socketSelectRead(S32 sockFd, U32 waitSecond);
U8 USER_FUNC socketSelectWrite(S32 sockFd);


#endif


