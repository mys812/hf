#ifndef __ASYNC_MESSAGE_H__
#define __ASYNC_MESSAGE_H__

#include <hsf.h>
#include <string.h>
#include <stdio.h>

#include "../inc/itoCommon.h"


typedef struct
{
	U8 cmdData;
	BOOL bReback;
	U16 snIndex;
	U16 dataLen;
	U8* pData;
	MSG_ORIGIN msgOrigin;
	U32 socketIp;
} MSG_NODE_BODY;



typedef struct msg_node
{
	MSG_NODE_BODY nodeBody;
	struct msg_node* pNodeNext;
} MSG_NODE;


typedef struct
{
	U8 noteCount;
	MSG_NODE* firstNodePtr;
} LIST_HEADER;


typedef struct
{
	U8 nodeNum;
	MSG_NODE* headerNext;
} NODE_HEADER;


typedef enum
{
	//Gpio data
	MSG_CMD_SET_GPIO_STATUS				= 0x01,
	MSG_CMD_GET_GPIO_STATUS				= 0x02,
	//Alarm data
	MSG_CMD_SET_ALARM_DATA				= 0x03,
	MSG_CMD_GET_ALARM_DATA				= 0x04,
	MSG_CMD_DELETE_ALARM_DATA			= 0x05,
	//Report data
	MSG_CMD_REPORT_GPIO_CHANGE			= 0x06,
	MSG_CMD_REPORT_ALARM_CHANGE			= 0x07,
	//Against thief
	MSG_CMD_SET_ABSENCE_DATA			= 0x09,
	MSG_CMD_GET_ABSENCE_DATA			= 0x0A,
	MSG_CMD_DELETE_ABSENCE_DATA			= 0x0B,
	//stop watch
	MSG_CMD_SET_COUNDDOWN_DATA			= 0x0C,
	MSG_CMD_GET_COUNTDOWN_DATA			= 0x0D,
	MSG_CMD_DELETE_COUNTDOWN_DATA		= 0x0E,


	MSG_CMD_FOUND_DEVICE				= 0x23,
	MSG_CMD_LOCK_DEVICE					= 0x24,

	MSG_CMD_GET_SERVER_ADDR				= 0x41,
	MSG_CMD_REQUST_CONNECT				= 0x42,

	MSG_CMD_HEART_BEAT 					= 0x61,

	MSG_CMD_QUARY_MODULE_INFO			= 0x62,
	MSG_CMD_SET_MODULE_NAME				= 0x63,
	MSG_CMD_MODULE_UPGRADE				= 0x65,

	MSG_CMD_ENTER_SMART_LINK			= 0x66,

	//Local message start from 0xE1
	MSG_CMD_LOCAL_ENTER_SMARTLINK		= 0xE1,
} MESSAGE_CMD_TYPE;



void USER_FUNC deviceMessageThread(void);


BOOL USER_FUNC insertSocketMsgToList(MSG_ORIGIN msgOrigin, U8* pData, U32 dataLen, U32 socketIp);
BOOL USER_FUNC insertLocalMsgToList(MSG_ORIGIN msgOrigin, U8* pData, U32 dataLen, U8 cmdData);


#endif
