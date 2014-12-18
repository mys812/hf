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
#define PER_READ_SIZE			256

#define MAX_FLASH_SIZE			(HFUFLASH_SIZE - (HFFLASH_PAGE_SIZE<<1))

#define FLASH_LOG_OFFSET		0
#define FLASH_LOG_SIZE			(MAX_FLASH_SIZE>>1)
#define FLASH_BAK_LOG_OFFSET	FLASH_LOG_SIZE

#define FLASH_LOG_INFO_OFFSET	MAX_FLASH_SIZE
#define FLASH_LOG_INFO_SIZE		sizeof(FLASH_LOG_INFO)
#define FLASH_LOG_FLAG			0xEDCBA987U


static U32 g_flashLogLen = 0;
hfthread_mutex_t g_flashWrite_mutex;




static void USER_FUNC writeFlashLogFlag(void)
{
	FLASH_LOG_INFO logInfo;
	
	
	logInfo.flag = FLASH_LOG_FLAG;
	logInfo.bakLogLen = g_flashLogLen;
	hfthread_mutext_lock(g_flashWrite_mutex);
	hfuflash_erase_page(FLASH_LOG_INFO_OFFSET,1);
	hfuflash_write(FLASH_LOG_INFO_OFFSET, (S8*)(&logInfo), FLASH_LOG_INFO_SIZE);
	hfthread_mutext_unlock(g_flashWrite_mutex);
	g_flashLogLen = 0;
}


static void USER_FUNC readFlashFlag(FLASH_LOG_INFO* logInfo)
{
	memset(logInfo, 0, sizeof(FLASH_LOG_INFO));
	hfuflash_read(FLASH_LOG_INFO_OFFSET, (S8*)(logInfo), FLASH_LOG_INFO_SIZE);
	lumi_debug("flag=0x%x, bakLen=%d\n", logInfo->flag, logInfo->bakLogLen);
}



static void USER_FUNC copyLogToBakFlash(void)
{
	S32 pages;
	U32 offset;
	S8 tmpBuf[PER_READ_SIZE + 2];


	pages = FLASH_LOG_SIZE/HFFLASH_PAGE_SIZE;
	offset = 0;

	hfthread_mutext_lock(g_flashWrite_mutex);
	hfuflash_erase_page(FLASH_BAK_LOG_OFFSET, pages);
	while(offset < FLASH_LOG_SIZE)
	{
		hfuflash_read((FLASH_LOG_OFFSET + offset), tmpBuf, PER_READ_SIZE);
		hfuflash_write((FLASH_BAK_LOG_OFFSET + offset), tmpBuf, PER_READ_SIZE);
		offset += PER_READ_SIZE;
	}
	hfuflash_erase_page(FLASH_LOG_OFFSET, pages);
	hfthread_mutext_unlock(g_flashWrite_mutex);
	writeFlashLogFlag();
}


static void clearAllFlashLog(void)
{
	hfthread_mutext_lock(g_flashWrite_mutex);
	hfuflash_erase_page(FLASH_LOG_OFFSET, MAX_FLASH_SIZE/HFFLASH_PAGE_SIZE);
	hfthread_mutext_unlock(g_flashWrite_mutex);
	writeFlashLogFlag();
}


static void USER_FUNC getFlashSavedLogLen(void)
{
	FLASH_LOG_INFO logInfo;
	S8 readBuf[PER_READ_SIZE + 2];
	U32 readOffset = 0;
	S32 readSize;
	U16 i;


	readFlashFlag(&logInfo);
	if(logInfo.flag != FLASH_LOG_FLAG)
	{
		clearAllFlashLog();
	}
	else
	{
		hfthread_mutext_lock(g_flashWrite_mutex);
		while(readOffset < FLASH_LOG_SIZE)
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
						readOffset += i;
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
		hfthread_mutext_unlock(g_flashWrite_mutex);
		lumi_debug("readOffset=%d, maxReadCount=%d\n", readOffset, FLASH_LOG_SIZE);
		if(readOffset >= FLASH_LOG_SIZE)
		{
			copyLogToBakFlash();
		}
		else
		{
			g_flashLogLen = readOffset;
		}
	}
}


static void USER_FUNC saveFlashLog(S8* saveData, U32 lenth)
{
	S32 writeLen;

	if(FLASH_LOG_SIZE <= (lenth+g_flashLogLen))
	{
		copyLogToBakFlash();
	}
	hfthread_mutext_lock(g_flashWrite_mutex);
	writeLen = hfuflash_write(g_flashLogLen, saveData, lenth);
	if(writeLen > 0)
	{
		g_flashLogLen += writeLen;
	}
	hfthread_mutext_unlock(g_flashWrite_mutex);
	//lumi_debug("g_flashLogLen=%d\n", g_flashLogLen);
}


static void USER_FUNC showFlashLog(U32 offset, U32 lenth)
{
	S8 readBuf[PER_READ_SIZE + 2];
	U32 readOffset;
	S32 readSize;
	U32 readLen;
	U32 maxReadOffset;

	readOffset = offset;
	readLen = PER_READ_SIZE;
	maxReadOffset = (readOffset + lenth);
		
	while(readOffset < maxReadOffset)
	{
		memset(readBuf, 0, sizeof(readBuf));
		if((readOffset + PER_READ_SIZE) > maxReadOffset)
		{
			readLen = maxReadOffset - readOffset;
		}
		readSize = hfuflash_read(readOffset, readBuf, readLen);

		if(readSize <= 0)
		{
			break;
		}
		//sendUdpData((U8*)readBuf, readSize, sendIP);
		u_printf("%s", readBuf);
		readOffset += readSize;
		msleep(50);
	}
}


static void USER_FUNC readFlashLogThread(void *arg)
{
	FLASH_LOG_INFO logInfo;

	//read bak flash log
	readFlashFlag(&logInfo);
	if(logInfo.bakLogLen != 0)
	{
		showFlashLog(FLASH_BAK_LOG_OFFSET, logInfo.bakLogLen);
	}

	//read cur flash log
	showFlashLog(FLASH_LOG_OFFSET, g_flashLogLen);	
	hfthread_destroy(NULL);
}



void USER_FUNC readFlashLog(void)
{
	if(hfthread_create((PHFTHREAD_START_ROUTINE)readFlashLogThread, "IOT_LOG_L",256, NULL, HFTHREAD_PRIORITIES_LOW,NULL,NULL)!= HF_SUCCESS)
	{
		lumi_error("Create IOT_LOG_L thread failed!\n");
	}
}


static U32 USER_FUNC formatSockeData(U8* socketData, U32 dataLen, S8* strData)
{
	U16 i;
	U32 index;


	sprintf(strData, "cmd=0x%02X ", socketData[16]);
	index = strlen(strData);

	for(i=0; i<dataLen; i++)
	{
		sprintf(strData + index, "%02X", socketData[i]);
		index += 2;

		if(i%4 == 3)
		{
			strData[index] = ' ';
			index++;
			if(i == 15) //header end
			{
				strData[index] = ' ';
				index++;
			}
		}
	}
	strData[index] = '\n';
	index++;

	return index;
}


static U32 USER_FUNC formatHeaderData(BOOL bRecive, MSG_ORIGIN socketFrom, U32 dataLen, S8* strData)
{
	static U16 g_sendIndex = 0; //
	S8 dateData[40];
	U32 strLenth;
	S8* fromStr;


	memset(dateData, 0, sizeof(dateData));

	//date
	getLocalTimeString(dateData, FALSE); //get date
	strcpy(strData, dateData);
	strLenth = strlen(strData);

	fromStr = bRecive?"<==":"==>";
	sprintf((strData + strLenth), "%s (%04d) %s len=%d ", fromStr, g_sendIndex, getMsgComeFrom(socketFrom), dataLen);
	g_sendIndex++;
	if(g_sendIndex >= 9999)
	{
		g_sendIndex = 0;
	}
	strLenth = strlen(strData);
	return strLenth;
}


void USER_FUNC saveSocketData(BOOL bRecive, MSG_ORIGIN socketFrom, U8* socketData, U32 dataLen)
{
	S8* strData;
	U32 mallocSize;
	U32 strLenth;


	if(socketData == NULL || dataLen == 0)
	{
		return;
	}
	mallocSize = (dataLen<<1) + (dataLen>>2) + 100;
	strData = (S8*)mallocSocketData(mallocSize);
	if(strData == NULL)
	{
		return;
	}
	memset(strData, 0, sizeof(mallocSize));

	strLenth = formatHeaderData(bRecive, socketFrom, dataLen, strData);
	if(strLenth >= 100)
	{
		lumi_error("header lenth to long strLenth=%d\n", strLenth);
	}
	strLenth += formatSockeData(socketData, dataLen, (strData + strLenth));

	saveFlashLog(strData, strLenth);
#ifdef SEND_LOG_BY_UDP
	sendUdpData((U8*)strData, strLenth, getBroadcastAddr());
#endif
	FreeSocketData((U8*)strData);
}


void USER_FUNC initFlashLog(void)
{
	S8 resetFlag[100];
	U32 flagLen;


	if((hfthread_mutext_new(&g_flashWrite_mutex)!= HF_SUCCESS))
	{
		lumi_debug("failed to create g_flashWrite_mutex");
	}
	getFlashSavedLogLen();
	lumi_debug("g_flashLogLen = %d\n", g_flashLogLen);
	memset(resetFlag, 0, sizeof(resetFlag));
	strcpy(resetFlag, "\n\n********************************HasBeenReset****************************\n\n"); //write flash can't used ROM data, So copy to RAM
	flagLen = strlen(resetFlag);
	saveFlashLog(resetFlag, flagLen);
}


#endif //SAVE_LOG_TO_FLASH

#endif
