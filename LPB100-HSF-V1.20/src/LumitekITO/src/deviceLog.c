/*
******************************
*Company:Lumitek
*Data:2014-12-13
*Author:Meiyusong
******************************
*/

#include "../inc/lumitekConfig.h"

#ifdef CONFIG_LUMITEK_DEVICE
#include <hsf.h>
#include <string.h>
#include <stdio.h>

#include "../inc/itoCommon.h"
#include "../inc/deviceTime.h"
#include "../inc/localSocketUdp.h"
#include "../inc/asyncMessage.h"
#include "../inc/deviceLog.h"




#ifdef SAVE_LOG_TO_FLASH
#define PER_READ_SIZE		256
#define FLASH_LOG_FLAG		0xDCBA
static S32 g_flashLogLen = -1;




static void USER_FUNC getFlashSavedLogLen(void)
{
	S8 readBuf[PER_READ_SIZE + 2];
	U32 readOffset = 0;
	S32 readSize;
	BOOL needErase = FALSE;
	U16 flag;
	U16 i;


	readSize = hfuflash_read(readOffset, (S8*)(&flag), 2);
	if(flag != FLASH_LOG_FLAG)
	{
		needErase = TRUE;
	}
	else
	{
		U32 maxReadCount;


		maxReadCount = HFUFLASH_SIZE - 1024;
		while(readOffset < maxReadCount)
		{
			memset(readBuf, 0, sizeof(readBuf));
			readSize = hfuflash_read(readOffset, readBuf, PER_READ_SIZE);
			if(readSize <= 0)
			{
				break;
			}
			if(readBuf[readSize-1] == 0xFF)
			{
				for(i=0; i<readSize; i++)
				{
					if(readBuf[i] == 0xFF)
					{
						readOffset += (i+1);
						break;
					}
				}
				break;
			}
			else
			{
				readOffset += readSize;
			}
		}
		lumi_debug("readOffset=%d, maxReadCount=%d\n", readOffset, maxReadCount);
		if(readOffset >= maxReadCount)
		{
			needErase = TRUE;
		}
	}

	if(needErase)
	{
		clearFlashLog();
	}
	else
	{
		g_flashLogLen = readOffset;
	}
}


void USER_FUNC initFlashLog(void)
{
	if(g_flashLogLen == -1)
	{
		getFlashSavedLogLen();
	}
	lumi_debug("g_flashLogLen = %d\n", g_flashLogLen);
}


void USER_FUNC saveFlashLog(S8* saveData, U32 lenth)
{
	S32 writeLen;

	writeLen = hfuflash_write(g_flashLogLen, saveData, lenth);
	if(writeLen > 0)
	{
		g_flashLogLen += writeLen;
	}
	lumi_debug("g_flashLogLen=%d\n", g_flashLogLen);
}


static void USER_FUNC readFlashLogThread(void *arg)
{
	S8 readBuf[PER_READ_SIZE + 2];
	U32 readOffset = 2;
	S32 readSize;
	U32 sendIP;


	sendIP = inet_addr(SEND_LOG_IP);
	while(readOffset < g_flashLogLen)
	{
		memset(readBuf, 0, sizeof(readBuf));
		readSize = hfuflash_read(readOffset, readBuf, PER_READ_SIZE);

		if(readSize <= 0)
		{
			break;
		}
		sendUdpData((U8*)readBuf, readSize, sendIP);
		readOffset += readSize;
		msleep(50);
	}
	hfthread_destroy(NULL);
}



void USER_FUNC readFlashLog(void)
{
	if(hfthread_create((PHFTHREAD_START_ROUTINE)readFlashLogThread, "IOT_LOG_L",256, NULL, HFTHREAD_PRIORITIES_LOW,NULL,NULL)!= HF_SUCCESS)
	{
		lumi_error("Create IOT_LOG_L thread failed!\n");
	}

}


void USER_FUNC clearFlashLog(void)
{
	U16 flag;
	
	
	flag = FLASH_LOG_FLAG;
	hfuflash_erase_page(0, HFUFLASH_SIZE/HFFLASH_PAGE_SIZE);
	hfuflash_write(0, (S8*)(&flag), 2);
	g_flashLogLen = 2;
}

#endif


#ifdef SEND_LOG_BY_UDP
static void USER_FUNC sendStrByUdp(S8* headerStr, U8* sendBuf, U32 dataLen)
{
	S8* sendStr;
	U32 sendLen;
	U32 i;
	U32 index = 0;
	U16 headerLen = 0;
	S8 dateData[40];


	if(headerStr != NULL)
	{
		headerLen = strlen(headerStr);
	}
	sendLen = (dataLen<<1) + (dataLen>>2) + headerLen + 50; //string data + space + header + date + other
	sendStr = (S8*)mallocSocketData(sendLen);

	if(sendStr == NULL)
	{
		return;
	}
	memset(sendStr, 0, sendLen);
	memset(dateData, 0, sizeof(dateData));

	getLocalTimeString(dateData, FALSE); //get date
	strcpy(sendStr, dateData);
	index += strlen(sendStr);
	
	if(headerStr != NULL)
	{		
		strcpy(sendStr+index, headerStr);
		index += headerLen;
	}
	for(i=0; i<dataLen; i++)
	{
		sprintf(sendStr+index, "%02X", sendBuf[i]);
		index += 2;

		if(i%4 == 3)
		{
			sendStr[index] = ' ';
			index++;
			if(i == 15) //header end
			{
				sendStr[index] = ' ';
				index++;
			}
		}
	}
	sendStr[index] = '\n';
	index++;

	//sendIp = inet_addr(SEND_LOG_IP);
	//sendUdpData((U8*)sendStr, index, socketIp);
	
#ifdef SAVE_LOG_TO_FLASH
	saveFlashLog(sendStr, index);
#endif //SAVE_LOG_TO_FLASH

	FreeSocketData((U8*)sendStr);
}


void USER_FUNC sendLogByUdp(BOOL bRecive, MSG_ORIGIN socketFrom, U8 cmdData, U8* sendBuf, U32 dataLen)
{
	S8 headerStr[100];
	S8* fromStr;	
	static U16 sendIndex = 0;


	memset(headerStr, 0, sizeof(headerStr));	
	fromStr = bRecive?"<==":"==>";
	sprintf(headerStr, "%s (%04d) %s len=%d cmd=%02X ", fromStr, sendIndex, getMsgComeFrom(socketFrom), dataLen, cmdData);
	sendIndex++;
	if(sendIndex >= 9999)
	{
		sendIndex = 0;
	}
	sendStrByUdp(headerStr, sendBuf, dataLen);
}

#endif


#endif
