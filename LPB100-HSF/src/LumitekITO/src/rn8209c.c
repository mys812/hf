/*
******************************
*Company:Lumitek
*Data:2014-12-24
*Author:Meiyusong
******************************
*/

#include "../inc/lumitekConfig.h"

#ifdef CONFIG_LUMITEK_DEVICE
#include <hsf.h>
#include <string.h>
#include <stdio.h>

#include "../inc/itoCommon.h"
#include "../inc/rn8209c.h"
#include "../inc/deviceLog.h"

#ifdef RN8209C_SUPPORT

hfthread_mutex_t g_rn8209c_mutex = NULL;


#if 0
static U32 USER_FUNC rn8209cDelay(U32 ms)
{
	U32 i=0;
	U32 j;
	U32 time1;
	U32 time2;
	U32 time3;

	time1 = hfsys_get_time();
	while(i < ms)
	{
		j = 0;
		while(j < 11500)
		{
			j++;
		}
		i++;
	}
	time2 = hfsys_get_time();
	time3 = time2 - time1;
	return time3;
}
#endif


U8 USER_FUNC rn8209cGetCheckksun(U8 cmd, U8* data, U8 dataLen)
{
	U32 totalSun;
	U8 checkSun;
	U8 i;


	if(dataLen > 0xFF || dataLen == 0)
	{
		return 0;
	}
	totalSun = cmd;
	for(i=0; i<dataLen; i++)
	{
		totalSun += data[i];
	}
	checkSun = (U8)(~(totalSun&0xFF));
	return checkSun;
}


static BOOL USER_FUNC rn8209cSetUartBaudrate(void)
{
	char rsp[64]={0};
	BOOL ret = FALSE;
	S8* sensBuf1 = "AT+UART=0\r\n";
	S8* sendBuf = "AT+UART=4800,8,1,EVEN,NFC\r\n";
	S8* cmpBuf = "+ok=4800";
		
		
	memset(rsp, 0, sizeof(rsp));
	hfat_send_cmd(sensBuf1, strlen(sensBuf1),rsp,64);
	if(((rsp[0]=='+')&&(rsp[1]=='o')&&(rsp[2]=='k')))
	{
		if(memcmp(rsp, cmpBuf, strlen(cmpBuf)) != 0)
		{	
			memset(rsp, 0, sizeof(rsp));
			hfat_send_cmd(sendBuf, strlen(sendBuf),rsp,64);
			if(((rsp[0]=='+')&&(rsp[1]=='o')&&(rsp[2]=='k')))
			{
				ret = TRUE;
			}
		}
		else
		{
			ret = TRUE;
		}
	}
	return ret;
}


static BOOL USER_FUNC uartLogDisable(void)
{
	char rsp[32]={0};
	BOOL ret = FALSE;
	S8* sendBuf = "AT+NDBGL=0,0\r\n";
	

	memset(rsp, 0, sizeof(rsp));
	hfat_send_cmd(sendBuf, strlen(sendBuf),rsp,32);
	if(((rsp[0]=='+')&&(rsp[1]=='o')&&(rsp[2]=='k')))
	{
		ret = TRUE;
	}
	return ret;
}


static void USER_FUNC rn8209cUartInit(void)
{
	hfthread_mutext_new(&g_rn8209c_mutex);
	hfuart_open(RN8209C_UART_NO);
	msleep(10);
	rn8209cSetUartBaudrate();
	uartLogDisable();
}


static void USER_FUNC rn8209cWriteFrame(U8 addr, U8* data, U8 dataLen)
{
	U8 sendData[RN9029C_MAX_DATA_LEN];
	U8 sendLen;
	U8 cmd;
	U8 checkSun;
	U8 i;


	memset(sendData, 0, sizeof(sendData));
	cmd = addr|0x80;
	sendData[0] = cmd;
	//memcpy((sendData + 1), data, dataLen);
	for(i=1; i<=dataLen; i++)
	{
		sendData[i] = data[dataLen-i];
	}
	
	checkSun = rn8209cGetCheckksun(cmd, data, dataLen);
	sendData[dataLen+1] = checkSun;
	sendLen = dataLen+2;

	hfuart_send(HFUART0, (S8*)sendData, sendLen, RN8209C_UART_TIMEOUT);
	
}


static BOOL USER_FUNC rn8209cReadFrame(U8 addr, U8* data, U8 readLen)
{
	U8 cmd;
	S32 recvLen = 0;
	S32 tmp;
	S8 readBuf[RN9029C_MAX_DATA_LEN];
	U8 checkSun;
	BOOL ret = FALSE;
	U8 i;


	cmd = addr&0x7F;
	memset(readBuf, 0, sizeof(readBuf));
	hfthread_mutext_lock(g_rn8209c_mutex);
	tmp = hfuart_recv(HFUART0, readBuf, RN9029C_MAX_DATA_LEN, 10);
	hfuart_send(HFUART0, (S8*)(&cmd), 1, 100);
	memset(readBuf, 0, sizeof(readBuf));
	while(recvLen < (readLen+1))
	{
		tmp = hfuart_recv(HFUART0, (readBuf + recvLen), readLen+1, 1000);
		if(tmp > 0)
		{
			recvLen += tmp;
		}
	}
	checkSun = rn8209cGetCheckksun(addr, (U8*)readBuf, readLen);
	if(checkSun == (U8)readBuf[readLen])
	{
		for(i=0; i<readLen; i++)
		{
			data[readLen-i-1] = readBuf[i];
		}
		ret = TRUE;
	}
	hfthread_mutext_unlock(g_rn8209c_mutex);
	return ret;
}



void USER_FUNC rn8209cInit(void)
{
	U8 writeData;
	U16 readData;
	U32 chipID;

	//uart init
	rn8209cUartInit();

	//rn8209c reset
	writeData = RN8209C_CMD_RESET;
	rn8209cWriteFrame(RN8209C_EA, &writeData, 1);
	msleep(300);
	
	//read chip ID
	rn8209cReadFrame(RN8209C_DeviceID, (U8*)&chipID, 3);
	saveNormalLogData("addr=0x%X ID=0x%x%x%x", RN8209C_EA, chipID);

	//write en
	writeData = RN9208C_WRITE_EN;
	rn8209cWriteFrame(RN8209C_EA, &writeData, 1);
	rn8209cReadFrame(RN8209C_WData, (U8*)&readData, 2);
	saveNormalLogData("addr=0x%X writeData=0x%x readData=0x%x", RN8209C_EA, writeData, readData);

	//select path A
	writeData = RN8209C_SELECT_PATH_A;
	rn8209cWriteFrame(RN8209C_EA, &writeData, 1);
	rn8209cReadFrame(RN8209C_WData, (U8*)&readData, 2);
	saveNormalLogData("addr=0x%X writeData=0x%x readData=0x%x", RN8209C_EA, writeData, readData);

	//write protect
	writeData = RN8209C_WRITE_PROTECT;
	rn8209cWriteFrame(RN8209C_EA, &writeData, 1);
	rn8209cReadFrame(RN8209C_WData, (U8*)&readData, 2);
	saveNormalLogData("addr=0x%X writeData=0x%x readData=0x%x", RN8209C_EA, writeData, readData);
}


void USER_FUNC rn8209cReadMeasureData(MeasureDataInfo* pMeatureData)
{
	S32 readData=0;

	//读电流有效值
	readData=0;
	rn8209cReadFrame(RN8209C_IARMS, (U8*)readData, 3);
	if(readData&0x800000)
	{
		readData=0;
	}
	pMeatureData->reco_irms=readData*100/2220;

	//读电压有效值
	readData=0;
	rn8209cReadFrame(RN8209C_URMS, (U8*)readData, 3);
	if(readData&0x800000)
	{
		readData=0;
	}
	pMeatureData->reco_urms=readData*100/2220;

	//读电压频率
	readData=0;
	rn8209cReadFrame(RN8209C_UFreq, (U8*)readData, 2);
	pMeatureData->reco_freq=357954500/8/readData;

	//读有功功率
	readData=0;
	rn8209cReadFrame(RN8209C_PowerPA, (U8*)readData, 4);
	pMeatureData->reco_powerp=readData;

	//读无功功率
	readData=0;
	rn8209cReadFrame(RN8209C_PowerQ, (U8*)readData, 4);
	pMeatureData->reco_powerq=readData;

	//读有功能量
	readData=0;
	rn8209cReadFrame(RN8209C_EnergyP, (U8*)readData, 3);
	pMeatureData->reco_energyp=readData;

	//读无功能量
	readData=0;
	rn8209cReadFrame(RN8209C_EnergyQ, (U8*)readData, 3);
	pMeatureData->reco_energyq=readData;

}
#endif /* RN8209C_SUPPORT */
#endif


